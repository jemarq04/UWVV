//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   PATElectronCorrector.cc                                                //
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
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"

#include "TRandom3.h"
#include "correction.h"


class PATElectronCorrector : public edm::stream::EDProducer<>{
public:
  explicit PATElectronCorrector(const edm::ParameterSet&);
  ~PATElectronCorrector() {}
private:
  // Methods
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  // Data
  edm::EDGetTokenT<edm::View<pat::Electron> > electronCollectionToken_;

  bool isMC_;
  std::string scaleFileName_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_;
};


// Constructors and destructors

PATElectronCorrector::PATElectronCorrector(const edm::ParameterSet& iConfig):
  electronCollectionToken_(consumes<edm::View<pat::Electron> >(iConfig.exists("src") ?
                                                               iConfig.getParameter<edm::InputTag>("src") :
                                                               edm::InputTag("slimmedElectrons"))),
  isMC_(iConfig.getParameter<bool>("isMC")),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile"))
{
  try{
    scaleFile_ = correction::CorrectionSet::from_file(scaleFileName_);
  }
  catch (...){
    throw cms::Exception("Invalid POG file") << "Filepath: " << scaleFileName_;
  }
  produces<std::vector<pat::Electron> >();
}


void PATElectronCorrector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::unique_ptr<std::vector<pat::Electron> > out = std::make_unique<std::vector<pat::Electron> >();

  edm::Handle<edm::View<pat::Electron> > electronsIn;
  iEvent.getByToken(electronCollectionToken_, electronsIn);

  for(edm::View<pat::Electron>::const_iterator ei = electronsIn->begin();
      ei != electronsIn->end(); ei++) // loop over electrons
  {
    out->push_back(*ei); // copy electron to save correctly in event
    
    float rho = 0, err_rho = 0;
    float scale = 1, err_scale = 0;
    if (isMC_){
      rho     = scaleFile_->at("Smearing")->evaluate({"rho", ei->eta(), ei->r9()});
      err_rho = scaleFile_->at("Smearing")->evaluate({"err_rho", ei->eta(), ei->r9()});
    }
    else{
      scale     = scaleFile_->at("Scale")->evaluate({"total_correction", ei->userInt("seedGain"), (double)iEvent.run(), ei->eta(), ei->r9(), ei->pt()});
      err_scale = scaleFile_->at("Scale")->evaluate({"total_uncertainty", ei->userInt("seedGain"), (double)iEvent.run(), ei->eta(), ei->r9(), ei->pt()});
    }
    float uncorrected_pt = ei->pt();

    TRandom3 rand;
    rand.SetSeed(std::abs(static_cast<int>(std::sin(ei->phi())*100000)));
    float smear = rand.Gaus(1., rho);
    float smear_up = rand.Gaus(1., rho+err_rho);
    float smear_dn = rand.Gaus(1., rho-err_rho);

    out->back().addUserFloat("uncorrected_pt", uncorrected_pt);
    out->back().setP4(reco::Particle::PolarLorentzVector(uncorrected_pt*smear*scale, ei->eta(), ei->phi(), ei->mass()*smear*scale));

    // Custom user floats to save scale and smearing
    out->back().addUserFloat("energyScaleValue", scale);
    out->back().addUserFloat("energyScaleUp", 1+err_scale);
    out->back().addUserFloat("energyScaleDn", 1-err_scale);
    out->back().addUserFloat("energySmearValue", smear);
    out->back().addUserFloat("energySmearUp", smear_up);
    out->back().addUserFloat("energySmearDn", smear_dn);

    //TODO: Determine if the following are necessary (written to ntuple but never accessed in VVAnalysis - not sure how to re-create the values
    //manually)
    //  referenced from electron ZZ ID embedder
    /*
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
    */
  }

  iEvent.put(std::move(out));
}


//define this as a plug-in
DEFINE_FWK_MODULE(PATElectronCorrector);
