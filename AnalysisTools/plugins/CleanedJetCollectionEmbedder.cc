///////////////////////////////////////////////////////////////////////////////
//      CleanedJetCollectionEmbedder.cc
//
//      Create new jet collection from input collection by removing all
//      jets which overlap a lepton candidate contained in the initial state.
//      Overlap is defined as dR(lepton candidate, jet) < DR_input. Default
//      overlap value is 0.4.Collection is named cleanedJets by default.
//
///////////////////////////////////////////////////////////////////////////////

// system includes
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Common/interface/View.h"

#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/Ref.h"
#include "FWCore/Framework/interface/EventSetup.h"

#include "UWVV/Utilities/interface/helpers.h"

#include "correction.h"

// ROOT includes
#include "TH2F.h"
#include "TFile.h"

typedef pat::CompositeCandidate CCand;
typedef pat::Jet Jet;
typedef std::vector<Jet> VJet;
typedef edm::View<Jet> JetView;

class CleanedJetCollectionEmbedder : public edm::stream::EDProducer<>
{

  public:
    explicit CleanedJetCollectionEmbedder(const edm::ParameterSet &iConfig);
    virtual ~CleanedJetCollectionEmbedder(){};

  private:
    virtual void produce(edm::Event &iEvent, const edm::EventSetup &iSetup);

    edm::PtrVector<pat::Jet> getCleanedJetCollection(edm::Event &iEvent,
        const edm::EDGetTokenT<edm::View<pat::Jet>> &jetToken,
        const reco::CompositeCandidate &initialState);

    edm::PtrVector<pat::Jet> getCleanedJetCollection2(edm::Event &iEvent,
        const edm::EDGetTokenT<edm::View<pat::Jet>> &jetToken,
        const reco::CompositeCandidate &initialState);

    const edm::EDGetTokenT<edm::View<CCand>> srcToken;
    const edm::EDGetTokenT<edm::View<pat::Jet>> jetSrcToken;

    const std::string collectionName;

    typedef edm::Association<reco::GenJetCollection> MatchMap;
    edm::EDGetTokenT<JetView> srcToken2;
    edm::EDGetTokenT<MatchMap> matchToken_;
    std::unique_ptr<correction::CorrectionSet> scaleFile_;
    bool domatch_;
    std::string scaleFileN_;
    const int setup_;

    const double deltaR;
    const bool APV;
    const std::string workingPoint;
    const std::string notSF = "sfFileNone";

    edm::EDGetTokenT<edm::View<pat::Jet>> jesUpJetSrcToken;
    edm::EDGetTokenT<edm::View<pat::Jet>> jesDownJetSrcToken;
    edm::EDGetTokenT<edm::View<pat::Jet>> jerUpJetSrcToken;
    edm::EDGetTokenT<edm::View<pat::Jet>> jerDownJetSrcToken;
    bool jesUpTagExists;
    bool jesDownTagExists;
    bool jerUpTagExists;
    bool jerDownTagExists;

    int PUid;
    int evtcount = 0;
    int jetcount = 0;
    float weight;

    typedef const edm::Ptr<reco::Candidate>(FType)(const reco::Candidate *const);
};

CleanedJetCollectionEmbedder::CleanedJetCollectionEmbedder(const edm::ParameterSet &iConfig) 
  : srcToken(consumes<edm::View<CCand>>(iConfig.getParameter<edm::InputTag>("src"))),
  jetSrcToken(consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jetSrc"))),
  collectionName(iConfig.getUntrackedParameter<std::string>("collectionName", "cleanedJets")),
  matchToken_(consumes<MatchMap>(edm::InputTag("patJetGenJetMatch2"))),
  domatch_(iConfig.exists("domatch") ? iConfig.getParameter<bool>("domatch") : false),
  scaleFileN_(iConfig.exists("scaleFile") ? iConfig.getParameter<std::string>("scaleFile") : notSF),
  // Which year JET ID we need
  setup_(iConfig.exists("setup") ? iConfig.getParameter<int>("setup") : 2016),
  deltaR(iConfig.getUntrackedParameter<double>("deltaR", 0.4)),
  APV(iConfig.exists("APV") ? iConfig.getParameter<bool>("APV") : false),
  workingPoint(iConfig.exists("workingPoint") ? iConfig.getParameter<std::string>("workingPoint") : "T"),
  jesUpTagExists(iConfig.existsAs<edm::InputTag>("jesUpJetSrc")),
  jesDownTagExists(iConfig.existsAs<edm::InputTag>("jesDownJetSrc")),
  jerUpTagExists(iConfig.existsAs<edm::InputTag>("jerUpJetSrc")),
  jerDownTagExists(iConfig.existsAs<edm::InputTag>("jerDownJetSrc"))
{
  if (jesUpTagExists)
    jesUpJetSrcToken = consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jesUpJetSrc"));
  if (jesDownTagExists)
    jesDownJetSrcToken = consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jesDownJetSrc"));
  if (jerUpTagExists)
    jerUpJetSrcToken = consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jerUpJetSrc"));
  if (jerDownTagExists)
    jerDownJetSrcToken = consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jerDownJetSrc"));

  if (domatch_ && scaleFileN_ != notSF) 
  {
    // Define correction set here
    try{
      scaleFile_ = correction::CorrectionSet::from_file(scaleFileN_);
    }
    catch (...){
      throw cms::Exception("Invalid POG file") << "Filepath: " << scaleFileN_;
    }
  }

  produces<std::vector<CCand>>();
}

void CleanedJetCollectionEmbedder::produce(edm::Event &iEvent,
    const edm::EventSetup &iSetup)
{
  edm::Handle<edm::View<CCand>> in;
  iEvent.getByToken(srcToken, in);

  std::unique_ptr<std::vector<CCand>> out(new std::vector<CCand>);

  for (size_t i = 0; i < in->size(); ++i)
  {
    edm::Ptr<CCand> cand = in->ptrAt(i);
    out->push_back(*cand);

    if (jesUpTagExists){
      edm::PtrVector<pat::Jet> cleanedJets = getCleanedJetCollection2(iEvent, jetSrcToken, *cand);
      out->back().addUserData<edm::PtrVector<pat::Jet>>(collectionName, cleanedJets);
    }
    else{
      edm::PtrVector<pat::Jet> cleanedJets = getCleanedJetCollection(iEvent, jetSrcToken, *cand); 
      out->back().addUserData<edm::PtrVector<pat::Jet>>(collectionName, cleanedJets);
    }


    if (jesUpTagExists) out->back().addUserFloat("jetPUSFmulfac", weight);

    if (jesUpTagExists)
    {
      edm::PtrVector<pat::Jet> cleanedJesUpJets = getCleanedJetCollection(iEvent, jesUpJetSrcToken, *cand);
      out->back().addUserData<edm::PtrVector<pat::Jet>>(collectionName + "_jesUp", cleanedJesUpJets);
    }
    if (jesDownTagExists)
    {
      edm::PtrVector<pat::Jet> cleanedJesDownJets = getCleanedJetCollection(iEvent, jesDownJetSrcToken, *cand);
      out->back().addUserData<edm::PtrVector<pat::Jet>>(collectionName + "_jesDown", cleanedJesDownJets);
    }
    if (jerUpTagExists)
    {
      edm::PtrVector<pat::Jet> cleanedJerUpJets = getCleanedJetCollection(iEvent, jerUpJetSrcToken, *cand);
      out->back().addUserData<edm::PtrVector<pat::Jet>>(collectionName + "_jerUp", cleanedJerUpJets);
    }
    if (jerDownTagExists)
    {
      edm::PtrVector<pat::Jet> cleanedJerDownJets = getCleanedJetCollection(iEvent, jerDownJetSrcToken, *cand);
      out->back().addUserData<edm::PtrVector<pat::Jet>>(collectionName + "_jerDown", cleanedJerDownJets);
    }
  }

  iEvent.put(std::move(out));
}

edm::PtrVector<pat::Jet> CleanedJetCollectionEmbedder::getCleanedJetCollection2(edm::Event &iEvent,
    const edm::EDGetTokenT<edm::View<pat::Jet>> &jetToken,
    const reco::CompositeCandidate &initialState)
{
  edm::Handle<edm::View<pat::Jet>> uncleanedJets;
  edm::PtrVector<pat::Jet> cleanedJets;
  std::unique_ptr<VJet> out2(new VJet());

  iEvent.getByToken(jetToken, uncleanedJets);

  weight = 1.; // mult factor for jet PU id SF correction
  edm::Handle<MatchMap> match;
  if (domatch_)
    iEvent.getByToken(matchToken_, match);

  for (size_t j = 0; j < uncleanedJets->size(); ++j)
  {
    if (!uwvv::helpers::overlapWithAnyDaughter(uncleanedJets->at(j), initialState, deltaR))
    {
      out2->push_back(uncleanedJets->at(j)); // just for copying, no output
      Jet &jet = out2->back();
      PUid = jet.userInt("pileupJetIdUpdated:fullId"); // can use jetRef, but just in case
      if (PUid >= 7 || jet.pt() > 50)
        cleanedJets.push_back(uncleanedJets->ptrAt(j));

      if (domatch_)
      {
        edm::Ref<JetView> jetRef(uncleanedJets, j);
        const auto genMatched = (*match)[jetRef];

        if (genMatched.isNonnull() && jet.pt() < 50 && scaleFileN_ != notSF){
          float jetPUSF = scaleFile_->at("PUJetID_eff")->evaluate({jet.eta(), jet.pt(), "nom", workingPoint});
          float jeffPU  = scaleFile_->at("PUJetID_eff")->evaluate({jet.eta(), jet.pt(), "MCEff", workingPoint});
          float mulfac = 1.;

          if (PUid < 7) mulfac = (1. - jetPUSF * jeffPU) / (1. - jeffPU);
          else mulfac = jetPUSF;

          weight *= mulfac;
        }
      }
    }
  }
  return cleanedJets;
}

edm::PtrVector<pat::Jet> CleanedJetCollectionEmbedder::getCleanedJetCollection(edm::Event &iEvent,
    const edm::EDGetTokenT<edm::View<pat::Jet>> &jetToken,
    const reco::CompositeCandidate &initialState)
{
  edm::Handle<edm::View<pat::Jet>> uncleanedJets;
  edm::PtrVector<pat::Jet> cleanedJets;

  iEvent.getByToken(jetToken, uncleanedJets);

  for (size_t j = 0; j < uncleanedJets->size(); ++j)
  {
    if (!uwvv::helpers::overlapWithAnyDaughter(uncleanedJets->at(j), initialState, deltaR))
      cleanedJets.push_back(uncleanedJets->ptrAt(j));
  }
  return cleanedJets;
}

DEFINE_FWK_MODULE(CleanedJetCollectionEmbedder);
