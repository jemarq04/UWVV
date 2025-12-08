//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   PATMuonZZIDEmbedder.cc                                                 //
//                                                                          //
//   Embeds muon ID and isolation decisions as userfloats                   //
//       (1 for true, 0 for false), for use in other modules using          //
//       HZZ4l2015 definitions.                                             //
//                                                                          //
//   Author: Nate Woods, U. Wisconsin                                       //
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
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

using pat::Muon, pat::MuonCollection;
typedef edm::View<Muon> MuonView;

class PATMuonZZIDEmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit PATMuonZZIDEmbedder(const edm::ParameterSet&);
    ~PATMuonZZIDEmbedder() {}


  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    bool passKinematics(const Muon& mu) const;
    bool passVertex(const Muon& mu) const;
    bool passType(const Muon& mu) const;

    edm::EDGetTokenT<MuonView> muonCollectionToken_;
    const std::string idLabel_; // label for the decision userfloat
    const edm::EDGetTokenT<reco::VertexCollection> vtxSrcToken_; // primary vertex (for veto PV and SIP cuts)

    const double ptCut;
    const double etaCut;
    const double sipCut;
    const double pvDXYCut;
    const double pvDZCut;

    // MVA Reader
    //MuonGBRForestReader *r;
};


PATMuonZZIDEmbedder::PATMuonZZIDEmbedder(const edm::ParameterSet& iConfig):
  muonCollectionToken_(consumes<MuonView>(iConfig.exists("src") ?
      iConfig.getParameter<edm::InputTag>("src") :
      edm::InputTag("slimmedMuons"))),
  idLabel_(iConfig.exists("idLabel") ?
      iConfig.getParameter<std::string>("idLabel") :
      std::string("HZZ4lIDPass")),
  vtxSrcToken_(consumes<reco::VertexCollection>(iConfig.exists("vtxSrc") ?
      iConfig.getParameter<edm::InputTag>("vtxSrc") :
      edm::InputTag("selectedPrimaryVertex"))),
  ptCut(iConfig.exists("ptCut") ? iConfig.getParameter<double>("ptCut") : 5.),
  etaCut(iConfig.exists("etaCut") ? iConfig.getParameter<double>("etaCut") : 2.4),
  sipCut(iConfig.exists("sipCut") ? iConfig.getParameter<double>("sipCut") : 4.),
  pvDXYCut(iConfig.exists("pvDXYCut") ? iConfig.getParameter<double>("pvDXYCut") : 0.5),
  pvDZCut(iConfig.exists("pvDZCut") ? iConfig.getParameter<double>("pvDZCut") : 1.)
{
  produces<MuonCollection>();
}


void PATMuonZZIDEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<MuonView> muonsIn;
  iEvent.getByToken(muonCollectionToken_, muonsIn);

  std::unique_ptr<MuonCollection> out = std::make_unique<MuonCollection>();

  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vtxSrcToken_, vertices);

  const reco::Vertex& pv = *vertices->begin();

  for(MuonView::const_iterator mi = muonsIn->begin(); mi != muonsIn->end(); mi++) // loop over muons
  {
    out->push_back(*mi); // copy muon to save correctly in event
    Muon& mu = out->back();

    bool vtxResult = vertices->size() && passVertex(mu);
    bool kinResult = passKinematics(mu);
    bool typeResult = passType(mu);
    bool idResultNoVtx = kinResult && typeResult;
    bool idResult = idResultNoVtx && vtxResult;

    mu.addUserFloat(idLabel_, float(idResult)); // 1 for true, 0 for false
    mu.addUserFloat(idLabel_+"NoVtx", float(idResultNoVtx)); // 1 for true, 0 for false

    mu.addUserFloat(idLabel_+"PF", float(idResult && mi->isPFMuon())); // 1 for true, 0 for false
    mu.addUserFloat(idLabel_+"PFNoVtx", float(idResultNoVtx && mi->isPFMuon())); // 1 for true, 0 for false

    bool trackerHighPtID = mi->passed(reco::Muon::CutBasedIdTrkHighPt) && mi->pt() > 200.;
    mu.addUserFloat(idLabel_+"HighPt", float(idResult && trackerHighPtID));
    mu.addUserFloat(idLabel_+"HighPtNoVtx", float(idResultNoVtx && trackerHighPtID));

    // PAS2019 version of TightMuonID (both PASID and Tight store the same result)
    mu.addUserFloat(idLabel_+"Tight", float(idResult && (mi->isPFMuon() || trackerHighPtID)));//PAS2019 version of TightMuonID
    mu.addUserFloat(idLabel_+"TightNoVtx", float(idResultNoVtx && (mi->isPFMuon() || trackerHighPtID)));//PAS2019 version of TightMuonID

    //Now both electrons and muons have BDT for ZZTightID and thats how its stored in "leptonBranches"
    //Some cut-based IDs for validation with other frameworks if needed
    mu.addUserInt("isTightMuon",mi->isTightMuon(pv));
    mu.addUserInt("CutBasedIdLoose",mi->passed(reco::Muon::CutBasedIdLoose));
    mu.addUserInt("CutBasedIdMedium",mi->passed(reco::Muon::CutBasedIdMedium));
    mu.addUserInt("CutBasedIdTight",mi->passed(reco::Muon::CutBasedIdTight));
    mu.addUserInt("PFIsoLoose",mi->passed(reco::Muon::PFIsoLoose));
    mu.addUserInt("PFIsoMedium",mi->passed(reco::Muon::PFIsoMedium));
    mu.addUserInt("PFIsoTight",mi->passed(reco::Muon::PFIsoTight));
    mu.addUserInt("PFIsoVeryTight",mi->passed(reco::Muon::PFIsoVeryTight));
  }

  iEvent.put(std::move(out));
}

bool PATMuonZZIDEmbedder::passKinematics(const Muon& mu) const
{
  return mu.pt() > ptCut && fabs(mu.eta()) < etaCut;
}


bool PATMuonZZIDEmbedder::passVertex(const Muon& mu) const
{
  return fabs(mu.dB(Muon::PV3D)/mu.edB(Muon::PV3D)) < sipCut &&
    fabs(mu.dB(Muon::PV2D)) < pvDXYCut && fabs(mu.dB(Muon::PVDZ)) < pvDZCut;
}


bool PATMuonZZIDEmbedder::passType(const Muon& mu) const
{
  // Global muon or (arbitrated) tracker muon
  return (mu.isGlobalMuon() || (mu.isTrackerMuon() && mu.numberOfMatchedStations() > 0));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATMuonZZIDEmbedder);
