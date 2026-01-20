///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    FinalDaughterOverlapRemover                                            //
//                                                                           //
//    Takes a collection of PAT CompositeCandidates in which the final       //
//    daughters all have the same type and removes any that have the same    //
//    final daughter twice.                                                  //
//                                                                           //
//    Nate Woods, U. Wisconsin                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// system includes
#include <memory>
#include <vector>
#include <iostream>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/RefToBase.h"

typedef reco::Candidate Cand;
typedef reco::CandidateBaseRef CandRef;
typedef pat::CompositeCandidate CCand;

template <size_t n>
class FinalDaughterOverlapRemover : public edm::stream::EDProducer<> {
public:
  explicit FinalDaughterOverlapRemover(const edm::ParameterSet& iConfig);
  virtual ~FinalDaughterOverlapRemover() { ; }

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  std::vector<CandRef> getDaughterRefs(const CCand& cand) const;

  const edm::EDGetTokenT<edm::View<CCand> > srcToken;
};

template <size_t n>
FinalDaughterOverlapRemover<n>::FinalDaughterOverlapRemover(const edm::ParameterSet& iConfig)
    : srcToken(consumes<edm::View<CCand> >(iConfig.getParameter<edm::InputTag>("src"))) {
  produces<std::vector<CCand> >();
}

template <size_t n>
void FinalDaughterOverlapRemover<n>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<edm::View<CCand> > in;
  std::unique_ptr<std::vector<CCand> > out(new std::vector<CCand>);

  iEvent.getByToken(srcToken, in);

  for (size_t i = 0; i < in->size(); ++i) {
    const CCand& c = in->at(i);

    std::vector<CandRef> finalDaughters = getDaughterRefs(c);

    bool good = true;
    for (size_t d1 = 0; d1 < n - 1; ++d1) {
      for (size_t d2 = d1 + 1; d2 < n; ++d2) {
        if (finalDaughters.at(d1) == finalDaughters.at(d2)) {
          good = false;
          break;
        }
      }
      if (!good)
        break;
    }

    if (good)
      out->push_back(c);
  }

  iEvent.put(std::move(out));
}

template <size_t n>
std::vector<CandRef> FinalDaughterOverlapRemover<n>::getDaughterRefs(const CCand& cand) const {
  std::vector<CandRef> out;

  for (size_t iZ = 0; iZ < n / 2; ++iZ) {
    const CCand* z = static_cast<const CCand*>(cand.daughter(iZ)->masterClone().get());
    out.push_back(z->daughter(0)->masterClone());
    out.push_back(z->daughter(1)->masterClone());
  }

  if (n % 2)
    out.push_back(cand.daughter(n / 2)->masterClone());

  return out;
}

typedef FinalDaughterOverlapRemover<3> FinalDaughterOverlapRemover3L;
typedef FinalDaughterOverlapRemover<4> FinalDaughterOverlapRemover4L;

DEFINE_FWK_MODULE(FinalDaughterOverlapRemover3L);
DEFINE_FWK_MODULE(FinalDaughterOverlapRemover4L);
