//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetSmearing.cc                                                     //
//                                                                          //
//    Apply energy smearing to PF jets. Outputs a collection of jets with   //
//    the smearing applied and with a correction that undoes the smearing   //
//    stored as a userFloat                                                 //
//    To get the original 4-momentum back:                                  //
//        jet.p4() * jet.userFloat("jerCorrInverse")                        //
//                                                                          //
//    If systematic shifts are also requested (systematics=cms.bool(True),  //
//    also produces one collection with the smearing shifted up by one      //
//    sigma, and another with the smearing shifted down by one sigma, for   //
//    systematics. Labels for these are "jerUp" and "jerDown".              //
//                                                                          //
//    Obviously, this only makes sense for MC                               //
//                                                                          //
//    Author: Nate Woods, U. Wisconsin                                      //
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

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "TRandom3.h"
#include "correction.h"


typedef pat::Jet Jet;
typedef std::vector<Jet> VJet;
typedef edm::View<Jet> JetView;

class PATJetSmearing : public edm::stream::EDProducer<>
{
public:
  explicit PATJetSmearing(const edm::ParameterSet& iConfig);
  virtual ~PATJetSmearing() {;}

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  edm::EDGetTokenT<JetView> srcToken;
  edm::EDGetTokenT<double> rhoToken;
  std::string scaleFileName_, smearFileName_, config_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_, smearFile_;

  const bool systematics_;
};


PATJetSmearing::PATJetSmearing(const edm::ParameterSet& iConfig) :
  srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  rhoToken(consumes<double>(iConfig.getParameter<edm::InputTag>("rhoSrc"))),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
  smearFileName_(iConfig.exists("smearFile") ?
      iConfig.getParameter<std::string>("smearFile") :
      "/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/JME/jer_smear.json.gz"),
  config_(iConfig.getParameter<std::string>("config")),
  systematics_(iConfig.exists("systematics") ?
      iConfig.getParameter<bool>("systematics") : false)
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
  if(systematics_)
  {
    produces<VJet>("jerUp");
    produces<VJet>("jerDown");
  }
}


void PATJetSmearing::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<double> rho;
  iEvent.getByToken(rhoToken, rho);

  std::unique_ptr<VJet> out(new VJet());
  std::unique_ptr<VJet> outUp(new VJet());
  std::unique_ptr<VJet> outDn(new VJet());
  
  std::string jerName = config_ + "_MC_PtResolution_AK4PFPuppi";
  std::string jersfName = config_ + "_MC_ScaleFactor_AK4PFPuppi";

  for (size_t i=0; i<in->size(); ++i)
  {
    const Jet& jet = in->at(i);
    out->push_back(jet);
    if (systematics_){
      outUp->push_back(jet);
      outDn->push_back(jet);
    }

    float pt = jet.pt();
    float eta = jet.eta();
    const reco::GenJet* gen = jet.genJet();
    float genpt = (gen!=nullptr)? gen->pt() : -1.0;

    // JER
    double jer       = scaleFile_->at(jerName)->evaluate({eta, pt, *rho});
    double jersf     = scaleFile_->at(jersfName)->evaluate({eta, pt, "nom"});
    double jerCorr   = smearFile_->at("JERSmear")->evaluate({pt, eta, genpt, *rho, int(iEvent.id().event()), jer, jersf});
    out->back().setP4(math::XYZTLorentzVector(jerCorr * jet.p4()));
    out->back().addUserFloat("jerCorrInverse", 1./jerCorr);

    if(systematics_)
    {
      // JER Uncertainty
      double jersfUp   = scaleFile_->at(jersfName)->evaluate({eta, pt, "up"});
      double jersfDn   = scaleFile_->at(jersfName)->evaluate({eta, pt, "down"});
      double jerCorrUp = smearFile_->at("JERSmear")->evaluate({pt, eta, genpt, *rho, int(iEvent.id().event()), jer, jersfUp});
      double jerCorrDn = smearFile_->at("JERSmear")->evaluate({pt, eta, genpt, *rho, int(iEvent.id().event()), jer, jersfDn});

      outUp->back().setP4(math::XYZTLorentzVector(jerCorrUp * jet.p4()));
      outUp->back().addUserFloat("jerCorrInverse", 1./jerCorrUp);

      outDn->back().setP4(math::XYZTLorentzVector(jerCorrDn * jet.p4()));
      outDn->back().addUserFloat("jerCorrInverse", 1./jerCorrDn);
    }
  }

  iEvent.put(std::move(out));
  if(systematics_)
  {
    iEvent.put(std::move(outUp), "jerUp");
    iEvent.put(std::move(outDn), "jerDown");
  }
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetSmearing);
