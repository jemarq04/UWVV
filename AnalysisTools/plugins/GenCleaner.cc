/////////////////////////////////////////////////////////////////////////////
//
//    GenCleaner
//
//    Reimplementation of PATCleaner that works with gen particle types.
//    Only does delta-R cross cleaning; other functionality removed.
//
//    Nate Woods, U. Wisconsin
//    Based on https://github.com/cms-sw/cmssw/blob/CMSSW_8_1_X/PhysicsTools/PatAlgos/plugins/PATCleaner.h
//
/////////////////////////////////////////////////////////////////////////////

#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "PhysicsTools/PatAlgos/interface/OverlapTest.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"

#include "FWCore/Framework/interface/MakerMacros.h"

#include <vector>
#include <memory>

namespace uwvv {

  template <class ObjType>
  class GenCleaner : public edm::stream::EDProducer<> {
  public:
    explicit GenCleaner(const edm::ParameterSet& iConfig);
    virtual ~GenCleaner() {}

    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  private:
    typedef StringCutObjectSelector<ObjType> Selector;

    const edm::EDGetTokenT<edm::View<ObjType> > srcToken;
    const Selector preselector;
    const Selector finalSelector;

    typedef pat::helper::OverlapTest OverlapTest;
    std::vector<std::unique_ptr<OverlapTest> > overlapTests;
  };

}  // namespace uwvv

template <class ObjType>
uwvv::GenCleaner<ObjType>::GenCleaner(const edm::ParameterSet& iConfig)
    : srcToken(consumes<edm::View<ObjType> >(iConfig.getParameter<edm::InputTag>("src"))),
      preselector(iConfig.exists("preselection") ? iConfig.getParameter<std::string>("preselection") : ""),
      finalSelector(iConfig.exists("finalCut") ? iConfig.getParameter<std::string>("finalCut") : "") {
  edm::ParameterSet overlapPSet = iConfig.getParameter<edm::ParameterSet>("checkOverlaps");
  std::vector<std::string> overlapNames = overlapPSet.getParameterNamesForType<edm::ParameterSet>();
  for (auto& name : overlapNames) {
    edm::ParameterSet cfg = overlapPSet.getParameter<edm::ParameterSet>(name);
    if (cfg.empty())
      continue;

    // We'll only actually use this for delta-R cross cleaning
    cfg.addParameter("checkRecoComponents", false);
    cfg.addParameter("pairCut", std::string(""));
    cfg.addParameter("requireNoOverlaps", true);

    overlapTests.emplace_back(new pat::helper::BasicOverlapTest(name, cfg, consumesCollector()));
  }

  produces<std::vector<ObjType> >();
}

template <class ObjType>
void uwvv::GenCleaner<ObjType>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<edm::View<ObjType> > in;
  iEvent.getByToken(srcToken, in);

  std::unique_ptr<std::vector<ObjType> > out = std::make_unique<std::vector<ObjType> >();

  for (auto& overlap : overlapTests) {
    overlap->readInput(iEvent, iSetup);
  }

  for (size_t i = 0; i < in->size(); ++i) {
    const ObjType& cand = in->at(i);
    if (!preselector(cand))
      continue;

    bool bad = false;
    for (auto& test : overlapTests) {
      reco::CandidatePtrVector overlaps;
      bad |= test->fillOverlapsForItem(cand, overlaps);
      if (bad)
        break;
    }

    if (!bad && finalSelector(cand))
      out->push_back(cand);
  }

  iEvent.put(std::move(out));
}

typedef uwvv::GenCleaner<reco::GenParticle> GenParticleCleaner;
typedef uwvv::GenCleaner<reco::GenJet> GenJetCleaner;
typedef uwvv::GenCleaner<pat::PackedGenParticle> PackedGenParticleCleaner;

DEFINE_FWK_MODULE(GenParticleCleaner);
DEFINE_FWK_MODULE(GenJetCleaner);
DEFINE_FWK_MODULE(PackedGenParticleCleaner);
