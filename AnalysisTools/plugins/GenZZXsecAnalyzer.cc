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
  sumWeightsTotal_ += genEvent->weight();

  GenParticleCollection leptons;
  for (GenParticleView::const_iterator it = genparticles->begin(); it != genparticles->end(); it++) {
    int absPdgId = std::abs(it->pdgId());

    // only consider leptons passing this condition
    if ((absPdgId != 11 && absPdgId != 13) || !it->fromHardProcessFinalState())
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
  double accuracy = 1e-5;
  auto l1zp4 = leptons[0].mother(0)->p4();
  for (size_t i = 1; i < nLeptons; i++) {
    auto l2zp4 = leptons[i].mother(0)->p4();
    double dPt = std::abs(l1zp4.pt() - l2zp4.pt());
    double dEta = std::abs(l1zp4.eta() - l2zp4.eta());
    double dPhi = std::abs(l1zp4.phi() - l2zp4.phi());
    double dM = std::abs(l1zp4.M() - l2zp4.M());
    if (dPt < accuracy && dEta < accuracy && dPhi < accuracy && dM < accuracy) {
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
  {
    auto l3zp4 = zzleptons[2].mother(0)->p4();
    auto l4zp4 = zzleptons[3].mother(0)->p4();
    double dPt = std::abs(l3zp4.pt() - l4zp4.pt());
    double dEta = std::abs(l3zp4.eta() - l4zp4.eta());
    double dPhi = std::abs(l3zp4.phi() - l4zp4.phi());
    double dM = std::abs(l3zp4.M() - l4zp4.M());
    if (dPt > accuracy || dEta > accuracy || dPhi > accuracy || dM > accuracy) {
      std::cout << "ERROR: the z2 leptons don't come from the same Z!" << std::endl;
      return;
    }
  }
  analyzeZZLeptons(zzleptons, genEvent->weight());
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
      if (leptons[i].pdgId() * leptons[j].pdgId() < 0 && (leptons[i].p4() + leptons[j].p4()).M() < 4.0)
        return;

    leppt[i] = leptons[i].pt();
  }
  // pt cut - check that at least one pt is above 20 GeV with at least one other above 10
  bool ptcheck1 = leppt[0] > 20 && (leppt[1] > 10 || leppt[2] > 10 || leppt[3] > 10);
  bool ptcheck2 = leppt[1] > 20 && (leppt[0] > 10 || leppt[2] > 10 || leppt[3] > 10);
  bool ptcheck3 = leppt[2] > 20 && (leppt[0] > 10 || leppt[1] > 10 || leppt[3] > 10);
  bool ptcheck4 = leppt[3] > 20 && (leppt[0] > 10 || leppt[1] > 10 || leppt[2] > 10);
  if (!ptcheck1 && !ptcheck2 && !ptcheck3 && !ptcheck4)
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
  std::cout << "Total         " << label_ << ": " << scale_ * sumWeightsTotal_ / numEventsTotal_;
  std::cout << " pb (" << scale_ * sumWeightsTotal_ << "/" << numEventsTotal_ << ")" << std::endl;
  std::cout << "---------" << std::endl;

  scale_ *= 1000;
  for (size_t i = 0; i < 3; i++) {
    std::string channel;
    if (i == 0)
      channel = "eemm";
    else if (i == 1)
      channel = "mmmm";
    else if (i == 2)
      channel = "eeee";

    std::cout << "On Shell " << channel << " " << label_ << ": " << scale_ * sumWeightsOnShell_[i] / numEventsTotal_;
    std::cout << " fb (" << scale_ * sumWeightsOnShell_[i] << "/" << numEventsTotal_ << ")" << std::endl;

    std::cout << "Fiducial " << channel << " " << label_ << ": " << scale_ * sumWeightsFiducial_[i] / numEventsTotal_;
    std::cout << " fb (" << scale_ * sumWeightsFiducial_[i] << "/" << numEventsTotal_ << ")" << std::endl;

    std::cout << "---------" << std::endl;
  }
  std::cout << std::endl;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GenZZXsecAnalyzer);
