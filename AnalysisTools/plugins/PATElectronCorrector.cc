//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   PATElectronCorrector.cc                                                //
//                                                                          //
//   Applies scale/smear corrections to electrons.                          //
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
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "TRandom3.h"
#include "correction.h"

using pat::Electron, pat::ElectronCollection;
typedef edm::View<Electron> ElectronView;

class PATElectronCorrector : public edm::stream::EDProducer<>{
public:
  explicit PATElectronCorrector(const edm::ParameterSet&);
  ~PATElectronCorrector() {}
private:
  // Methods
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  // Data
  edm::EDGetTokenT<ElectronView> srcToken_;

  const bool isMC_;
  std::string scaleFileName_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_;
  const bool hasSeed_;
  const ULong64_t seed_;
};


// Constructors and destructors

PATElectronCorrector::PATElectronCorrector(const edm::ParameterSet& iConfig) :
  srcToken_(consumes<ElectronView>(iConfig.exists("src") ?
      iConfig.getParameter<edm::InputTag>("src") :
      edm::InputTag("slimmedElectrons"))),
  isMC_(iConfig.getParameter<bool>("isMC")),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
  hasSeed_(iConfig.exists("seed")),
  seed_(hasSeed_? iConfig.getParameter<ULong64_t>("seed") : 0)
{
  try{
    scaleFile_ = correction::CorrectionSet::from_file(scaleFileName_);
    if (scaleFile_ == nullptr) throw cms::Exception("Invalid JSON file");
  }
  catch (...){
    throw cms::Exception("Invalid JSON file") << "Filepath: " << scaleFileName_;
  }

  produces<ElectronCollection>();
}


void PATElectronCorrector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<ElectronView> electronsIn;
  iEvent.getByToken(srcToken_, electronsIn);

  std::unique_ptr<ElectronCollection> out(new ElectronCollection());

  for(ElectronView::const_iterator ei = electronsIn->begin(); ei != electronsIn->end(); ei++)
  {
    out->push_back(*ei); // copy electron to save correctly in event
    Electron& ele = out->back();
    
    float rho = 0, err_rho = 0;
    float scale = 1, err_scale = 0;
    float smear = 1, smear_up = 1, smear_dn = 1;
    if (isMC_){
      rho       = scaleFile_->at("Smearing")->evaluate({"rho", ele.eta(), ele.r9()});
      err_rho   = scaleFile_->at("Smearing")->evaluate({"err_rho", ele.eta(), ele.r9()});
      err_scale = scaleFile_->at("Scale")->evaluate({"total_uncertainty", ele.userInt("seedGain"), (double)iEvent.run(), ele.eta(), ele.r9(), ele.pt()});

      TRandom3 rand;
      rand.SetSeed(hasSeed_? seed_ : std::abs(static_cast<int>(std::sin(ele.phi())*100000)));
      smear = rand.Gaus(1., rho);
      smear_up = rand.Gaus(1., rho+err_rho);
      smear_dn = rand.Gaus(1., rho-err_rho);
    }
    else
      scale = scaleFile_->at("Scale")->evaluate({"total_correction", ele.userInt("seedGain"), (double)iEvent.run(), ele.eta(), ele.r9(), ele.pt()});
    
    float uncorrected_pt = ele.pt();
    float corrected_pt = uncorrected_pt * (isMC_? smear : scale);

    ele.addUserFloat("uncorrected_pt", uncorrected_pt);
    ele.setP4(reco::Particle::PolarLorentzVector(corrected_pt, ele.eta(), ele.phi(), ele.mass()));

    // Custom user floats to save scale and smearing
    ele.addUserFloat("energyScaleValue", scale);
    ele.addUserFloat("energyScaleUp", 1+err_scale);
    ele.addUserFloat("energyScaleDn", 1-err_scale);
    ele.addUserFloat("energySmearValue", smear);
    ele.addUserFloat("energySmearUp", smear_up);
    ele.addUserFloat("energySmearDn", smear_dn);

    //TODO: Determine if the following are necessary (written to ntuple but never accessed in VVAnalysis - not sure how to re-create the values
    //manually)
    //  referenced from electron ZZ ID embedder
    /*
    //get all scale uncertainties and their breakdown
    float scale_total_up = ele.userFloat("energyScaleUp") / ele.energy();
    float scale_stat_up = ele.userFloat("energyScaleStatUp") / ele.energy();
    float scale_syst_up = ele.userFloat("energyScaleSystUp") / ele.energy();
    float scale_gain_up = ele.userFloat("energyScaleGainUp") / ele.energy();
    float scale_total_dn = ele.userFloat("energyScaleDown") / ele.energy();
    float scale_stat_dn = ele.userFloat("energyScaleStatDown") / ele.energy();
    float scale_syst_dn = ele.userFloat("energyScaleSystDown") / ele.energy();
    float scale_gain_dn = ele.userFloat("energyScaleGainDown") / ele.energy();
    //get all smearing uncertainties and their breakdown
    float sigma_total_up = ele.userFloat("energySigmaUp") / ele.energy();
    float sigma_rho_up = ele.userFloat("energySigmaRhoUp") / ele.energy();
    float sigma_phi_up = ele.userFloat("energySigmaPhiUp") / ele.energy();
    float sigma_total_dn = ele.userFloat("energySigmaDown") / ele.energy();
    float sigma_rho_dn = ele.userFloat("energySigmaRhoDown") / ele.energy();
    float sigma_phi_dn = ele.userFloat("energySigmaPhiDown") / ele.energy();
    */
  }

  iEvent.put(std::move(out));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronCorrector);
