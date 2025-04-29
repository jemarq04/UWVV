//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetCorrector.cc                                                 //
//                                                                          //
//    Author: Justin Marquez, U. Wisconsin                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include<memory>
#include<string>
#include<vector>
#include<cmath> // std::sqrt, std::abs, std::sin
#include<algorithm> // std::max

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


typedef pat::Jet Jet;
typedef pat::JetCollection VJet;
typedef edm::View<Jet> JetView;

class PATJetCorrector : public edm::stream::EDProducer<>
{
public:
  explicit PATJetCorrector(const edm::ParameterSet& iConfig);
  virtual ~PATJetCorrector() {;}

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  edm::EDGetTokenT<JetView> srcToken;
  edm::EDGetTokenT<double> rhoToken;
  std::string scaleFileName_, smearFileName_, config_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_, smearFile_;
  const bool isMC_, systematics_;
  edm::ConsumesCollector cc;
  edm::ESGetToken<JetCorrectorParametersCollection,JetCorrectionsRecord> jecToken;
};


PATJetCorrector::PATJetCorrector(const edm::ParameterSet& iConfig) :
  srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  rhoToken(consumes<double>(iConfig.getParameter<edm::InputTag>("rhoSrc"))),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
  config_(iConfig.getParameter<std::string>("config")),
  isMC_(iConfig.exists("isMC") ? iConfig.getParameter<bool>("isMC") : false),
  systematics_(iConfig.exists("systematics") ? iConfig.getParameter<bool>("systematics") : isMC_),
  cc(consumesCollector()),
  jecToken(cc.esConsumes(edm::ESInputTag("","AK4PFPuppi")))
{
  try{
    scaleFile_ = correction::CorrectionSet::from_file(scaleFileName_);
    if (scaleFile_ == nullptr) throw cms::Exception("Invalid JSON file") << scaleFileName_;
  }
  catch (...){
    throw cms::Exception("Invalid JSON file") << "Filepath: " << scaleFileName_;
  }
  try{
    smearFile_ = correction::CorrectionSet::from_file(smearFileName_);
    if (smearFile_ == nullptr) throw cms::Exception("Invalid JER Smear file") << smearFileName_;
  }
  catch (...){
    throw cms::Exception("Invalid JER Smear file") << smearFileName_;
  }

  produces<VJet>();
  if (systematics_){
    produces<VJet>("jesUp");
    produces<VJet>("jesDown");
  }
}


void PATJetCorrector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<double> rho;
  iEvent.getByToken(rhoToken, rho);
  
  edm::ESHandle<JetCorrectorParametersCollection> jecParams;
  jecParams = iSetup.get<JetCorrectionsRecord>().getHandle(jecToken);
  const JetCorrectorParameters & param = (*jecParams)["Uncertainty"];
  JetCorrectionUncertainty jecUnc(param);

  std::unique_ptr<VJet> out(new VJet());
  std::unique_ptr<VJet> out_jesUp(new VJet());
  std::unique_ptr<VJet> out_jesDn(new VJet());

  std::string jesName = config_ + (isMC_? "_MC" : "_DATA") + "_L1L2L3Res_AK4PFPuppi";

  for (size_t i = 0; i<in->size(); ++i)
  {
    const Jet& jet = in->at(i);

    // JES
    double jes = scaleFile_->compound().at(jesName)->evaluate({jet.jetArea(), jet.eta(), jet.pt(), *rho});
    out->push_back(jet);
    out->back().setP4(math::XYZTLorentzVector(jes * jet.p4()));

    if (systematics_){
      const Jet& jetCorr = out->back();

      // JES Uncertainty
      jecUnc.setJetEta(jetCorr.eta());
      jecUnc.setJetPt(jetCorr.pt());
      float unc = jecUnc.getUncertainty(true);

      out_jesUp->push_back(jetCorr);
      out_jesUp->back().setP4(math::XYZTLorentzVector((1.+unc) * jetCorr.p4()));

      out_jesDn->push_back(jetCorr);
      out_jesDn->back().setP4(math::XYZTLorentzVector((1.-unc) * jetCorr.p4()));
    }
  }

  iEvent.put(std::move(out));
  if (systematics_){
    iEvent.put(std::move(out_jesUp), "jesUp");
    iEvent.put(std::move(out_jesDn), "jesDown");
  }
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetCorrector);
