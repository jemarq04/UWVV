///////////////////////////////////////////////////////////////////////////////
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// system includes
#include <memory>
#include <iostream>

// CMS includes
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

using reco::GenParticle, reco::GenParticleCollection;
typedef edm::View<GenParticle> GenParticleView;

class GenZZXsecAnalyzer : public edm::one::EDAnalyzer<> {
public:
  explicit GenZZXsecAnalyzer(const edm::ParameterSet &iConfig);
  virtual ~GenZZXsecAnalyzer() {};

private:
  void beginJob() override;
  void analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) override;
  void endJob() override;

  enum Channel { c_eemm, c_mmmm, c_eeee };

  void analyzeZZLeptons(const GenParticleCollection &leptons, double weight);

  edm::EDGetTokenT<GenParticleView> srcToken_;
  edm::EDGetTokenT<GenEventInfoProduct> genToken_;

  int numEventsTotal_;
  double sumWeightsTotal_;
  double sumWeightsOnShell_[3];
  double sumWeightsFiducial_[3];
  std::string label_;
  double scale_;
};

GenZZXsecAnalyzer::GenZZXsecAnalyzer(const edm::ParameterSet &iConfig)
    : srcToken_(consumes<GenParticleView>(iConfig.getParameter<edm::InputTag>("src"))),
      genToken_(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
      label_(iConfig.exists("label") ? iConfig.getParameter<std::string>("label") : "xsec"),
      scale_(iConfig.exists("scale") ? iConfig.getParameter<double>("scale") : 1.0) {}

void GenZZXsecAnalyzer::analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) {
  edm::Handle<GenParticleView> genparticles;
  iEvent.getByToken(srcToken_, genparticles);

  edm::Handle<GenEventInfoProduct> genEvent;
  iEvent.getByToken(genToken_, genEvent);

  numEventsTotal_++;
  sumWeightsTotal_ += scale_ * genEvent->weight();

  GenParticleCollection leptons;
  for (GenParticleView::const_iterator it = genparticles->begin(); it != genparticles->end(); it++) {
    int absPdgId = std::abs(it->pdgId());

    // only consider leptons passing this condition
    if ((absPdgId != 11 && absPdgId != 13))  // || !it->fromHardProcessFinalState())
      continue;

    // check if lepton came from Z
    if (it->numberOfMothers() > 0 && std::abs(it->mother(0)->pdgId()) == 23)
      leptons.push_back(*it);
  }

  size_t nLeptons = leptons.size();
  if (nLeptons < 4)
    return;  //definitely not a ZZ event
  if (nLeptons > 4) {
    std::cout << "ERROR: Number of true FS leptons differ from expected: " << nLeptons << std::endl;
    return;
  }

  //properly order leptons
  GenParticleCollection zzleptons;
  zzleptons.push_back(leptons[0]);
  size_t z1lepidx = 0;
  for (size_t i = 1; i < nLeptons; i++) {
    if (leptons[0].mother(0) == leptons[i].mother(0)){
      z1lepidx = i;
      zzleptons.push_back(leptons[i]);
      break;
    }
  }
  if (z1lepidx == 0) {
    std::cout << "ERROR: could not find two leptons from the same Z!" << std::endl;
    return;
  }
  for (size_t i = 1; i < nLeptons; i++) {
    if (i == z1lepidx)
      continue;
    zzleptons.push_back(leptons[i]);
  }
  if (zzleptons[2].mother(0) != zzleptons[3].mother(0)){
    std::cout << "ERROR: the z2 leptons don't come from the same Z!" << std::endl;
    return;
  }
  analyzeZZLeptons(zzleptons, scale_ * genEvent->weight());
}

void GenZZXsecAnalyzer::analyzeZZLeptons(const GenParticleCollection &leptons, double weight) {
  int nElectrons = 0, nMuons = 0;
  for (const auto &lepton : leptons) {
    if (std::abs(lepton.pdgId()) == 11)
      nElectrons++;
    else
      nMuons++;
  }
  Channel channel;
  if (nElectrons == 4)
    channel = c_eeee;
  else if (nMuons == 4)
    channel = c_mmmm;
  else
    channel = c_eemm;

  double z1mass = (leptons[0].p4() + leptons[1].p4()).M();
  double z2mass = (leptons[2].p4() + leptons[3].p4()).M();
  if (z1mass < 60 || z1mass > 120 || z2mass < 60 || z2mass > 120)
    return;

  sumWeightsOnShell_[channel] += weight;

  double leppt[4] = {0.0};
  for (size_t i = 0; i < 4; i++) {
    // eta cut
    if (std::abs(leptons[i].eta()) > 2.5)
      return;

    // QCD veto
    for (size_t j = i + 1; j < 4; j++)
      if (leptons[i].pdgId() == -leptons[j].pdgId() && (leptons[i].p4() + leptons[j].p4()).M() < 4.0)
        return;

    leppt[i] = leptons[i].pt();
  }
  // pt cut - check that at least one pt is above 20 GeV with at least one other above 10
  std::sort(leppt, leppt + sizeof(leppt) / sizeof(leppt[0]), std::greater<double>());
  if (leppt[0] < 20 || leppt[1] < 10 || leppt[2] < 5 || leppt[3] < 5)
    return;

  sumWeightsFiducial_[channel] += weight;
}

void GenZZXsecAnalyzer::beginJob() {
  numEventsTotal_ = 0;
  sumWeightsTotal_ = 0.0;
  for (size_t i = 0; i < 3; i++) {
    sumWeightsOnShell_[i] = 0.0;
    sumWeightsFiducial_[i] = 0.0;
  }
}

void GenZZXsecAnalyzer::endJob() {
  std::cout << "=== Gen ZZ Xsec Analyzer ===" << std::endl;

  std::cout << "---------" << std::endl;
  std::cout << "Total         " << label_ << ": " << sumWeightsTotal_ / numEventsTotal_;
  std::cout << " pb (" << sumWeightsTotal_ << "/" << numEventsTotal_ << ")" << std::endl;
  std::cout << "---------" << std::endl;

  for (size_t i = 0; i < 3; i++) {
    std::string channel;
    if (i == 0)
      channel = "eemm";
    else if (i == 1)
      channel = "mmmm";
    else if (i == 2)
      channel = "eeee";

    std::cout << "On Shell " << channel << " " << label_ << ": " << 1000 * sumWeightsOnShell_[i] / numEventsTotal_;
    std::cout << " fb (" << 1000 * sumWeightsOnShell_[i] << "/" << numEventsTotal_ << ")" << std::endl;
  }
  std::cout << "---------" << std::endl;

  for (size_t i = 0; i < 3; i++) {
    std::string channel;
    if (i == 0)
      channel = "eemm";
    else if (i == 1)
      channel = "mmmm";
    else if (i == 2)
      channel = "eeee";

    std::cout << "Fiducial " << channel << " " << label_ << ": " << 1000 * sumWeightsFiducial_[i] / numEventsTotal_;
    std::cout << " fb (" << 1000 * sumWeightsFiducial_[i] << "/" << numEventsTotal_ << ")" << std::endl;
  }
  std::cout << "---------" << std::endl;

  std::cout << std::endl;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GenZZXsecAnalyzer);
