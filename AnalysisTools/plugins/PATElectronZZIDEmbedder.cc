//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   PATElectronZZIDEmbedder.cc                                         //
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
#include "FWCore/Framework/interface/MakerMacros.h"
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


class PATElectronZZIDEmbedder : public edm::stream::EDProducer<>
{
public:
  explicit PATElectronZZIDEmbedder(const edm::ParameterSet&);
  ~PATElectronZZIDEmbedder() {}


private:
  // Methods
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  bool passKinematics(const edm::Ptr<pat::Electron>& elec) const;
  bool passVertex(const edm::Ptr<pat::Electron>& elec) const;
  bool passBDT(const edm::Ptr<pat::Electron>& elec) const;
  bool passMissingHits(const edm::Ptr<pat::Electron>& elec) const;

  // Data
  edm::EDGetTokenT<edm::View<pat::Electron> > electronCollectionToken_;
  const std::string idLabel_; // label for the decision userfloat
  const std::string isoLabel_;
  const edm::EDGetTokenT<reco::VertexCollection> vtxSrcToken_; // primary vertex (for veto PV and SIP cuts)
  edm::Handle<reco::VertexCollection> vertices;

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
  //const std::string HZZWP;
  const int missingHitsCut;
  const bool checkMVAID;

  StringCutObjectSelector<pat::Electron> selector;
};


// Constructors and destructors

PATElectronZZIDEmbedder::PATElectronZZIDEmbedder(const edm::ParameterSet& iConfig):
  electronCollectionToken_(consumes<edm::View<pat::Electron> >(iConfig.exists("src") ?
                                                               iConfig.getParameter<edm::InputTag>("src") :
                                                               edm::InputTag("slimmedElectrons"))),
  idLabel_(iConfig.exists("idLabel") ?
	   iConfig.getParameter<std::string>("idLabel") :
	   std::string("HZZ4lIDPass")),
  isoLabel_(iConfig.exists("isoLabel") ?
	   iConfig.getParameter<std::string>("isoLabel") :
	   std::string("HZZ4lIsoPass")),
  vtxSrcToken_(consumes<reco::VertexCollection>(iConfig.exists("vtxSrc") ? iConfig.getParameter<edm::InputTag>("vtxSrc") : edm::InputTag("selectedPrimaryVertex"))),
  ptCut(iConfig.exists("ptCut") ? iConfig.getParameter<double>("ptCut") : 7.),
  etaCut(iConfig.exists("etaCut") ? iConfig.getParameter<double>("etaCut") : 2.5),
  sipCut(iConfig.exists("sipCut") ? iConfig.getParameter<double>("sipCut") : 10.),
  pvDXYCut(iConfig.exists("pvDXYCut") ? iConfig.getParameter<double>("pvDXYCut") : 0.5),
  pvDZCut(iConfig.exists("pvDZCut") ? iConfig.getParameter<double>("pvDZCut") : 1.),
  idPtThr(iConfig.exists("idPtThr") ? iConfig.getParameter<double>("idPtThr") : 10.),
  idEtaThrLow(iConfig.exists("idEtaThrLow") ? iConfig.getParameter<double>("idEtaThrLow") : 0.8),
  idEtaThrHigh(iConfig.exists("idEtaThrHigh") ? iConfig.getParameter<double>("idEtaThrHigh") : 1.479),
  idCutLowPtLowEta(iConfig.exists("idCutLowPtLowEta") ? iConfig.getParameter<double>("idCutLowPtLowEta") : -0.586),
  idCutLowPtMedEta(iConfig.exists("idCutLowPtMedEta") ? iConfig.getParameter<double>("idCutLowPtMedEta") : -0.712),
  idCutLowPtHighEta(iConfig.exists("idCutLowPtHighEta") ? iConfig.getParameter<double>("idCutLowPtHighEta") : -0.662),
  idCutHighPtLowEta(iConfig.exists("idCutHighPtLowEta") ? iConfig.getParameter<double>("idCutHighPtLowEta") : 0.652),
  idCutHighPtMedEta(iConfig.exists("idCutHighPtMedEta") ? iConfig.getParameter<double>("idCutHighPtMedEta") : 0.701),
  idCutHighPtHighEta(iConfig.exists("idCutHighPtHighEta") ? iConfig.getParameter<double>("idCutHighPtHighEta") : 0.350),
  bdtLabel(iConfig.exists("bdtLabel") ? iConfig.getParameter<std::string>("bdtLabel") : "ElectronMVAEstimatorRun2Fall17IsoV2Values"),
  missingHitsCut(iConfig.exists("missingHitsCut") ? iConfig.getParameter<int>("missingHitsCut") : 1),
  checkMVAID(bdtLabel != ""),
  selector(iConfig.exists("selection") ?
	    iConfig.getParameter<std::string>("selection") :
	    "")
{
  produces<std::vector<pat::Electron> >();
}


void PATElectronZZIDEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::unique_ptr<std::vector<pat::Electron> >out = std::make_unique<std::vector<pat::Electron> >();

  edm::Handle<edm::View<pat::Electron> > electronsIn;
  iEvent.getByToken(vtxSrcToken_,vertices);

  iEvent.getByToken(electronCollectionToken_, electronsIn);

  for(edm::View<pat::Electron>::const_iterator ei = electronsIn->begin();
      ei != electronsIn->end(); ei++) // loop over electrons
    {
      const edm::Ptr<pat::Electron> eptr(electronsIn, ei - electronsIn->begin());

      out->push_back(*ei); // copy electron to save correctly in event

      bool vtxResult = passVertex(eptr);
      bool kinResult = passKinematics(eptr);
      bool missingHitsResult = passMissingHits(eptr);
      bool idResultNoVtx = selector(*eptr) && kinResult && missingHitsResult;
      bool idResult = idResultNoVtx && vtxResult;
      
      out->back().addUserFloat(idLabel_+"NoVtx", float(idResultNoVtx)); // 1 for true, 0 for false
      out->back().addUserFloat(idLabel_, float(idResult)); // 1 for true, 0 for false

      //std::cout << iEvent.id().run() << ":" << iEvent.id().luminosityBlock() <<":"
      //        << iEvent.id().event() << std::endl;
      out->back().addUserFloat(idLabel_+"TightNoVtx", float(idResultNoVtx && passBDT(eptr))); // 1 for true, 0 for false
      out->back().addUserFloat(idLabel_+"Tight", float(idResult && passBDT(eptr))); // 1 for true, 0 for false

      //Also add some cut-based Run2 electron IDs for validation
      out->back().addUserFloat("Loose",float(ei->electronID("cutBasedElectronID-Fall17-94X-V2-loose")));
      out->back().addUserFloat("Medium",float(ei->electronID("cutBasedElectronID-Fall17-94X-V2-medium")));
      out->back().addUserFloat("Tight",float(ei->electronID("cutBasedElectronID-Fall17-94X-V2-tight")));
      out->back().addUserFloat("Veto",float(ei->electronID("cutBasedElectronID-Fall17-94X-V2-veto")));
      //-- Scale and smearing corrections are now stored in the miniAOD https://twiki.cern.ch/twiki/bin/view/CMS/EgammaMiniAODV2#Energy_Scale_and_Smearing
      float uncorrected_pt = ei->pt();
      float corr_factor = ei->userFloat("ecalTrkEnergyPostCorr") / ei->energy();//get scale/smear correction factor directly from miniAOD       
      //scale and smear electron
      out->back().setP4(reco::Particle::PolarLorentzVector(uncorrected_pt*corr_factor, ei->eta(), ei->phi(), ei->mass()*corr_factor));
      out->back().addUserFloat("uncorrected_pt",uncorrected_pt);
      //get all scale uncertainties and their breakdown
      float scale_total_up = ei->userFloat("energyScaleUp") / ei->energy();
      float scale_stat_up = ei->userFloat("energyScaleStatUp") / ei->energy();
      float scale_syst_up = ei->userFloat("energyScaleSystUp") / ei->energy();
      float scale_gain_up = ei->userFloat("energyScaleGainUp") / ei->energy();
      float scale_total_dn = ei->userFloat("energyScaleDown") / ei->energy();
      float scale_stat_dn = ei->userFloat("energyScaleStatDown") / ei->energy();
      float scale_syst_dn = ei->userFloat("energyScaleSystDown") / ei->energy();
      float scale_gain_dn = ei->userFloat("energyScaleGainDown") / ei->energy();
      //get all smearing uncertainties and their breakdown
      float sigma_total_up = ei->userFloat("energySigmaUp") / ei->energy();
      float sigma_rho_up = ei->userFloat("energySigmaRhoUp") / ei->energy();
      float sigma_phi_up = ei->userFloat("energySigmaPhiUp") / ei->energy();
      float sigma_total_dn = ei->userFloat("energySigmaDown") / ei->energy();
      float sigma_rho_dn = ei->userFloat("energySigmaRhoDown") / ei->energy();
      float sigma_phi_dn = ei->userFloat("energySigmaPhiDown") / ei->energy();
      
      out->back().addUserFloat("scale_total_up",scale_total_up);
      out->back().addUserFloat("scale_stat_up",scale_stat_up);
      out->back().addUserFloat("scale_syst_up",scale_syst_up);
      out->back().addUserFloat("scale_gain_up",scale_gain_up);
      out->back().addUserFloat("scale_total_dn",scale_total_dn);
      out->back().addUserFloat("scale_stat_dn",scale_stat_dn);
      out->back().addUserFloat("scale_syst_dn",scale_syst_dn);
      out->back().addUserFloat("scale_gain_dn",scale_gain_dn);
      out->back().addUserFloat("sigma_total_up",sigma_total_up);
      out->back().addUserFloat("sigma_total_dn",sigma_total_dn);
      out->back().addUserFloat("sigma_rho_up",sigma_rho_up);
      out->back().addUserFloat("sigma_rho_dn",sigma_rho_dn);
      out->back().addUserFloat("sigma_phi_up",sigma_phi_up);
      out->back().addUserFloat("sigma_phi_dn",sigma_phi_dn);
    }

  iEvent.put(std::move(out));
}


bool PATElectronZZIDEmbedder::passKinematics(const edm::Ptr<pat::Electron>& elec) const
{
  return elec->pt() > ptCut && fabs(elec->eta()) < etaCut;
}


bool PATElectronZZIDEmbedder::passVertex(const edm::Ptr<pat::Electron>& elec) const
{
  if(!vertices->size())
    return false;

  return (fabs(elec->dB(pat::Electron::PV3D))/elec->edB(pat::Electron::PV3D) < sipCut &&
          fabs(elec->dB(pat::Electron::PV2D)) < pvDXYCut &&
          fabs(elec->dB(pat::Electron::PVDZ)) < pvDZCut);
}


bool PATElectronZZIDEmbedder::passBDT(const edm::Ptr<pat::Electron>& elec) const
{
  if(!checkMVAID)
    return true;

  double pt = elec->pt();
  double eta = fabs(elec->superCluster()->eta());

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

  return (elec->userFloat(bdtLabel) > bdtCut);
}


bool PATElectronZZIDEmbedder::passMissingHits(const edm::Ptr<pat::Electron>& elec) const
{
  return (elec->gsfTrack()->hitPattern().numberOfAllHits(reco::HitPattern::MISSING_INNER_HITS) <= missingHitsCut);
}


//define this as a plug-in
DEFINE_FWK_MODULE(PATElectronZZIDEmbedder);








