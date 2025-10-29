//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetIDEmbedder.cc                                                   //
//                                                                          //
//    Embed basic PF Jet IDs as userFloats                                  //
//                                                                          //
//    Author: Justin Marquez, U. Wisconsin                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include<memory>
#include<string>
#include<vector>
#include <fstream>

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
    std::string idFileName_, idConfig_, lepvetoConfig_;
    std::unique_ptr<correction::CorrectionSet> idFile_;
};


PATJetIDEmbedder::PATJetIDEmbedder(const edm::ParameterSet& iConfig) :
  srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  matchToken_(consumes<MatchMap>(edm::InputTag("patJetGenJetMatch"))),
  domatch_(iConfig.exists("domatch") ? iConfig.getParameter<bool>("domatch") : false),
  idFileName_(iConfig.getParameter<std::string>("idFile")),
  idConfig_(iConfig.getParameter<std::string>("config")),
  lepvetoConfig_(iConfig.getParameter<std::string>("lepveto"))
{
  std::ifstream checkfile(idFileName_);
  if (!checkfile.good()) idFileName_ = idFileName_.substr(idFileName_.find("/UWVV/") + 6);
  else checkfile.close();

  try{
    idFile_ = correction::CorrectionSet::from_file(idFileName_);
    if (idFile_ == nullptr) throw cms::Exception("Invalid JSON file");
  }
  catch (...){
    throw cms::Exception("Invalid JSON file") << "Filename: " << idFileName_;
  }

  auto it = idFile_->begin();
  for (;it != idFile_->end(); it++)
    if (it->first == idConfig_) break;
  if (it == idFile_->end())
    throw cms::Exception("Invalid jet ID config") << "Config: " << idConfig_;
  it = idFile_->begin();
  for (;it != idFile_->end(); it++)
    if (it->first == lepvetoConfig_) break;
  if (it == idFile_->end())
    throw cms::Exception("Invalid jet ID config") << "Config: " << lepvetoConfig_;

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
    float chEmEF = jet.chargedEmEnergyFraction();
    float neEmEF = jet.neutralEmEnergyFraction();
    float muEF   = jet.muonEnergyFraction();
    int chMult   = jet.chargedMultiplicity();
    int neMult   = jet.neutralMultiplicity();
    int mult     = chMult + neMult;

    float passTight = idFile_->at(idConfig_)->evaluate({eta, chHF, neHF, chEmEF, neEmEF, muEF, chMult, neMult, mult});
    jet.addUserFloat("idTight", float(passTight > 0.5));

    float passTightLepVeto = idFile_->at(lepvetoConfig_)->evaluate({eta, chHF, neHF, chEmEF, neEmEF, muEF, chMult, neMult, mult});
    jet.addUserFloat("idTightLepVeto", float(passTightLepVeto > 0.5));

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
