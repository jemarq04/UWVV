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

  void analyzeZZLeptons(const GenParticleCollection &leptons, double weight, Channel channel);

  edm::EDGetTokenT<GenParticleView> srcToken_;
  edm::EDGetTokenT<GenEventInfoProduct> genToken_;

  int numEventsTotal_;
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

  numEventsTotal_++;
  sumWeightsTotal_ += scale_ * genEvent->weight();

  GenParticleCollection dressedleptons, photons, zzleptons;
  for (GenParticleView::const_iterator it = genparticles->begin(); it != genparticles->end(); it++) {
    int absPdgId = std::abs(it->pdgId());
    if ((absPdgId == 11 || absPdgId == 13) && it->fromHardProcessFinalState())
      dressedleptons.push_back(*it);
    else if (absPdgId == 22 && it->statusFlags().isPrompt() && it->status() == 1)
      photons.push_back(*it);
  }

  size_t nLeptons = dressedleptons.size();
  if (nLeptons < 4)
    return;  // definitely not a ZZ event

  for (const auto &photon : photons)
    for (auto &lepton : dressedleptons)
      if (reco::deltaR(lepton.p4(), photon.p4()) < 0.1)
        lepton.setP4(lepton.p4() + photon.p4());

  // determine best ZZ candidate leptons
  int bestLeptonsIdx[4] = {-1, -1, -1, -1};
  int currLeptonsIdx[4] = {-1, -1, -1, -1};
  double min_dMZ = 999, max_z2LepPt = 0;
  Channel channel = c_eemm;

  int pdgIds[4] = {0, 0, 0, 0};
  size_t nElectrons, nMuons;
  for (size_t i = 0; i < nLeptons - 3; i++) {
    for (size_t j = i + 1; j < nLeptons - 2; j++) {
      for (size_t k = j + 1; k < nLeptons - 1; k++) {
        for (size_t l = k + 1; l < nLeptons; l++) {
          pdgIds[0] = dressedleptons[i].pdgId();
          pdgIds[1] = dressedleptons[j].pdgId();
          pdgIds[2] = dressedleptons[k].pdgId();
          pdgIds[3] = dressedleptons[l].pdgId();

          // check opposite-sign pairs
          if (pdgIds[0] * pdgIds[1] * pdgIds[2] * pdgIds[3] < 0)
            continue;

          nElectrons = nMuons = 0;
          pdgIds[0] = std::abs(pdgIds[0]);
          pdgIds[1] = std::abs(pdgIds[1]);
          pdgIds[2] = std::abs(pdgIds[2]);
          pdgIds[3] = std::abs(pdgIds[3]);
          for (const auto &id : pdgIds) {
            if (id == 11)
              nElectrons++;
            else
              nMuons++;
          }

          // check opposite-flavor pairs
          if (nElectrons % 2 != 0 || nMuons % 2 != 0)
            continue;

          // determine primary and secondary Z candidates
          if (nElectrons == 4 || nMuons == 4) {
            currLeptonsIdx[0] = i;
            currLeptonsIdx[1] = j;
            currLeptonsIdx[2] = k;
            currLeptonsIdx[3] = l;
          } else {
            if (pdgIds[0] == pdgIds[1]) {
              currLeptonsIdx[0] = i;
              currLeptonsIdx[1] = j;
              currLeptonsIdx[2] = k;
              currLeptonsIdx[3] = l;
            } else if (pdgIds[0] == pdgIds[2]) {
              currLeptonsIdx[0] = i;
              currLeptonsIdx[1] = k;
              currLeptonsIdx[2] = j;
              currLeptonsIdx[3] = l;
            } else {
              currLeptonsIdx[0] = i;
              currLeptonsIdx[1] = l;
              currLeptonsIdx[2] = j;
              currLeptonsIdx[3] = k;
            }
          }

          double dMZ1 =
              std::abs((dressedleptons[currLeptonsIdx[0]].p4() + dressedleptons[currLeptonsIdx[1]].p4()).M() - 91.1876);
          double dMZ2 =
              std::abs((dressedleptons[currLeptonsIdx[2]].p4() + dressedleptons[currLeptonsIdx[3]].p4()).M() - 91.1876);

          // swap Z candidates if Z2 is closer to Z mass
          if (dMZ2 < dMZ1) {
            std::swap(currLeptonsIdx[0], currLeptonsIdx[2]);
            std::swap(currLeptonsIdx[1], currLeptonsIdx[3]);
            std::swap(dMZ1, dMZ2);
          }
          double z2LepPt = dressedleptons[currLeptonsIdx[2]].p4().pt() + dressedleptons[currLeptonsIdx[3]].p4().pt();

          // check if this is best ZZ candidate
          if (dMZ1 < min_dMZ || (dMZ1 == min_dMZ && z2LepPt > max_z2LepPt)) {
            bestLeptonsIdx[0] = currLeptonsIdx[0];
            bestLeptonsIdx[1] = currLeptonsIdx[1];
            bestLeptonsIdx[2] = currLeptonsIdx[2];
            bestLeptonsIdx[3] = currLeptonsIdx[3];
            min_dMZ = dMZ1;
            max_z2LepPt = z2LepPt;
            if (nElectrons == 4)
              channel = c_eeee;
            else if (nMuons == 4)
              channel = c_mmmm;
          }
        }
      }
    }
  }

  // add best ZZ candidate leptons to final collection
  for (int idx : bestLeptonsIdx)
    zzleptons.push_back(dressedleptons[idx]);

  if (verbose_)
    for (size_t i = 0; i < zzleptons.size(); i += 2)
      std::cout << "dhist000 " << (zzleptons[i].p4() + zzleptons[i + 1].p4()).M() << std::endl;

  analyzeZZLeptons(zzleptons, scale_ * genEvent->weight(), channel);
}

void GenZZDressedXsecAnalyzer::analyzeZZLeptons(const GenParticleCollection &leptons, double weight, Channel channel) {
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

void GenZZDressedXsecAnalyzer::beginJob() {
  numEventsTotal_ = 0;
  sumWeightsTotal_ = 0.0;
  for (size_t i = 0; i < 3; i++) {
    sumWeightsOnShell_[i] = 0.0;
    sumWeightsFiducial_[i] = 0.0;
  }
}

void GenZZDressedXsecAnalyzer::endJob() {
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
DEFINE_FWK_MODULE(GenZZDressedXsecAnalyzer);
