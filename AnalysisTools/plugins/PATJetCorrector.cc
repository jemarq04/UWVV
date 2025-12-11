//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetCorrector.cc                                                    //
//                                                                          //
//    Author: Justin Marquez, U. Wisconsin                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include<memory>
#include<string>
#include<vector>
#include<cmath> // std::sqrt, std::abs, std::sin
#include<algorithm> // std::max
#include <fstream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "correction.h"

using pat::Jet, pat::JetCollection;
typedef edm::View<Jet> JetView;

class PATJetCorrector : public edm::stream::EDProducer<>
{
public:
  explicit PATJetCorrector(const edm::ParameterSet& iConfig);
  virtual ~PATJetCorrector() {;}

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  double getRawFactor(const Jet& jet);
  void scaleJetP4(Jet& jet, double scale);

  edm::EDGetTokenT<JetView> srcToken;
  edm::EDGetTokenT<double> rhoToken;
  std::string scaleFileName_, config_, algo_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_;
  const bool isMC_, systematics_;
  edm::ConsumesCollector cc;
  edm::ESGetToken<JetCorrectorParametersCollection,JetCorrectionsRecord> jecToken;

  std::string jesName_, jesUncName_;
};


PATJetCorrector::PATJetCorrector(const edm::ParameterSet& iConfig) :
  srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  rhoToken(consumes<double>(iConfig.getParameter<edm::InputTag>("rhoSrc"))),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
  config_(iConfig.getParameter<std::string>("config")),
  algo_(iConfig.exists("algo") ? iConfig.getParameter<std::string>("algo") : "AK4PFPuppi"),
  isMC_(iConfig.exists("isMC") ? iConfig.getParameter<bool>("isMC") : false),
  systematics_(iConfig.exists("systematics") ? iConfig.getParameter<bool>("systematics") : isMC_),
  cc(consumesCollector()),
  jecToken(cc.esConsumes(edm::ESInputTag("",algo_)))
{
  std::ifstream checkfile(scaleFileName_);
  if (!checkfile.good()) scaleFileName_ = scaleFileName_.substr(scaleFileName_.find("/UWVV/") + 6);
  else checkfile.close();

  try{
    scaleFile_ = correction::CorrectionSet::from_file(scaleFileName_);
    if (scaleFile_ == nullptr) throw cms::Exception("Invalid JSON file") << scaleFileName_;
  }
  catch (...){
    throw cms::Exception("Invalid JSON file") << "Filepath: " << scaleFileName_;
  }

  jesName_ = config_ + (isMC_? "_MC" : "_DATA") + "_L1L2L3Res_" + algo_;
  auto it = scaleFile_->compound().begin();
  for (;it != scaleFile_->compound().end(); it++)
    if (it->first == jesName_) break;
  if (it == scaleFile_->compound().end())
    throw cms::Exception("Invalid JES config") << "Config: " << jesName_;

  if (systematics_){
    jesUncName_ = config_ + "_MC_Total_" + algo_;
    auto unc_it = scaleFile_->begin();
    for (; unc_it != scaleFile_->end(); unc_it++)
      if (unc_it->first == jesUncName_) break;
    if (unc_it == scaleFile_->end())
      throw cms::Exception("Invalid JES uncertainty config") << "Config: " << jesUncName_;
  }


  produces<JetCollection>();
  if (systematics_){
    produces<JetCollection>("jesUp");
    produces<JetCollection>("jesDown");
  }
}


void PATJetCorrector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<double> rho;
  iEvent.getByToken(rhoToken, rho);

  std::unique_ptr<JetCollection> out(new JetCollection());
  std::unique_ptr<JetCollection> out_jesUp(new JetCollection());
  std::unique_ptr<JetCollection> out_jesDn(new JetCollection());

  bool includePhi = !(
    config_.find("Summer22") != std::string::npos ||
    (config_.find("Summer23") != std::string::npos && jesName_.find("BPix") == std::string::npos)
  ); //true starting 2023BPix
  bool includeRun = !(config_.find("Summer22") != std::string::npos); //true starting 2023

  for (size_t i = 0; i<in->size(); ++i)
  {
    const Jet& jet = in->at(i);

    // JES
    double jes;
    if (isMC_)
      jes = includePhi ?
        scaleFile_->compound().at(jesName_)->evaluate({jet.jetArea(), jet.eta(), jet.pt(), *rho, jet.phi()}) :
        scaleFile_->compound().at(jesName_)->evaluate({jet.jetArea(), jet.eta(), jet.pt(), *rho});
    else{
      if (includeRun)
        jes = includePhi ?
          scaleFile_->compound().at(jesName_)->evaluate({jet.jetArea(), jet.eta(), jet.pt(), *rho, jet.phi(), (double)iEvent.run()}) :
          scaleFile_->compound().at(jesName_)->evaluate({jet.jetArea(), jet.eta(), jet.pt(), *rho, (double)iEvent.run()});
      else
        jes = scaleFile_->compound().at(jesName_)->evaluate({jet.jetArea(), jet.eta(), jet.pt(), *rho});
    }
    out->push_back(jet);
    scaleJetP4(out->back(), jes * getRawFactor(jet));

    if (systematics_){
      const Jet& jetCorr = out->back();

      // JES Uncertainty
      double unc = scaleFile_->at(jesUncName_)->evaluate({jetCorr.eta(), jetCorr.pt()});

      out_jesUp->push_back(jetCorr);
      scaleJetP4(out_jesUp->back(), 1.0 + unc);

      out_jesDn->push_back(jetCorr);
      scaleJetP4(out_jesUp->back(), 1.0 - unc);
    }
  }

  iEvent.put(std::move(out));
  if (systematics_){
    iEvent.put(std::move(out_jesUp), "jesUp");
    iEvent.put(std::move(out_jesDn), "jesDown");
  }
}

double PATJetCorrector::getRawFactor(const Jet& jet){
  // Used to uncorrect the initial jet collection before re-correcting them

  double ptCorr = jet.pt();
  double ptRaw  = jet.correctedP4("Uncorrected").pt();

  double rawFactor = (ptCorr > 0.0)? 1.0 - ptRaw/ptCorr : 0.0;
  return 1.0 - std::min( std::max(rawFactor, 0.0), 1.0); // factor of [0,1], 1 being already raw
}

void PATJetCorrector::scaleJetP4(Jet& jet, double scale){
  const auto p4 = jet.p4();
  jet.setP4(reco::Particle::LorentzVector(
        p4.px()*scale, p4.py()*scale, p4.pz()*scale, p4.energy()*scale
  ));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetCorrector);
