//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   PATLeptonIsoEmbedder.cc                                                //
//                                                                          //
//   Embeds lepton relative isolation and isolation decisions as userfloats //
//       (1 for true, 0 for false) for use in other modules.                //
//                                                                          //
//   Author: Justin Marquez, U. Wisconsin                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// system includes
#include <memory>
#include <vector>
#include <iostream>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Common/interface/View.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/MuonReco/interface/MuonPFIsolation.h"

using pat::Electron, pat::ElectronCollection;
using reco::CandidatePtr;
typedef edm::View<pat::Electron> ElectronView;
using pat::Muon, pat::MuonCollection;
typedef edm::View<pat::Muon> MuonView;

class PATLeptonIsoEmbedder : public edm::stream::EDProducer<> {
public:
  explicit PATLeptonIsoEmbedder(const edm::ParameterSet&);
  ~PATLeptonIsoEmbedder() {}

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  double getIso(const Electron& electron);
  double getIso(const Muon& muon);

  edm::EDGetTokenT<ElectronView> electronToken_;
  edm::EDGetTokenT<MuonView> muonToken_;

  std::string fsrLabel_;
  std::string rhoLabel_;
  std::string eaLabel_;

  std::string isoValueLabel_;
  std::string isoDecisionLabel_;

  double muIsoDRMinCut_;
  double eIsoDRMinCut_;
  double eIsoEtaCut_;
  double isoDRMaxCut_;

  double eIsoCut_;
  double muIsoCut_;
};

PATLeptonIsoEmbedder::PATLeptonIsoEmbedder(const edm::ParameterSet& iConfig)
    : electronToken_(consumes<ElectronView>(iConfig.getParameter<edm::InputTag>("electrons"))),
      muonToken_(consumes<MuonView>(iConfig.getParameter<edm::InputTag>("muons"))),
      fsrLabel_(iConfig.exists("fsrLabel") ? iConfig.getParameter<std::string>("fsrLabel") : "dREtFSRCand"),
      rhoLabel_(iConfig.exists("rhoLabel") ? iConfig.getParameter<std::string>("rhoLabel") : "rho_fastjet"),
      eaLabel_(iConfig.exists("eaLabel") ? iConfig.getParameter<std::string>("eaLabel") : "EffectiveArea"),
      isoValueLabel_(iConfig.exists("isoValueLabel") ? iConfig.getParameter<std::string>("isoValueLabel") : "HZZ4lIso"),
      isoDecisionLabel_(iConfig.exists("isoDecisionLabel") ? iConfig.getParameter<std::string>("isoDecisionLabel")
                                                           : "HZZ4lIsoPass"),
      muIsoDRMinCut_(iConfig.exists("muIsoDRMinCut") ? iConfig.getParameter<double>("muIsoDRMinCut") : 0.01),
      eIsoDRMinCut_(iConfig.exists("eIsoDRMinCut") ? iConfig.getParameter<double>("eIsoDRMinCut") : 0.08),
      eIsoEtaCut_(iConfig.exists("eIsoEtaCut") ? iConfig.getParameter<double>("eIsoEtaCut") : 1.479),
      isoDRMaxCut_(iConfig.exists("isoDRMaxCut") ? iConfig.getParameter<double>("isoDRMaxCut") : 0.3),
      eIsoCut_(iConfig.exists("eIsoCut") ? iConfig.getParameter<double>("eIsoCut") : 9999),
      muIsoCut_(iConfig.exists("muIsoCut") ? iConfig.getParameter<double>("muIsoCut") : 0.35) {
  produces<ElectronCollection>("electrons");
  produces<MuonCollection>("muons");
}

void PATLeptonIsoEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<ElectronView> electronsIn;
  iEvent.getByToken(electronToken_, electronsIn);

  edm::Handle<MuonView> muonsIn;
  iEvent.getByToken(muonToken_, muonsIn);

  // Get all FSR photons
  std::vector<CandidatePtr> selectedFSR;
  for (auto electron = electronsIn->begin(); electron != electronsIn->end(); electron++)
    if (electron->hasUserCand(fsrLabel_))
      selectedFSR.push_back(electron->userCand(fsrLabel_));
  for (auto muon = muonsIn->begin(); muon != muonsIn->end(); muon++)
    if (muon->hasUserCand(fsrLabel_))
      selectedFSR.push_back(muon->userCand(fsrLabel_));

  // Embed iso into electron collection
  std::unique_ptr<ElectronCollection> electronsOut(new ElectronCollection());
  for (auto it = electronsIn->begin(); it != electronsIn->end(); it++) {
    electronsOut->push_back(*it);
    Electron& electron = electronsOut->back();

    double iso = 9999;
    bool pass = false;
    if (electron.pt() > 0.0) {
      iso = getIso(electron);
      for (auto fsr : selectedFSR) {
        double deltaR = reco::deltaR(fsr->p4(), electron.p4());
        if (deltaR < isoDRMaxCut_ && (deltaR > eIsoDRMinCut_ || std::abs(electron.superCluster()->eta()) < eIsoEtaCut_))
          iso = std::max(0.0, iso - fsr->pt() / electron.pt());
      }

      pass = iso < eIsoCut_;
    }

    electron.addUserFloat(isoValueLabel_, iso);
    electron.addUserFloat(isoDecisionLabel_, float(pass));
  }

  // Embed iso into muon collection
  std::unique_ptr<MuonCollection> muonsOut(new MuonCollection());
  for (auto it = muonsIn->begin(); it != muonsIn->end(); it++) {
    muonsOut->push_back(*it);
    Muon& muon = muonsOut->back();

    double iso = 9999;
    bool pass = false;
    if (muon.pt() > 0.0) {
      iso = getIso(muon);
      for (auto fsr : selectedFSR) {
        double deltaR = reco::deltaR(fsr->p4(), muon.p4());
        if (deltaR < isoDRMaxCut_ && deltaR > muIsoDRMinCut_)
          iso = std::max(0.0, iso - fsr->pt() / muon.pt());
      }

      pass = iso < muIsoCut_;
    }

    muon.addUserFloat(isoValueLabel_, iso);
    muon.addUserFloat(isoDecisionLabel_, float(pass));
  }

  // Save to event
  iEvent.put(std::move(electronsOut), "electrons");
  iEvent.put(std::move(muonsOut), "muons");
}

double PATLeptonIsoEmbedder::getIso(const Electron& electron) {
  auto vars = electron.pfIsolationVariables();
  double chHadIso = vars.sumChargedHadronPt;
  double neHadIso = vars.sumNeutralHadronEt;
  double phoIso = vars.sumPhotonEt;
  double puCorr = electron.userFloat(rhoLabel_) * electron.userFloat(eaLabel_);

  return (chHadIso + std::max(neHadIso + phoIso - puCorr, 0.0)) / electron.pt();
}

double PATLeptonIsoEmbedder::getIso(const Muon& muon) {
  auto vars = muon.pfIsolationR03();
  double chHadIso = vars.sumChargedHadronPt;
  double neHadIso = vars.sumNeutralHadronEt;
  double phoIso = vars.sumPhotonEt;
  double puCorr = vars.sumPUPt / 2.0;

  return (chHadIso + std::max(neHadIso + phoIso - puCorr, 0.0)) / muon.pt();
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATLeptonIsoEmbedder);
