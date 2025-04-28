//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetJERCEmbedder.cc                                                 //
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

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "correction.h"


typedef pat::Jet Jet;
typedef std::vector<Jet> VJet;
typedef edm::View<Jet> JetView;

class PATJetJERCEmbedder : public edm::stream::EDProducer<>
{
public:
  explicit PATJetJERCEmbedder(const edm::ParameterSet& iConfig);
  virtual ~PATJetJERCEmbedder() {;}

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  edm::EDGetTokenT<JetView> srcToken;
  edm::EDGetTokenT<double> rhoToken;
  std::string scaleFileName_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_;
};


PATJetJERCEmbedder::PATJetJERCEmbedder(const edm::ParameterSet& iConfig) :
  srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  rhoToken(consumes<double>(iConfig.getParameter<edm::InputTag>("rhoSrc"))),
  scaleFileName(iConfig.getParameter<std::string>("scaleFile"))
{
  try{
    scaleFile_ = correction::CorrectionSet::from_file(scaleFileName_);
    if (scaleFile_ == nullptr) throw cms::Exception("Invalid JSON file");
  }
  catch (...){
    throw cms::Exception("Invalid JSON file") << "Filepath: " << scaleFileName_;
  }

  produces<VJet>();
}


void PATJetJERCEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<double> rho;
  iEvent.getByToken(rhoToken, rho);

  std::unique_ptr<VJet> out(new VJet());

  for (size_t i = 0; i<in->size(); ++i){
    const Jet& jet = in->at(i);

    float pt = jet.pt();
    float eta = jet.eta();
    float phi = jet.phi();
    const reco::GenJet* gen = jet.genJet();

    math::XYZTLorentzVector p4JER = jerCorr * jet.p4();

    out->push_back(jet);
    out->back().setP4(p4JER);
  }

  iEvent.put(std::move(out));
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetJERCEmbedder);
