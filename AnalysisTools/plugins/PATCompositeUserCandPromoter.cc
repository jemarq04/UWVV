///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATCompositeUserCandPromoter                                           //
//                                                                           //
//    Takes a collection of PAT CompositeCandidates, checks their daughters  //
//    to see if the daughters have userCands with a given label, and if so   //
//    adds them to the CompositeCandidates as new daughters.                 //
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
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"


typedef reco::Candidate Cand;
typedef edm::Ptr<Cand> CandPtr;
typedef pat::CompositeCandidate CCand;
typedef edm::Ptr<CCand> CCandPtr;

namespace
{

  template<class T>
  CandPtr getUserCand(const T& cand,
                      const std::string& label)
  {
    if(cand.hasUserCand(label))
      return cand.userCand(label);

    return CandPtr(NULL, 0);
  }

}


template<class T1, class T2>
class PATCompositeUserCandPromoter : public edm::stream::EDProducer<>
{

public:
  explicit PATCompositeUserCandPromoter(const edm::ParameterSet& iConfig);
  virtual ~PATCompositeUserCandPromoter() {};

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  const edm::EDGetTokenT<edm::View<CCand> > srcToken;
  const std::string label;
};


template<class T1, class T2>
PATCompositeUserCandPromoter<T1,T2>::PATCompositeUserCandPromoter(const edm::ParameterSet& iConfig) :
  srcToken(consumes<edm::View<CCand> >(iConfig.getParameter<edm::InputTag>("src"))),
  label(iConfig.getParameter<std::string>("label"))
{
  produces<std::vector<CCand> >();
}


template<class T1, class T2>
void PATCompositeUserCandPromoter<T1,T2>::produce(edm::Event& iEvent,
                                                  const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<CCand> > in;
  std::unique_ptr<std::vector<CCand> > out(new std::vector<CCand>);

  iEvent.getByToken(srcToken, in);

  for(size_t i = 0; i < in->size(); ++i)
  {
    out->push_back(in->at(i));
    CCand& c = out->back();

    const T1* dau1 = static_cast<const T1*>(c.daughter(0)->masterClone().get());
    CandPtr fsr1 = ::getUserCand(*dau1, label);
    const T2* dau2 = static_cast<const T2*>(c.daughter(1)->masterClone().get());
    CandPtr fsr2 = ::getUserCand(*dau2, label);

    size_t nFSR = 0;

    if(fsr1.isNonnull())
    {
      c.setP4(c.p4() + fsr1->p4());
      c.addDaughter(*fsr1, label + std::to_string(nFSR));
      nFSR++;
    }
    if(fsr2.isNonnull())
    {
      c.setP4(c.p4() + fsr2->p4());
      c.addDaughter(*fsr2, label + std::to_string(nFSR));
      nFSR++;
    }

    c.addUserInt("n"+label+"Cands", nFSR);
  }

  iEvent.put(std::move(out));
}


typedef PATCompositeUserCandPromoter<pat::Electron, pat::Electron> PATElectronCompositeUserCandPromoter;
typedef PATCompositeUserCandPromoter<pat::Muon, pat::Muon> PATMuonCompositeUserCandPromoter;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronCompositeUserCandPromoter);
DEFINE_FWK_MODULE(PATMuonCompositeUserCandPromoter);
