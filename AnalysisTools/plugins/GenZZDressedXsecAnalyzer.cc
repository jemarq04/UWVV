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

class GenZZDressedXsecAnalyzer : public edm::one::EDAnalyzer<> {
public:
  explicit GenZZDressedXsecAnalyzer(const edm::ParameterSet &iConfig);
  virtual ~GenZZDressedXsecAnalyzer() {};

private:
  void beginJob() override;
  void analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) override;
  void endJob() override;

  enum Channel { c_eemm, c_mmmm, c_eeee };

  GenParticleCollection getZZLeptons(const GenParticleCollection &leptons);
  bool selectionOnShell(const GenParticleCollection &leptons);
  bool selectionFiducial(const GenParticleCollection &leptons);

  edm::EDGetTokenT<GenParticleView> srcToken_;
  edm::EDGetTokenT<GenEventInfoProduct> genToken_;

  double sumWeightsTotal_;
  double sumWeightsOnShell_[3];
  double sumWeightsFiducial_[3];
  std::string label_;
  double scale_;
  bool verbose_;
};

GenZZDressedXsecAnalyzer::GenZZDressedXsecAnalyzer(const edm::ParameterSet &iConfig)
    : srcToken_(consumes<GenParticleView>(iConfig.getParameter<edm::InputTag>("src"))),
      genToken_(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
      label_(iConfig.exists("label") ? iConfig.getParameter<std::string>("label") : "xsec"),
      scale_(iConfig.exists("scale") ? iConfig.getParameter<double>("scale") : 1.0),
      verbose_(iConfig.exists("verbose") ? iConfig.getParameter<bool>("verbose") : false) {}

void GenZZDressedXsecAnalyzer::analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) {
  edm::Handle<GenParticleView> genparticles;
  iEvent.getByToken(srcToken_, genparticles);

  edm::Handle<GenEventInfoProduct> genEvent;
  iEvent.getByToken(genToken_, genEvent);

  double weight = genEvent->weight();
  sumWeightsTotal_ += weight;

  GenParticleCollection leptons, photons;
  for (GenParticleView::const_iterator it = genparticles->begin(); it != genparticles->end(); it++) {
    int absPdgId = std::abs(it->pdgId());
    if ((absPdgId == 11 || absPdgId == 13) && it->isHardProcess())
      leptons.push_back(*it);
    else if (absPdgId == 22 && it->statusFlags().isPrompt() && it->status() == 1)
      photons.push_back(*it);
  }

  if (leptons.size() < 4)
    return;  //definitely not a ZZ event

  for (const auto &photon : photons)
    for (auto &lepton : leptons)
      if (reco::deltaR(lepton.p4(), photon.p4()) < 0.1)
        lepton.setP4(lepton.p4() + photon.p4());

  GenParticleCollection zzleptons = getZZLeptons(leptons);
  if (zzleptons.size() == 0){
    std::cout << "ERROR: no ZZ leptons???" << std::endl;
    return;
  }
  size_t nElectrons = 0;
  for (const auto &lep : zzleptons)
    if (std::abs(lep.pdgId()) == 11)
      nElectrons++;

  Channel channel;
  if (nElectrons == 4)
    channel = c_eeee;
  else if (nElectrons == 0)
    channel = c_mmmm;
  else
    channel = c_eemm;

  if (!selectionOnShell(zzleptons))
    return;
  sumWeightsOnShell_[channel] += weight;

  if (!selectionFiducial(zzleptons))
    return;
  sumWeightsFiducial_[channel] += weight;
}

GenParticleCollection GenZZDressedXsecAnalyzer::getZZLeptons(const GenParticleCollection & leptons){
  GenParticleCollection zzleptons;

  double min_dMZ1 = 1e10, max_z2LepPt = 0;
  for (size_t i=0; i<leptons.size(); i++){
    for (size_t j=0; j<leptons.size(); j++){
      if (j==i) continue;
      for (size_t k=0; k<leptons.size(); k++){
        if (k==j || k==i) continue;
        for (size_t l=0; l<leptons.size(); l++){
          if (l==k || l==j || l==i) continue;

          // Ensure OSSF pairs
          if (leptons[i].pdgId() != -leptons[j].pdgId())
            continue;
          if (leptons[k].pdgId() != -leptons[l].pdgId())
            continue;

          double dMZ1 = std::abs((leptons[i].p4() + leptons[j].p4()).M() - 91.1876);
          double dMZ2 = std::abs((leptons[k].p4() + leptons[l].p4()).M() - 91.1876);
          if (dMZ2 < dMZ1)
            continue;  // will be found in another permutation
          double z2LepPt = leptons[k].pt() + leptons[l].pt();

          if (dMZ1 < min_dMZ1 || (dMZ1 == min_dMZ1 && z2LepPt > max_z2LepPt)) {
            min_dMZ1 = dMZ1;
            max_z2LepPt = z2LepPt;
            zzleptons.clear();
            zzleptons.push_back(leptons[i]);
            zzleptons.push_back(leptons[j]);
            zzleptons.push_back(leptons[k]);
            zzleptons.push_back(leptons[l]);
          }
        }
      }
    }
  }
  return zzleptons;
}

bool GenZZDressedXsecAnalyzer::selectionOnShell(const GenParticleCollection &leptons) {
  double best_mZ1 = (leptons[0].p4() + leptons[1].p4()).M();
  double best_mZ2 = (leptons[2].p4() + leptons[3].p4()).M();
  bool z1pass = best_mZ1 > 60 && best_mZ1 < 120;
  bool z2pass = best_mZ2 > 60 && best_mZ2 < 120;

  return z1pass && z2pass;
}

bool GenZZDressedXsecAnalyzer::selectionFiducial(const GenParticleCollection &leptons) {
  double leppt[4] = {0.0};
  for (size_t i = 0; i < 4; i++) {
    // eta cut
    if (std::abs(leptons[i].eta()) > 2.5 || leptons[i].pt() < 5)
      return false;

    // QCD veto
    for (size_t j = i + 1; j < 4; j++)
      if (leptons[i].pdgId() == -leptons[j].pdgId() && (leptons[i].p4() + leptons[j].p4()).M() < 4.0)
        return false;

    leppt[i] = leptons[i].pt();
  }
  // pt cut - check that at least one pt is above 20 GeV with at least one other above 10
  std::sort(leppt, leppt + sizeof(leppt) / sizeof(leppt[0]), std::greater<double>());
  if (leppt[0] < 20 || leppt[1] < 10)
    return false;

  return true;
}

void GenZZDressedXsecAnalyzer::beginJob() {
  sumWeightsTotal_ = 0.0;
  for (size_t i = 0; i < 3; i++) {
    sumWeightsOnShell_[i] = 0.0;
    sumWeightsFiducial_[i] = 0.0;
  }
}

void GenZZDressedXsecAnalyzer::endJob() {
  std::cout << "=== Gen ZZ Xsec Analyzer ===" << std::endl;

  std::cout << "---------" << std::endl;
  std::cout << "Total         " << label_ << ": " << scale_ * sumWeightsTotal_ / sumWeightsTotal_;
  std::cout << " pb (" << scale_ * sumWeightsTotal_ << "/" << sumWeightsTotal_ << ")" << std::endl;
  std::cout << "---------" << std::endl;

  for (size_t i = 0; i < 3; i++) {
    std::string channel;
    if (i == 0)
      channel = "eemm";
    else if (i == 1)
      channel = "mmmm";
    else if (i == 2)
      channel = "eeee";

    std::cout << "On Shell " << channel << " " << label_ << ": "
              << 1000 * scale_ * sumWeightsOnShell_[i] / sumWeightsTotal_;
    std::cout << " fb (" << 1000 * scale_ * sumWeightsOnShell_[i] << "/" << sumWeightsTotal_ << ")" << std::endl;
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

    std::cout << "Fiducial " << channel << " " << label_ << ": "
              << 1000 * scale_ * sumWeightsFiducial_[i] / sumWeightsTotal_;
    std::cout << " fb (" << 1000 * scale_ * sumWeightsFiducial_[i] << "/" << sumWeightsTotal_ << ")" << std::endl;
  }
  std::cout << "---------" << std::endl;

  std::cout << std::endl;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GenZZDressedXsecAnalyzer);
