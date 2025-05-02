//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetIDEmbedder.cc                                                   //
//                                                                          //
//    Embed basic PF Jet IDs as userFloats                                  //
//                                                                          //
//    Author: Nate Woods, U. Wisconsin                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include<memory>
#include<string>
#include<vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/Ref.h"

#include "correction.h"

using pat::Jet, pat::JetCollection;
typedef edm::View<Jet> JetView;

class PATJetIDEmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit PATJetIDEmbedder(const edm::ParameterSet& iConfig);
    virtual ~PATJetIDEmbedder() {;}

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    typedef edm::Association<reco::GenJetCollection> MatchMap;

    edm::EDGetTokenT<JetView> srcToken;
    edm::EDGetTokenT<MatchMap> matchToken_;
    bool domatch_;
    std::string idFileName_, idConfig_;
    std::unique_ptr<correction::CorrectionSet> idFile_;
    const bool useUL_;
};


PATJetIDEmbedder::PATJetIDEmbedder(const edm::ParameterSet& iConfig) :
  srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  matchToken_(consumes<MatchMap>(edm::InputTag("patJetGenJetMatch"))),
  domatch_(iConfig.exists("domatch") ? iConfig.getParameter<bool>("domatch") : false),
  idFileName_(iConfig.getParameter<std::string>("idFile")),
  idConfig_(iConfig.getParameter<std::string>("config")),
  useUL_(iConfig.exists("useUL") ? iConfig.getParameter<bool>("useUL") : false)
{
  if (!useUL_){
    try{
      idFile_ = correction::CorrectionSet::from_file(idFileName_);
      if (idFile_ == nullptr) throw cms::Exception("Invalid JSON file");
    }
    catch (...){
      throw cms::Exception("Invalid JSON file") << "Filename: " << idFileName_;
    }
  }

  produces<JetCollection>();
}


void PATJetIDEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);

  edm::Handle<MatchMap> match;
  if (domatch_)
    iEvent.getByToken(matchToken_, match);

  std::unique_ptr<JetCollection> out(new JetCollection());

  for(size_t i = 0; i < in->size(); ++i)
  {
    out->push_back(in->at(i)); // copies, transfers ownership

    Jet& jet = out->back();
  
    float eta    = jet.eta();
    float chHF   = jet.chargedHadronEnergyFraction();
    float neHF   = jet.neutralHadronEnergyFraction();
    float neEmEF = jet.neutralEmEnergyFraction();
    float chMult = jet.chargedMultiplicity();
    float neMult = jet.neutralMultiplicity();
    float mult   = chMult + neMult;

    float passTight = 0;
    if (useUL_){
      float absEta = fabs(eta); 
      passTight = float(
        (absEta <= 2.4 && neHF < 0.90 && neEmEF < 0.90 && mult > 1 && chHF > 0 && chMult > 0) ||
        (absEta > 2.4 && absEta <= 2.7 && neHF < 0.90 && neEmEF < 0.99 && chMult > 0) ||
        (absEta > 2.7 && absEta <= 3.0 && neEmEF > 0.01 && neEmEF < 0.99 && neMult > 1) ||
        (absEta > 3.0 && neHF > 0.2 && neEmEF < 0.9 && neMult > 10)
      );
    }
    else 
      passTight = idFile_->at(idConfig_)->evaluate({eta, chHF, neHF, neEmEF, chMult, neMult, mult});
    jet.addUserFloat("idTight", float(passTight > 0.5));

    if (domatch_){
      edm::Ref<JetView> jetRef(in, i);
      const auto genMatched = (*match)[jetRef];
      jet.addUserFloat("genjetMatched", float(genMatched.isNonnull()));
    }
  }

  iEvent.put(std::move(out));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetIDEmbedder);
