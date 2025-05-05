//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATObjectValueMapProducer.cc                                          //
//                                                                          //
//    Embed TEMPORARY passing PUID userFloat                                //
//                                                                          //
//    Author: Justin Marquez, U. Wisconsin                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <string>
#include <vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

template <typename T>
class PATObjectValueMapProducer : public edm::stream::EDProducer<>
{
  public:
    explicit PATObjectValueMapProducer(const edm::ParameterSet& iConfig);
    virtual ~PATObjectValueMapProducer() {;}

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    typedef edm::View<T> ViewT;

    edm::EDGetTokenT<ViewT> srcToken;
    const int intVal_;
    std::string label_;
};

template <typename T>
PATObjectValueMapProducer<T>::PATObjectValueMapProducer(const edm::ParameterSet& iConfig) :
  srcToken(consumes<ViewT>(iConfig.getParameter<edm::InputTag>("src"))),
  intVal_(iConfig.getParameter<int>("intVal")),
  label_(iConfig.getParameter<std::string>("label"))
{
  produces<edm::ValueMap<int>>(label_);
}

template <typename T>
void PATObjectValueMapProducer<T>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<ViewT> in;
  iEvent.getByToken(srcToken, in);

  std::vector<int> ids;
  for (size_t i=0; i<in->size(); i++)
    ids.push_back(intVal_);

  auto out = std::make_unique<edm::ValueMap<int>>();
  edm::ValueMap<int>::Filler filler(*out);
  filler.insert(in, ids.begin(), ids.end());
  filler.fill();
  iEvent.put(std::move(out), label_);
}

typedef PATObjectValueMapProducer<pat::Jet> PATJetValueMapProducer;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetValueMapProducer);
