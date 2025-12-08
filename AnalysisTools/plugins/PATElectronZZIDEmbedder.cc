//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   PATElectronZZIDEmbedder.cc                                             //
//                                                                          //
//   Embeds electron ID decisions as userfloats                             //
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
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"

using pat::Electron, pat::ElectronCollection;
typedef edm::View<Electron> ElectronView;

class PATElectronZZIDEmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit PATElectronZZIDEmbedder(const edm::ParameterSet&);
    ~PATElectronZZIDEmbedder() {}

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    bool passKinematics(const Electron& ele) const;
    bool passVertex(const Electron& ele) const;
    bool passBDT(const Electron& ele) const;
    bool passMissingHits(const Electron& ele) const;

    edm::EDGetTokenT<ElectronView> srcToken_;
    const std::string idLabel_; // label for the decision userfloat
    const std::string isoLabel_;
    const edm::EDGetTokenT<reco::VertexCollection> vtxSrcToken_; // primary vertex (for veto PV and SIP cuts)

    const double ptCut;
    const double etaCut;
    const double sipCut;
    const double pvDXYCut;
    const double pvDZCut;
    const double idPtThr;
    const double idEtaThrLow;
    const double idEtaThrHigh;
    const double idCutLowPtLowEta;
    const double idCutLowPtMedEta;
    const double idCutLowPtHighEta;
    const double idCutHighPtLowEta;
    const double idCutHighPtMedEta;
    const double idCutHighPtHighEta;
    const std::string bdtLabel;
    const std::string mvaLabel;
    const bool useMVA;
    const std::string cutBasedLabel;
    //const std::string HZZWP;
    const int missingHitsCut;

    StringCutObjectSelector<Electron> selector;
};

PATElectronZZIDEmbedder::PATElectronZZIDEmbedder(const edm::ParameterSet& iConfig):
  srcToken_(consumes<ElectronView>(iConfig.exists("src") ?
      iConfig.getParameter<edm::InputTag>("src") :
      edm::InputTag("slimmedElectrons"))),
  idLabel_(iConfig.exists("idLabel") ?
      iConfig.getParameter<std::string>("idLabel") :
      std::string("HZZ4lIDPass")),
  isoLabel_(iConfig.exists("isoLabel") ?
      iConfig.getParameter<std::string>("isoLabel") :
      std::string("HZZ4lIsoPass")),
  vtxSrcToken_(consumes<reco::VertexCollection>(iConfig.exists("vtxSrc") ?
      iConfig.getParameter<edm::InputTag>("vtxSrc") :
      edm::InputTag("selectedPrimaryVertex"))),
  ptCut(iConfig.exists("ptCut") ? iConfig.getParameter<double>("ptCut") : 7.),
  etaCut(iConfig.exists("etaCut") ? iConfig.getParameter<double>("etaCut") : 2.5),
  sipCut(iConfig.exists("sipCut") ? iConfig.getParameter<double>("sipCut") : 4.),
  pvDXYCut(iConfig.exists("pvDXYCut") ? iConfig.getParameter<double>("pvDXYCut") : 0.5),
  pvDZCut(iConfig.exists("pvDZCut") ? iConfig.getParameter<double>("pvDZCut") : 1.),
  idPtThr(iConfig.exists("idPtThr") ? iConfig.getParameter<double>("idPtThr") : 10.),
  idEtaThrLow(iConfig.exists("idEtaThrLow") ? iConfig.getParameter<double>("idEtaThrLow") : 0.8),
  idEtaThrHigh(iConfig.exists("idEtaThrHigh") ? iConfig.getParameter<double>("idEtaThrHigh") : 1.479),
  idCutLowPtLowEta(iConfig.exists("idCutLowPtLowEta") ? iConfig.getParameter<double>("idCutLowPtLowEta") : 0.9044286167), //EB1_5
  idCutLowPtMedEta(iConfig.exists("idCutLowPtMedEta") ? iConfig.getParameter<double>("idCutLowPtMedEta") : 0.9094166886), //EB2_5
  idCutLowPtHighEta(iConfig.exists("idCutLowPtHighEta") ? iConfig.getParameter<double>("idCutLowPtHighEta") : 0.9443653660), //EE_5
  idCutHighPtLowEta(iConfig.exists("idCutHighPtLowEta") ? iConfig.getParameter<double>("idCutHighPtLowEta") : 0.1968600840), //EB1_10
  idCutHighPtMedEta(iConfig.exists("idCutHighPtMedEta") ? iConfig.getParameter<double>("idCutHighPtMedEta") : 0.0759172100), //EB2_10
  idCutHighPtHighEta(iConfig.exists("idCutHighPtHighEta") ? iConfig.getParameter<double>("idCutHighPtHighEta") : -0.5169136775), //EE_10
  bdtLabel(iConfig.exists("bdtLabel") ? iConfig.getParameter<std::string>("bdtLabel") : "ElectronMVAEstimatorRun2Summer18ULIdIsoValues"),
  mvaLabel(iConfig.exists("mvaLabel") ? iConfig.getParameter<std::string>("mvaLabel") : "mvaEleID-Winter22-HZZ-V1"),
  useMVA(iConfig.exists("useMVA") ? iConfig.getParameter<bool>("useMVA") : true),
  cutBasedLabel(iConfig.exists("cutBasedLabel") ? iConfig.getParameter<std::string>("cutBasedLabel") : "cutBasedElectronID-RunIIIWinter22-V1"),
  missingHitsCut(iConfig.exists("missingHitsCut") ? iConfig.getParameter<int>("missingHitsCut") : 1),
  selector(iConfig.exists("selection") ? iConfig.getParameter<std::string>("selection") : "")
{
  produces<ElectronCollection>();
}


void PATElectronZZIDEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<ElectronView> electronsIn;
  iEvent.getByToken(srcToken_, electronsIn);

  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vtxSrcToken_,vertices);

  std::unique_ptr<ElectronCollection> out(new ElectronCollection());

  for(ElectronView::const_iterator ei = electronsIn->begin(); ei != electronsIn->end(); ei++)
  {
    out->push_back(*ei); // copy electron to save correctly in event
    Electron& ele = out->back();

    bool vtxResult = vertices->size() && passVertex(ele);
    bool kinResult = passKinematics(ele);
    bool missingHitsResult = passMissingHits(ele);
    bool idResultNoVtx = selector(ele) && kinResult && missingHitsResult;
    bool idResult = idResultNoVtx && vtxResult;

    ele.addUserFloat(idLabel_+"NoVtx", float(idResultNoVtx)); // 1 for true, 0 for false
    ele.addUserFloat(idLabel_, float(idResult)); // 1 for true, 0 for false

    bool bdtID = useMVA? ele.electronID(mvaLabel) : passBDT(ele);
    ele.addUserFloat(idLabel_+"TightNoVtx", float(idResultNoVtx && bdtID)); // 1 for true, 0 for false
    ele.addUserFloat(idLabel_+"Tight", float(idResult && bdtID)); // 1 for true, 0 for false

    //Also add some cut-based Run2 electron IDs for validation
    ele.addUserFloat("Loose",float(ele.electronID((cutBasedLabel + "-loose").c_str())));
    ele.addUserFloat("Medium",float(ele.electronID((cutBasedLabel + "-medium").c_str())));
    ele.addUserFloat("Tight",float(ele.electronID((cutBasedLabel + "-tight").c_str())));
    ele.addUserFloat("Veto",float(ele.electronID((cutBasedLabel + "-veto").c_str())));
  }

  iEvent.put(std::move(out));
}

bool PATElectronZZIDEmbedder::passKinematics(const Electron& ele) const
{
  return ele.pt() > ptCut && fabs(ele.eta()) < etaCut;
}


bool PATElectronZZIDEmbedder::passVertex(const Electron& ele) const
{
  return (fabs(ele.dB(Electron::PV3D)/ele.edB(Electron::PV3D)) < sipCut &&
          fabs(ele.dB(Electron::PV2D)) < pvDXYCut &&
          fabs(ele.dB(Electron::PVDZ)) < pvDZCut);
}


bool PATElectronZZIDEmbedder::passBDT(const Electron& ele) const
{
  if (bdtLabel == "") return true;

  double pt = ele.pt();
  double eta = fabs(ele.superCluster()->eta());

  double bdtCut;
  if(pt < idPtThr)
  {
    if(eta < idEtaThrLow)
      bdtCut = idCutLowPtLowEta;
    else if(eta < idEtaThrHigh)
      bdtCut = idCutLowPtMedEta;
    else
      bdtCut = idCutLowPtHighEta;
  }
  else
  {
    if(eta < idEtaThrLow)
      bdtCut = idCutHighPtLowEta;
    else if(eta < idEtaThrHigh)
      bdtCut = idCutHighPtMedEta;
    else
      bdtCut = idCutHighPtHighEta;
  }

  return ele.userFloat(bdtLabel) > bdtCut;
}


bool PATElectronZZIDEmbedder::passMissingHits(const Electron& ele) const
{
  return ele.gsfTrack()->hitPattern().numberOfAllHits(reco::HitPattern::MISSING_INNER_HITS) <= missingHitsCut;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronZZIDEmbedder);
