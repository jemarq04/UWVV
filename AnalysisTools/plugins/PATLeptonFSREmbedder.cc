//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///    PATLeptonFSREmbedder.cc                                             ///
///                                                                        ///
///    From a collection of photons, a collection of muons, and a          ///
///        collection of electrons: make sure each photon is not in an     ///
///        electron supercluster, and pair it to its closest lepton.       ///
///        For each lepton, embed the photon with the smallest deltaR/eT   ///
///        as a usercand. Cut strings may be supplied for leptons          ///
///        of objects, and parameters for calculating isolation from       ///
///        packed PF candidates (which can also have a cut string) can be  ///
///        specified.                                                      ///
///    Based on standard CMSSW LeptonFSRProducer, modified for our needs.  ///
///                                                                        ///
///    Author: Justin Marquez, U. Wisconsin                                ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////


// system include files
#include <memory>
#include <iostream>
#include <cmath> // pow, abs

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"


using pat::PackedCandidate, pat::PackedCandidateRef;
typedef edm::View<PackedCandidate> PackedCandidateView;
using pat::Electron, pat::ElectronCollection;
typedef edm::View<pat::Electron> ElectronView;
using pat::Muon, pat::MuonCollection;
typedef edm::View<pat::Muon> MuonView;

class PATLeptonFSREmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit PATLeptonFSREmbedder(const edm::ParameterSet&);
    ~PATLeptonFSREmbedder();

  private:
    virtual void produce(edm::Event&, const edm::EventSetup&);

    double getIso(const PackedCandidate& photon, const PackedCandidateView& cands);
    bool electronFootprintVeto(const PackedCandidateRef& phoRef, const edm::Handle<ElectronCollection>& electronsForVeto);

    edm::EDGetTokenT<PackedCandidateView> candToken_;
    edm::EDGetTokenT<ElectronView> electronToken_;
    edm::EDGetTokenT<MuonView> muonToken_;
    edm::EDGetTokenT<ElectronCollection> electronVetoToken_;

    std::string fsrLabel_, fsrDREt2Label_;

    const double ptCut_;
    const double etaCut_;
    const double drEtCut_;
    const double deltaRMax_;
    const double isoCut_;
    const double chIsoConeMin_;
    const double chIsoPtCut_;
    const double nIsoConeMin_;
    const double nIsoPtCut_;
    const double isoConeMin_;
    const double isoConeMax_;

    StringCutObjectSelector<Electron> eCut_;
    StringCutObjectSelector<Muon> muCut_;
};


PATLeptonFSREmbedder::PATLeptonFSREmbedder(const edm::ParameterSet& iConfig) :
  candToken_(consumes<PackedCandidateView>(iConfig.getParameter<edm::InputTag>("candidates"))),
  electronToken_(consumes<ElectronView>(iConfig.getParameter<edm::InputTag>("electrons"))),
  muonToken_(consumes<MuonView>(iConfig.getParameter<edm::InputTag>("muons"))),
  electronVetoToken_(consumes<ElectronCollection>(iConfig.exists("electronsForVeto") ?
        iConfig.getParameter<edm::InputTag>("electronsForVeto") :
        edm::InputTag("slimmedElectrons"))),
  fsrLabel_(iConfig.exists("fsrLabel") ?
      iConfig.getParameter<std::string>("fsrLabel") : "dREtFSRCand"),
  fsrDREt2Label_(fsrLabel_+"DREt2"),
  ptCut_(iConfig.exists("ptCut") ?
      iConfig.getParameter<double>("ptCut") : 2.),
  etaCut_(iConfig.exists("etaCut") ?
      iConfig.getParameter<double>("etaCut") : 2.5),
  drEtCut_(iConfig.exists("drEtCut") ?
      iConfig.getParameter<double>("drEtCut") : 0.012),
  deltaRMax_(iConfig.exists("deltaRMax") ?
      iConfig.getParameter<double>("deltaRMax") : 0.5),
  isoCut_(iConfig.exists("isoCut") ?
      iConfig.getParameter<double>("isoCut") : 1.8),
  chIsoConeMin_(iConfig.exists("chIsoConeMin") ?
      iConfig.getParameter<double>("chIsoConeMin") : 0.0001),
  chIsoPtCut_(iConfig.exists("chIsoPtCut") ?
      iConfig.getParameter<double>("chIsoPtCut") : 0.2),
  nIsoConeMin_(iConfig.exists("nIsoConeMin") ?
      iConfig.getParameter<double>("nIsoConeMin") : 0.01),
  nIsoPtCut_(iConfig.exists("nIsoPtCut") ?
      iConfig.getParameter<double>("nIsoPtCut") : 0.5),
  isoConeMin_(std::min(chIsoConeMin_, nIsoConeMin_)),
  isoConeMax_(iConfig.exists("isoConeMax") ?
      iConfig.getParameter<double>("isoConeMax") : 0.3),
  eCut_(iConfig.exists("eCut") ?
      iConfig.getParameter<std::string>("eCut") : ""),
  muCut_(iConfig.exists("muCut") ?
      iConfig.getParameter<std::string>("muCut") : "")
{
  produces<ElectronCollection>("electrons");
  produces<MuonCollection>("muons");
}


PATLeptonFSREmbedder::~PATLeptonFSREmbedder(){}


void PATLeptonFSREmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  // Get input collections
  edm::Handle<PackedCandidateView> cands;
  iEvent.getByToken(candToken_, cands);

  edm::Handle<ElectronView> electronsIn;
  iEvent.getByToken(electronToken_, electronsIn);

  edm::Handle<MuonView> muonsIn;
  iEvent.getByToken(muonToken_, muonsIn);

  edm::Handle<ElectronCollection> electronsForVeto;
  iEvent.getByToken(electronVetoToken_, electronsForVeto);

  // Make output collections
  std::unique_ptr<ElectronCollection> electronsOut(new ElectronCollection());
  for (auto electron = electronsIn->begin(); electron != electronsIn->end(); electron++)
    electronsOut->push_back(*electron);

  std::unique_ptr<MuonCollection> muonsOut(new MuonCollection());
  for (auto muon = muonsIn->begin(); muon != muonsIn->end(); muon++)
    muonsOut->push_back(*muon);

  // Loop over photons
  for (auto photon = cands->begin(); photon != cands->end(); photon++){
    edm::Ptr<PackedCandidate> phoPtr = cands->ptrAt(photon - cands->begin());
    PackedCandidateRef phoRef = cands->refAt(photon - cands->begin()).castTo<PackedCandidateRef>();

    if (std::abs(photon->pdgId()) != 22 || photon->pt() < ptCut_ || std::abs(photon->eta()) > etaCut_)
      continue;

    double deltaRMin = deltaRMax_; //only consider leptons with deltaR less than max
    int closestMu = -1;
    int closestE = -1;
    double iso = 1e9;
    bool skipPhoton = false;

    // Check for closest muon
    for (auto muon = muonsIn->begin(); muon != muonsIn->end(); muon++){
      if (!muCut_(*muon))
        continue;

      double deltaR = reco::deltaR(photon->p4(), muon->p4());
      if (deltaR < deltaRMin && deltaR > isoConeMin_ && deltaR/photon->pt()/photon->pt() < drEtCut_){
        double iso = getIso(*photon, *cands);
        if (iso > isoCut_){
          skipPhoton = true;
          break;
        }

        skipPhoton = electronFootprintVeto(phoRef, electronsForVeto);
        if (skipPhoton)
          break;

        deltaRMin = deltaR;
        closestMu = muon - muonsIn->begin();
      }
    }

    if (skipPhoton) continue;

    // Check for closest electron
    for (auto electron = electronsIn->begin(); electron != electronsIn->end(); electron++){
      if (!eCut_(*electron))
        continue;

      double deltaR = reco::deltaR(photon->p4(), electron->p4());
      if (deltaR < deltaRMin && deltaR > isoConeMin_ && deltaR/photon->pt()/photon->pt() < drEtCut_){
        if (iso > 1e8) iso = getIso(*photon, *cands);
        if (iso > isoCut_)
          break;

        if (electronFootprintVeto(phoRef, electronsForVeto))
          break;

        deltaRMin = deltaR;
        closestE = electron - electronsIn->begin();

        // Electron was closer, reset muon match
        closestMu = -1;
      }
    }

    if (closestMu >= 0 || closestE >= 0){
      double dREt2 = deltaRMin/photon->pt()/photon->pt();

      // Save candidate and dREt2 to closest lepton
      if (closestMu >= 0){
        Muon& muon = muonsOut->at(closestMu);
        if (!muon.hasUserFloat(fsrDREt2Label_) || dREt2 < muon.userFloat(fsrDREt2Label_)){
          muon.addUserCand(fsrLabel_, phoPtr, true);
          muon.addUserFloat(fsrDREt2Label_, dREt2, true);
        }
      }
      else if (closestE >= 0){
        Electron& electron = electronsOut->at(closestE);
        if (!electron.hasUserFloat(fsrDREt2Label_) || dREt2 < electron.userFloat(fsrDREt2Label_)){
          electron.addUserCand(fsrLabel_, phoPtr, true);
          electron.addUserFloat(fsrDREt2Label_, dREt2, true);
        }
      }
    }
  }

  iEvent.put(std::move(electronsOut), "electrons");
  iEvent.put(std::move(muonsOut), "muons");
}

double PATLeptonFSREmbedder::getIso(const PackedCandidate& photon, const PackedCandidateView& cands){
  double ptsum = 0;

  for (const auto& cand : cands){
    double deltaR2 = reco::deltaR2(photon.p4(), cand.p4());
    if (deltaR2 > isoConeMax_ * isoConeMax_ || deltaR2 < isoConeMin_ * isoConeMin_)
      continue;

    //Charged hadrons
    if (cand.charge() != 0 && std::abs(cand.pdgId()) == 211 && cand.pt() > chIsoPtCut_ && deltaR2 > chIsoConeMin_ * chIsoConeMin_)
      ptsum += cand.pt();
    //Neutral hadrons and photons
    else if (cand.charge() == 0 && (std::abs(cand.pdgId()) == 22 || std::abs(cand.pdgId()) == 130) && cand.pt() > nIsoPtCut_ && deltaR2 > nIsoConeMin_ * nIsoConeMin_)
      ptsum += cand.pt();
  }

  return ptsum / photon.pt();
}

bool PATLeptonFSREmbedder::electronFootprintVeto(const PackedCandidateRef& phoRef, const edm::Handle<ElectronCollection>& electronsForVeto){
  for (auto electron = electronsForVeto->begin(); electron != electronsForVeto->end(); electron++){
    for (const auto& cand : electron->associatedPackedPFCandidates()){
      if (!cand.isAvailable())
        continue;

      if (cand.id() != phoRef.id())
        throw cms::Exception("Configuration")
          << "The electron associatedPackedPFCandidates item does not have "
          << "the same ID of packed candidate collection used for cleaning the electron footprint: " << cand.id()
          << " (" << phoRef.id() << ")\n";

      if (cand.key() == phoRef.key())
        return true;
    }
  }

  return false;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATLeptonFSREmbedder);
