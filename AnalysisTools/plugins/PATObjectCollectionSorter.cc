///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectCollectionSorter                                              //
//                                                                           //
//    Takes a string function and a collection of PAT objects and sorts the  //
//    collection using the function value as the comparison value.           //
//                                                                           //
//    Nate Woods, U. Wisconsin                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


// system includes
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Utilities/interface/transform.h"


namespace
{
  template<class T, typename VF, typename CF>
  std::function<bool(const T&,const T&)>
  makeComparator(const VF& valueFunction, const CF& comparisonFunction)
  {
    return std::function<bool(const T&,const T&)>([&valueFunction,comparisonFunction](const T& t1, const T& t2)
                                                  {
                                                    return comparisonFunction(valueFunction(t1),valueFunction(t2));
                                                  });
  }
};

template<class T>
class PATObjectCollectionSorter : public edm::stream::EDProducer<>
{
  public:
    explicit PATObjectCollectionSorter(const edm::ParameterSet& iConfig);
    virtual ~PATObjectCollectionSorter() {;}

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    const edm::EDGetTokenT<edm::View<T> > srcToken_;
    const StringObjectFunction<T,true> fun_;
    const bool ascending_;
    const std::function<bool(const T&,const T&)> comp_;
};


template<class T>
PATObjectCollectionSorter<T>::PATObjectCollectionSorter(const edm::ParameterSet& iConfig) :
  srcToken_(consumes<edm::View<T> >(iConfig.getParameter<edm::InputTag>("src"))),
  fun_(iConfig.getParameter<std::string>("function")),
  ascending_(iConfig.getUntrackedParameter<bool>("ascending", false)),
  comp_(ascending_ ?
        std::function<bool(const T&,const T&)>([&](const T& t1, const T& t2){return fun_(t1) < fun_(t2);}) :
        std::function<bool(const T&,const T&)>([&](const T& t1, const T& t2){return fun_(t1) > fun_(t2);}))
{
  produces<std::vector<T> >();
}


template<class T>
void PATObjectCollectionSorter<T>::produce(edm::Event& iEvent,
                                           const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T> > in;
  iEvent.getByToken(srcToken_, in);

  std::unique_ptr<std::vector<T> > out = std::make_unique<std::vector<T> >();

  for(size_t i = 0; i < in->size(); ++i)
    out->push_back(in->at(i));

  std::sort(out->begin(), out->end(), comp_);

  iEvent.put(std::move(out));
}


typedef PATObjectCollectionSorter<pat::Electron> PATElectronCollectionSorter;
typedef PATObjectCollectionSorter<pat::Muon> PATMuonCollectionSorter;
typedef PATObjectCollectionSorter<pat::Tau> PATTauCollectionSorter;
typedef PATObjectCollectionSorter<pat::Jet> PATJetCollectionSorter;
typedef PATObjectCollectionSorter<pat::CompositeCandidate> PATCompositeCandidateCollectionSorter;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronCollectionSorter);
DEFINE_FWK_MODULE(PATMuonCollectionSorter);
DEFINE_FWK_MODULE(PATTauCollectionSorter);
DEFINE_FWK_MODULE(PATJetCollectionSorter);
DEFINE_FWK_MODULE(PATCompositeCandidateCollectionSorter);
