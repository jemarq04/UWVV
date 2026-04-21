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

GenZZXsecAnalyzer::GenZZXsecAnalyzer(const edm::ParameterSet &iConfig)
    : srcToken_(consumes<GenParticleView>(iConfig.getParameter<edm::InputTag>("src"))),
      genToken_(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
      label_(iConfig.exists("label") ? iConfig.getParameter<std::string>("label") : "xsec"),
      scale_(iConfig.exists("scale") ? iConfig.getParameter<double>("scale") : 1.0),
      verbose_(iConfig.exists("verbose") ? iConfig.getParameter<bool>("verbose") : false) {}

void GenZZXsecAnalyzer::analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) {
  edm::Handle<GenParticleView> genparticles;
  iEvent.getByToken(srcToken_, genparticles);

  edm::Handle<GenEventInfoProduct> genEvent;
  iEvent.getByToken(genToken_, genEvent);

  double weight = genEvent->weight();
  sumWeightsTotal_ += weight;

  int nElectrons = 0, nMuons = 0;
  GenParticleCollection leptons;
  for (GenParticleView::const_iterator it = genparticles->begin(); it != genparticles->end(); it++) {
    int absPdgId = std::abs(it->pdgId());

    // only consider leptons passing this condition
    if ((absPdgId != 11 && absPdgId != 13) || !it->isHardProcess())
      continue;

    // check if lepton came from Z
    if (it->numberOfMothers() == 0 || std::abs(it->mother(0)->pdgId()) != 23)
      continue;

    leptons.push_back(*it);
    if (absPdgId == 11)
      nElectrons++;
    else
      nMuons++;
  }

  size_t nLeptons = leptons.size();
  if (nLeptons < 4)
    return;  //definitely not a ZZ event
  if (nLeptons > 4) {
    std::cout << "ERROR: Number of true FS leptons differ from expected: " << nLeptons << std::endl;
    return;
  }
  if (nElectrons % 2 != 0) {
    std::cout << "ERROR: Odd number of electrons/muons! (" << nElectrons << "e, " << nMuons << "m)" << std::endl;
    return;
  }

  Channel channel;
  if (nElectrons == 4)
    channel = c_eeee;
  else if (nElectrons == 0)
    channel = c_mmmm;
  else
    channel = c_eemm;

  if (!selectionOnShell(leptons))
    return;
  sumWeightsOnShell_[channel] += weight;

  if (!selectionFiducial(leptons))
    return;
  sumWeightsFiducial_[channel] += weight;
}

bool GenZZXsecAnalyzer::selectionOnShell(const GenParticleCollection &leptons) {
  double best_mZ1 = -1;
  double best_mZ2 = -1;

  double min_dMZ1 = 1e10, max_z2LepPt = 0;
  std::vector<size_t> idx = {0, 1, 2, 3};
  do {
    // Ensure OSSF pairs
    if (leptons[idx[0]].pdgId() != -leptons[idx[1]].pdgId())
      continue;
    if (leptons[idx[2]].pdgId() != -leptons[idx[3]].pdgId())
      continue;

    double mZ1 = (leptons[idx[0]].p4() + leptons[idx[1]].p4()).M();
    double mZ2 = (leptons[idx[2]].p4() + leptons[idx[3]].p4()).M();
    double dMZ1 = std::abs(mZ1 - 91.1876);
    double dMZ2 = std::abs(mZ2 - 91.1876);
    if (dMZ2 < dMZ1)
      continue;  // will be found in another permutation
    double z2LepPt = leptons[idx[2]].pt() + leptons[idx[3]].pt();

    if (dMZ1 < min_dMZ1 || (dMZ1 == min_dMZ1 && z2LepPt > max_z2LepPt)) {
      min_dMZ1 = dMZ1;
      max_z2LepPt = z2LepPt;
      best_mZ1 = mZ1;
      best_mZ2 = mZ2;
    }

  } while (std::next_permutation(idx.begin(), idx.end()));

  bool z1pass = best_mZ1 > 60 && best_mZ1 < 120;
  bool z2pass = best_mZ2 > 60 && best_mZ2 < 120;
  return z1pass && z2pass;
}

bool GenZZXsecAnalyzer::selectionFiducial(const GenParticleCollection &leptons) {
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

void GenZZXsecAnalyzer::beginJob() {
  sumWeightsTotal_ = 0.0;
  for (size_t i = 0; i < 3; i++) {
    sumWeightsOnShell_[i] = 0.0;
    sumWeightsFiducial_[i] = 0.0;
  }
}

void GenZZXsecAnalyzer::endJob() {
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
DEFINE_FWK_MODULE(GenZZXsecAnalyzer);
