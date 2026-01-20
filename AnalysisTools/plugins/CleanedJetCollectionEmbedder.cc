///////////////////////////////////////////////////////////////////////////////
//      CleanedJetCollectionEmbedder.cc                                      //
//                                                                           //
//      Create new jet collection from input collection by removing all      //
//      jets which overlap a lepton candidate contained in the initial state.//
//      Overlap is defined as dR(lepton candidate, jet) < DR_input. Default  //
//      overlap value is 0.4.Collection is named cleanedJets by default.     //
//                                                                           //
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
typedef edm::PtrVector<Jet> VJetPtr;
typedef edm::View<Jet> JetView;

class CleanedJetCollectionEmbedder : public edm::stream::EDProducer<> {
public:
  explicit CleanedJetCollectionEmbedder(const edm::ParameterSet &iConfig);
  virtual ~CleanedJetCollectionEmbedder() {};

private:
  virtual void produce(edm::Event &iEvent, const edm::EventSetup &iSetup);

  VJetPtr getCleanedJetCollection(edm::Event &iEvent,
                                  const edm::EDGetTokenT<JetView> &jetToken,
                                  const reco::CompositeCandidate &initialState,
                                  bool checkPUID = false);

  const edm::EDGetTokenT<edm::View<CCand>> srcToken;
  const edm::EDGetTokenT<JetView> jetSrcToken;

  const std::string collectionName;

  typedef edm::Association<reco::GenJetCollection> MatchMap;
  edm::EDGetTokenT<MatchMap> matchToken_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_;
  bool domatch_;
  std::string scaleFileName_;

  const double deltaR;
  const std::string workingPoint;

  edm::EDGetTokenT<JetView> jesUpJetSrcToken;
  edm::EDGetTokenT<JetView> jesDownJetSrcToken;
  edm::EDGetTokenT<JetView> jerUpJetSrcToken;
  edm::EDGetTokenT<JetView> jerDownJetSrcToken;
  bool jesUpTagExists;
  bool jesDownTagExists;
  bool jerUpTagExists;
  bool jerDownTagExists;

  float weight;

  typedef const edm::Ptr<reco::Candidate>(FType)(const reco::Candidate *const);
};

CleanedJetCollectionEmbedder::CleanedJetCollectionEmbedder(const edm::ParameterSet &iConfig)
    : srcToken(consumes<edm::View<CCand>>(iConfig.getParameter<edm::InputTag>("src"))),
      jetSrcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("jetSrc"))),
      collectionName(iConfig.getUntrackedParameter<std::string>("collectionName", "cleanedJets")),
      matchToken_(consumes<MatchMap>(edm::InputTag("patJetGenJetMatch2"))),
      domatch_(iConfig.exists("domatch") ? iConfig.getParameter<bool>("domatch") : false),
      scaleFileName_(iConfig.exists("scaleFile") ? iConfig.getParameter<std::string>("scaleFile") : "sfFileNone"),
      // Which year JET ID we need
      deltaR(iConfig.getUntrackedParameter<double>("deltaR", 0.4)),
      workingPoint(iConfig.exists("workingPoint") ? iConfig.getParameter<std::string>("workingPoint") : "T"),
      jesUpTagExists(iConfig.existsAs<edm::InputTag>("jesUpJetSrc")),
      jesDownTagExists(iConfig.existsAs<edm::InputTag>("jesDownJetSrc")),
      jerUpTagExists(iConfig.existsAs<edm::InputTag>("jerUpJetSrc")),
      jerDownTagExists(iConfig.existsAs<edm::InputTag>("jerDownJetSrc")) {
  if (jesUpTagExists)
    jesUpJetSrcToken = consumes<JetView>(iConfig.getParameter<edm::InputTag>("jesUpJetSrc"));
  if (jesDownTagExists)
    jesDownJetSrcToken = consumes<JetView>(iConfig.getParameter<edm::InputTag>("jesDownJetSrc"));
  if (jerUpTagExists)
    jerUpJetSrcToken = consumes<JetView>(iConfig.getParameter<edm::InputTag>("jerUpJetSrc"));
  if (jerDownTagExists)
    jerDownJetSrcToken = consumes<JetView>(iConfig.getParameter<edm::InputTag>("jerDownJetSrc"));

  if (scaleFileName_ != "sfFileNone" && domatch_) {
    // Define correction set here
    std::ifstream checkfile(scaleFileName_);
    if (!checkfile.good())
      scaleFileName_ = scaleFileName_.substr(scaleFileName_.find("/UWVV/") + 6);
    else
      checkfile.close();

    try {
      scaleFile_ = correction::CorrectionSet::from_file(scaleFileName_);
      if (scaleFile_ == nullptr)
        throw cms::Exception("Invalid POG file") << "Filepath: " << scaleFileName_;
    } catch (...) {
      throw cms::Exception("Invalid POG file") << "Filepath: " << scaleFileName_;
    }
  }

  produces<std::vector<CCand>>();
}

void CleanedJetCollectionEmbedder::produce(edm::Event &iEvent, const edm::EventSetup &iSetup) {
  edm::Handle<edm::View<CCand>> in;
  iEvent.getByToken(srcToken, in);

  std::unique_ptr<std::vector<CCand>> out(new std::vector<CCand>);

  for (size_t i = 0; i < in->size(); ++i) {
    edm::Ptr<CCand> cand = in->ptrAt(i);
    out->push_back(*cand);

    if (jesUpTagExists) {
      VJetPtr cleanedJets = getCleanedJetCollection(iEvent, jetSrcToken, *cand, true);
      out->back().addUserData<VJetPtr>(collectionName, cleanedJets);
      out->back().addUserFloat("jetPUSFmulfac", weight);
    } else {
      VJetPtr cleanedJets = getCleanedJetCollection(iEvent, jetSrcToken, *cand);
      out->back().addUserData<VJetPtr>(collectionName, cleanedJets);
    }

    if (jesUpTagExists) {
      VJetPtr cleanedJesUpJets = getCleanedJetCollection(iEvent, jesUpJetSrcToken, *cand);
      out->back().addUserData<VJetPtr>(collectionName + "_jesUp", cleanedJesUpJets);
    }
    if (jesDownTagExists) {
      VJetPtr cleanedJesDownJets = getCleanedJetCollection(iEvent, jesDownJetSrcToken, *cand);
      out->back().addUserData<VJetPtr>(collectionName + "_jesDown", cleanedJesDownJets);
    }
    if (jerUpTagExists) {
      VJetPtr cleanedJerUpJets = getCleanedJetCollection(iEvent, jerUpJetSrcToken, *cand);
      out->back().addUserData<VJetPtr>(collectionName + "_jerUp", cleanedJerUpJets);
    }
    if (jerDownTagExists) {
      VJetPtr cleanedJerDownJets = getCleanedJetCollection(iEvent, jerDownJetSrcToken, *cand);
      out->back().addUserData<VJetPtr>(collectionName + "_jerDown", cleanedJerDownJets);
    }
  }

  iEvent.put(std::move(out));
}

VJetPtr CleanedJetCollectionEmbedder::getCleanedJetCollection(edm::Event &iEvent,
                                                              const edm::EDGetTokenT<JetView> &jetToken,
                                                              const reco::CompositeCandidate &initialState,
                                                              bool checkPUID) {
  edm::Handle<JetView> uncleanedJets;
  iEvent.getByToken(jetToken, uncleanedJets);

  VJetPtr cleanedJets;

  edm::Handle<MatchMap> match;
  if (domatch_)
    iEvent.getByToken(matchToken_, match);

  weight = 1.;  // mult factor for jet PU id SF correction
  for (size_t j = 0; j < uncleanedJets->size(); ++j) {
    if (!uwvv::helpers::overlapWithAnyDaughter(uncleanedJets->at(j), initialState, deltaR)) {
      const Jet &jet = uncleanedJets->at(j);
      int PUID = checkPUID ? jet.userInt("pileupJetIdUpdated:fullId") : 7;
      if (PUID >= 7 || jet.pt() > 50)
        cleanedJets.push_back(uncleanedJets->ptrAt(j));

      if (checkPUID && scaleFileName_ != "sfFileNone" && domatch_) {
        edm::Ref<JetView> jetRef(uncleanedJets, j);
        const auto genMatched = (*match)[jetRef];

        if (genMatched.isNonnull() && jet.pt() < 50) {
          float jetPUSF = scaleFile_->at("PUJetID_eff")->evaluate({jet.eta(), jet.pt(), "nom", workingPoint});
          float jeffPU = scaleFile_->at("PUJetID_eff")->evaluate({jet.eta(), jet.pt(), "MCEff", workingPoint});
          float mulfac = 1.;

          if (PUID < 7)
            mulfac = (1. - jetPUSF * jeffPU) / (1. - jeffPU);
          else
            mulfac = jetPUSF;

          weight *= mulfac;
        }
      }
    }
  }
  return cleanedJets;
}

DEFINE_FWK_MODULE(CleanedJetCollectionEmbedder);
