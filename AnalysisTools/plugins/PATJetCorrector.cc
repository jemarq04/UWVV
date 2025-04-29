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
  std::string scaleFileName_, smearFileName_, jesConfig_, jerConfig_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_, smearFile_;
  const bool isMC_;
  edm::ConsumesCollector cc;
  edm::ESGetToken<JetCorrectorParametersCollection,JetCorrectionsRecord> jecToken;
};


PATJetCorrector::PATJetCorrector(const edm::ParameterSet& iConfig) :
  srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  rhoToken(consumes<double>(iConfig.getParameter<edm::InputTag>("rhoSrc"))),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
  smearFileName_(iConfig.exists("smearFile") ?
      iConfig.getParameter<std::string>("smearFile") : 
      "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/jer_smear.json.gz"),
  jesConfig_(iConfig.getParameter<std::string>("jesConfig")),
  jerConfig_(iConfig.getParameter<std::string>("jerConfig")),
  isMC_(iConfig.exists("isMC") ? iConfig.getParameter<bool>("isMC") : false),
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
  if (isMC_){
    produces<VJet>("jerUp");
    produces<VJet>("jerDown");
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
  std::unique_ptr<VJet> out_jerUp(new VJet());
  std::unique_ptr<VJet> out_jerDn(new VJet());

  std::string jesName = jesConfig_ + (isMC_? "_MC" : "_DATA") + "_L1L2L3Res_AK4PFPuppi";
  std::string jerName = jerConfig_ + "_MC_PtResolution_AK4PFPuppi";
  std::string jersfName = jerConfig_ + "_MC_ScaleFactor_AK4PFPuppi";

  for (size_t i = 0; i<in->size(); ++i){
    const Jet& jet = in->at(i);
    out->push_back(jet);
    if (isMC_){
      out_jesUp->push_back(jet);
      out_jesDn->push_back(jet);
      out_jerUp->push_back(jet);
      out_jerDn->push_back(jet);
    }

    // JES
    double jes = scaleFile_->compound().at(jesName)->evaluate({jet.jetArea(), jet.eta(), jet.pt(), *rho});
    out->back().setP4(math::XYZTLorentzVector(jes * jet.p4()));

    if (isMC_){
      // Get jet values
      float eta = out->back().eta();
      float pt = out->back().pt();
      const reco::GenJet* gen = jet.genJet();
      float genpt = (gen!=nullptr)? gen->pt() : -1.0;

      // JES Uncertainty
      jecUnc.setJetEta(jet.eta());
      jecUnc.setJetPt(jet.pt());
      float unc = jecUnc.getUncertainty(true);

      out_jesUp->back().setP4(math::XYZTLorentzVector((1.+unc) * out_jesUp->back().p4()));
      out_jesDn->back().setP4(math::XYZTLorentzVector((1.-unc) * out_jesDn->back().p4()));

      // JER
      double jer       = scaleFile_->at(jerName)->evaluate({eta, pt, *rho});
      double jersf     = scaleFile_->at(jersfName)->evaluate({eta, pt, "nom"});
      double jerCorr   = smearFile_->at("JERSmear")->evaluate({pt, eta, genpt, *rho, int(iEvent.id().event()), jer, jersf});
      out->back().setP4(math::XYZTLorentzVector(jerCorr * jet.p4()));
      out->back().addUserFloat("jerCorrInverse", 1./jerCorr);

      // JER Uncertainty
      double jerUp     = scaleFile_->at(jerName)->evaluate({eta, pt, *rho});
      double jerDn     = scaleFile_->at(jerName)->evaluate({eta, pt, *rho});
      double jersfUp   = scaleFile_->at(jersfName)->evaluate({eta, pt, "up"});
      double jersfDn   = scaleFile_->at(jersfName)->evaluate({eta, pt, "down"});
      double jerCorrUp = smearFile_->at("JERSmear")->evaluate({pt, eta, genpt, *rho, int(iEvent.id().event()), jerUp, jersfUp});
      double jerCorrDn = smearFile_->at("JERSmear")->evaluate({pt, eta, genpt, *rho, int(iEvent.id().event()), jerDn, jersfDn});
      out_jerUp->back().setP4(math::XYZTLorentzVector(jerCorrUp * jet.p4()));
      out_jerUp->back().addUserFloat("jerCorrInverse", 1./jerCorrUp);
      out_jerDn->back().setP4(math::XYZTLorentzVector(jerCorrDn * jet.p4()));
      out_jerDn->back().addUserFloat("jerCorrInverse", 1./jerCorrDn);

      // JER Smearing on JES Uncertainties
      double jer_jesUp     = scaleFile_->at(jerName)->evaluate({eta, out_jesUp->back().pt(), *rho});
      double jer_jesDn     = scaleFile_->at(jerName)->evaluate({eta, out_jesDn->back().pt(), *rho});
      double jersf_jesUp   = scaleFile_->at(jersfName)->evaluate({eta, out_jesUp->back().pt(), "nom"});
      double jersf_jesDn   = scaleFile_->at(jersfName)->evaluate({eta, out_jesDn->back().pt(), "nom"});
      double jerCorr_jesUp = smearFile_->at("JERSmear")->evaluate({out_jesUp->back().pt(), eta, genpt, *rho, int(iEvent.id().event()), jer_jesUp, jersf_jesUp});
      double jerCorr_jesDn = smearFile_->at("JERSmear")->evaluate({out_jesDn->back().pt(), eta, genpt, *rho, int(iEvent.id().event()), jer_jesDn, jersf_jesDn});
      out_jesUp->back().setP4(math::XYZTLorentzVector(jerCorr_jesUp * out_jesUp->back().p4()));
      out_jesUp->back().addUserFloat("jerCorrInverse", 1./jerCorr_jesUp);
      out_jesDn->back().setP4(math::XYZTLorentzVector(jerCorr_jesDn * out_jesDn->back().p4()));
      out_jesDn->back().addUserFloat("jerCorrInverse", 1./jerCorr_jesDn);
    }
  }

  iEvent.put(std::move(out));
  if (isMC_){
    iEvent.put(std::move(out_jesUp), "jesUp");
    iEvent.put(std::move(out_jesDn), "jesDown");
    iEvent.put(std::move(out_jerUp), "jerUp");
    iEvent.put(std::move(out_jerDn), "jerDown");
  }
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetCorrector);
