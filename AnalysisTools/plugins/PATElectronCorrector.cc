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
#include <fstream>

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
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  edm::EDGetTokenT<ElectronView> srcToken_;
  const bool isMC_;
  const double minPt_;
  std::string scaleFileName_, scaleConfig_, smearConfig_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_;
  const bool hasSeed_;
  const ULong64_t seed_;
};

PATElectronCorrector::PATElectronCorrector(const edm::ParameterSet& iConfig) :
  srcToken_(consumes<ElectronView>(iConfig.exists("src") ?
      iConfig.getParameter<edm::InputTag>("src") :
      edm::InputTag("slimmedElectrons"))),
  isMC_(iConfig.getParameter<bool>("isMC")),
  minPt_(iConfig.exists("minPt") ?
      iConfig.getParameter<double>("minPt") :
      20),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
  scaleConfig_(iConfig.getParameter<std::string>("scaleConfig")),
  smearConfig_(iConfig.getParameter<std::string>("smearConfig")),
  hasSeed_(iConfig.exists("seed")),
  seed_(hasSeed_? iConfig.getParameter<ULong64_t>("seed") : 0)
{
  std::ifstream checkfile(scaleFileName_);
  if (!checkfile.good()) scaleFileName_ = scaleFileName_.substr(scaleFileName_.find("/UWVV/") + 6);
  else checkfile.close();

  try{
    scaleFile_ = correction::CorrectionSet::from_file(scaleFileName_);
    if (scaleFile_ == nullptr) throw cms::Exception("Invalid JSON file");
  }
  catch (...){
    throw cms::Exception("Invalid JSON file") << "Filepath: " << scaleFileName_;
  }

  auto scale_it = scaleFile_->compound().begin();
  for (;scale_it != scaleFile_->compound().end(); scale_it++)
    if (scale_it->first == scaleConfig_) break;
  if (scale_it == scaleFile_->compound().end())
    throw cms::Exception("Invalid scale config") << "Config: " << scaleConfig_;

  auto smear_it = scaleFile_->begin();
  for (; smear_it != scaleFile_->end(); smear_it++)
    if (smear_it->first == smearConfig_) break;
  if (smear_it == scaleFile_->end())
    throw cms::Exception("Invalid smear config") << "Config: " << smearConfig_;

  produces<ElectronCollection>();
}


void PATElectronCorrector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<ElectronView> electronsIn;
  iEvent.getByToken(srcToken_, electronsIn);

  std::unique_ptr<ElectronCollection> out(new ElectronCollection());

  bool includeAbsEta = (
      smearConfig_.find("2022") != std::string::npos ||
      smearConfig_.find("2023") != std::string::npos
  );

  for(ElectronView::const_iterator ei = electronsIn->begin(); ei != electronsIn->end(); ei++)
  {
    out->push_back(*ei); // copy electron to save correctly in event
    Electron& ele = out->back();

    float rho = 0, err_rho = 0;
    float scale = 1, err_scale = 0;
    float smear = 1, smear_up = 1, smear_dn = 1;

    if (ele.pt() > minPt_){
      if (isMC_){
        if (includeAbsEta){
          rho       = scaleFile_->at(smearConfig_)->evaluate({"smear", ele.pt(), ele.r9(), std::fabs(ele.eta())});
          err_rho   = scaleFile_->at(smearConfig_)->evaluate({"esmear", ele.pt(), ele.r9(), std::fabs(ele.eta())});
          err_scale = scaleFile_->compound().at(scaleConfig_)->evaluate({
              "escale", (double)iEvent.run(), ele.eta(), ele.r9(),
              std::fabs(ele.eta()), ele.pt(), (double)ele.userInt("seedGain")
          });
        }
        else{
          rho       = scaleFile_->at(smearConfig_)->evaluate({"smear", ele.pt(), ele.r9(), ele.eta()});
          err_rho   = scaleFile_->at(smearConfig_)->evaluate({"esmear", ele.pt(), ele.r9(), ele.eta()});
          err_scale = scaleFile_->compound().at(scaleConfig_)->evaluate({
              "escale", (double)iEvent.run(), ele.eta(), ele.r9(),
              ele.pt(), (double)ele.userInt("seedGain")
          });
        }

        TRandom3 rand;
        rand.SetSeed(hasSeed_? seed_ : std::abs(static_cast<int>(std::sin(ele.phi())*100000)));
        smear = rand.Gaus(1., rho);
        smear_up = rand.Gaus(1., rho+err_rho);
        smear_dn = rand.Gaus(1., rho-err_rho);
      }
      else{
        if (includeAbsEta)
          scale = scaleFile_->compound().at(scaleConfig_)->evaluate({
              "scale", (double)iEvent.run(), ele.eta(), ele.r9(),
              std::fabs(ele.eta()), ele.pt(), (double)ele.userInt("seedGain")
          });
        else
          scale = scaleFile_->compound().at(scaleConfig_)->evaluate({
              "scale", (double)iEvent.run(), ele.eta(), ele.r9(),
              ele.pt(), (double)ele.userInt("seedGain")
          });
      }
    }

    float uncorr_pt = ele.pt();
    float corr_pt = uncorr_pt * (isMC_? smear : scale);

    ele.addUserFloat("uncorrected_pt", uncorr_pt);
    ele.addUserFloat("ptScaleFactor",  corr_pt/uncorr_pt);
    if (isMC_){
      ele.addUserFloat("scaleUp_pt", uncorr_pt*(1+err_scale));
      ele.addUserFloat("scaleDn_pt", uncorr_pt*(1-err_scale));
      ele.addUserFloat("smearUp_pt", uncorr_pt*smear_up);
      ele.addUserFloat("smearDn_pt", uncorr_pt*smear_dn);
    }
    ele.setP4(reco::Particle::PolarLorentzVector(corr_pt, ele.eta(), ele.phi(), ele.mass()));
  }

  iEvent.put(std::move(out));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronCorrector);
