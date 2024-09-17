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
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
//#include "MuonMVAReader/Reader/interface/MuonGBRForestReader.hpp"

class PATMuonZZIDEmbedder : public edm::stream::EDProducer<>
{
public:
  explicit PATMuonZZIDEmbedder(const edm::ParameterSet&);
  ~PATMuonZZIDEmbedder() {}


private:
  // Methods
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  bool passKinematics(const edm::Ptr<pat::Muon>& mu) const;
  bool passVertex(const edm::Ptr<pat::Muon>& mu) const;
  bool passType(const edm::Ptr<pat::Muon>& mu) const;
  bool passTrackerHighPtID(const edm::Ptr<pat::Muon>& mu) const;
  bool passBDT(const edm::Ptr<pat::Muon>& mu) const; //Muon MVA for Run 2


  // Data
  edm::EDGetTokenT<edm::View<pat::Muon> > muonCollectionToken_;
  const std::string idLabel_; // label for the decision userfloat
  const std::string isoLabel_;
  const edm::EDGetTokenT<reco::VertexCollection> vtxSrcToken_; // primary vertex (for veto PV and SIP cuts)
  edm::Handle<reco::VertexCollection> vertices;
  edm::EDGetTokenT<double> rhoToken_;
  edm::Handle<double> rhoHandle;
  const int  setup_;

  const double ptCut;
  const double etaCut;
  const double sipCut;
  const double pvDXYCut;
  const double pvDZCut;

  // MVA Reader
  //MuonGBRForestReader *r;

};


// Constructors and destructors

PATMuonZZIDEmbedder::PATMuonZZIDEmbedder(const edm::ParameterSet& iConfig):
  muonCollectionToken_(consumes<edm::View<pat::Muon> >(iConfig.exists("src") ?
						       iConfig.getParameter<edm::InputTag>("src") :
						       edm::InputTag("slimmedMuons"))),
  idLabel_(iConfig.exists("idLabel") ?
	   iConfig.getParameter<std::string>("idLabel") :
	   std::string("HZZ4lIDPass")),
  isoLabel_(iConfig.exists("isoLabel") ?
	   iConfig.getParameter<std::string>("isoLabel") :
	   std::string("HZZ4lIsoPass")),
  vtxSrcToken_(consumes<reco::VertexCollection>(iConfig.exists("vtxSrc") ?
                                                iConfig.getParameter<edm::InputTag>("vtxSrc") :
                                                edm::InputTag("selectedPrimaryVertex"))),
  rhoToken_(consumes<double>(iConfig.exists("rhoSrc") ?
                                                iConfig.getParameter<edm::InputTag>("rhoSrc") :
                                                edm::InputTag("fixedGridRhoFastjetAll"))),
  //Which year lepton setup for MuonGBRForestReader
  setup_(iConfig.exists("setup") ?
	   iConfig.getParameter<int>("setup") : 2022),
  ptCut(iConfig.exists("ptCut") ? iConfig.getParameter<double>("ptCut") : 5.),
  etaCut(iConfig.exists("etaCut") ? iConfig.getParameter<double>("etaCut") : 2.4),
  sipCut(iConfig.exists("sipCut") ? iConfig.getParameter<double>("sipCut") : 4.),
  pvDXYCut(iConfig.exists("pvDXYCut") ? iConfig.getParameter<double>("pvDXYCut") : 0.5),
  pvDZCut(iConfig.exists("pvDZCut") ? iConfig.getParameter<double>("pvDZCut") : 1.)
{
  produces<std::vector<pat::Muon> >();
}


void PATMuonZZIDEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::unique_ptr<std::vector<pat::Muon> > out = std::make_unique<std::vector<pat::Muon> >();

  edm::Handle<edm::View<pat::Muon> > muonsIn;
  iEvent.getByToken(muonCollectionToken_, muonsIn);

  iEvent.getByToken(vtxSrcToken_,vertices);
  
  const reco::Vertex& pv = *vertices->begin();
  
  iEvent.getByToken(rhoToken_, rhoHandle);

  for(edm::View<pat::Muon>::const_iterator mi = muonsIn->begin();
      mi != muonsIn->end(); mi++) // loop over muons
    {
      const edm::Ptr<pat::Muon> mptr(muonsIn, mi - muonsIn->begin());

      out->push_back(*mi); // copy muon to save correctly in event

      bool vtxResult = passVertex(mptr);
      bool kinResult = passKinematics(mptr);
      bool typeResult = passType(mptr);
      bool idResultNoVtx = kinResult && typeResult;
      bool idResult = idResultNoVtx && vtxResult;
      out->back().addUserFloat(idLabel_, float(idResult)); // 1 for true, 0 for false
      out->back().addUserFloat(idLabel_+"NoVtx", float(idResultNoVtx)); // 1 for true, 0 for false

      out->back().addUserFloat(idLabel_+"PF", float(idResult && mi->isPFMuon())); // 1 for true, 0 for false
      out->back().addUserFloat(idLabel_+"PFNoVtx", float(idResultNoVtx && mi->isPFMuon())); // 1 for true, 0 for false

      //bool trackerHighPtID = passTrackerHighPtID(mptr);
      bool trackerHighPtID = mi->passed(reco::Muon::CutBasedIdTrkHighPt) && mi->pt() > 200.;

      //Some Indepenent Muon IDs including the PAS2019 version of TightMuonID
      out->back().addUserFloat(idLabel_+"HighPt", float(idResult && trackerHighPtID));
      out->back().addUserFloat(idLabel_+"PASID", float(idResult && (mi->isPFMuon() || trackerHighPtID)));//PAS2019 version of TightMuonID
      out->back().addUserFloat(idLabel_+"HighPtNoVtx", float(idResultNoVtx && trackerHighPtID));
      out->back().addUserFloat(idLabel_+"PASIDNoVtx", float(idResultNoVtx && (mi->isPFMuon() || trackerHighPtID)));//PAS2019 version of TightMuonID

      //replace tight id BDT with PAS2019 id for UL sample for now
      out->back().addUserFloat(idLabel_+"Tight", float(idResult && (mi->isPFMuon() || trackerHighPtID)));//PAS2019 version of TightMuonID
      out->back().addUserFloat(idLabel_+"TightNoVtx", float(idResultNoVtx && (mi->isPFMuon() || trackerHighPtID)));//PAS2019 version of TightMuonID
      
      //Now both electrons and muons have BDT for ZZTightID and thats how its stored in "leptonBranches"
      //out->back().addUserFloat(idLabel_+"TightNoVtx", float(idResultNoVtx && (passBDT(mptr) || trackerHighPtID))); // 1 for true, 0 for false
      //out->back().addUserFloat(idLabel_+"Tight", float(idResult && (passBDT(mptr) || trackerHighPtID))); // 1 for true, 0 for false
      //Some cut-based IDs for validation with other frameworks if needed 
      out->back().addUserInt("isTightMuon",mi->isTightMuon(pv));
      out->back().addUserInt("CutBasedIdLoose",mi->passed(reco::Muon::CutBasedIdLoose));
      out->back().addUserInt("CutBasedIdMedium",mi->passed(reco::Muon::CutBasedIdMedium));
      //out->back().addUserInt("CutBasedIdMediumPrompt",mi->passed(reco::Muon::CutBasedIdMediumPrompt));
      out->back().addUserInt("CutBasedIdTight",mi->passed(reco::Muon::CutBasedIdTight));
      //out->back().addUserInt("CutBasedIdGlobalHighPt",mi->passed(reco::Muon::CutBasedIdGlobalHighPt));
      //out->back().addUserInt("CutBasedIdTrkHighPt",mi->passed(reco::Muon::CutBasedIdTrkHighPt));
      //out->back().addUserInt("PFIsoVeryLoose",mi->passed(reco::Muon::PFIsoVeryLoose));
      out->back().addUserInt("PFIsoLoose",mi->passed(reco::Muon::PFIsoLoose));
      out->back().addUserInt("PFIsoMedium",mi->passed(reco::Muon::PFIsoMedium));
      out->back().addUserInt("PFIsoTight",mi->passed(reco::Muon::PFIsoTight));
      out->back().addUserInt("PFIsoVeryTight",mi->passed(reco::Muon::PFIsoVeryTight));
      //out->back().addUserInt("TkIsoLoose",mi->passed(reco::Muon::TkIsoLoose));
      //out->back().addUserInt("TkIsoTight",mi->passed(reco::Muon::TkIsoTight));
      //out->back().addUserInt("SoftCutBasedId",mi->passed(reco::Muon::SoftCutBasedId));
    }

  iEvent.put(std::move(out));
}

bool PATMuonZZIDEmbedder::passBDT(const edm::Ptr<pat::Muon>& mu) const
{
  return true; //Use PASid for muon instead
}
bool PATMuonZZIDEmbedder::passKinematics(const edm::Ptr<pat::Muon>& mu) const
{
  bool result = (mu->pt() > ptCut);
  result = (result && fabs(mu->eta()) < etaCut);
  return result;
}


bool PATMuonZZIDEmbedder::passVertex(const edm::Ptr<pat::Muon>& mu) const
{
  if(!vertices->size())
    return false;

  return fabs(mu->dB(pat::Muon::PV3D))/mu->edB(pat::Muon::PV3D) < sipCut &&
	  (fabs(mu->muonBestTrack()->dxy(vertices->at(0).position())) < pvDXYCut &&
	  fabs(mu->muonBestTrack()->dz(vertices->at(0).position())) < pvDZCut);
}


bool PATMuonZZIDEmbedder::passType(const edm::Ptr<pat::Muon>& mu) const
{
  // Global muon or (arbitrated) tracker muon
  return (mu->isGlobalMuon() || (mu->isTrackerMuon() && mu->numberOfMatchedStations() > 0));
}


bool PATMuonZZIDEmbedder::passTrackerHighPtID(const edm::Ptr<pat::Muon>& mu) const
{
  if(!vertices->size())
    return false;

  return (mu->pt() > 200. &&
          mu->isTrackerMuon() &&
          mu->numberOfMatchedStations() > 1 &&
          mu->dB() < 0.2 &&
          fabs(mu->muonBestTrack()->dz(vertices->at(0).position())) < 0.5 &&
          mu->innerTrack()->hitPattern().numberOfValidPixelHits() > 0 &&
          mu->innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 &&
          mu->muonBestTrack()->ptError() / mu->muonBestTrack()->pt() < 0.3);
}


//define this as a plug-in
DEFINE_FWK_MODULE(PATMuonZZIDEmbedder);








