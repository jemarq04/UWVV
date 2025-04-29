//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetPUIDProducer.cc                                                 //
//                                                                          //
//    Embed TEMPORARY passing PUID userFloat                                //
//                                                                          //
//    Author: Nate Woods, U. Wisconsin                                      //
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

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Common/interface/ValueMap.h"

using pat::Jet, pat::JetCollection;
typedef edm::View<Jet> JetView;

class PATJetPUIDProducer : public edm::stream::EDProducer<>
{
  public:
    explicit PATJetPUIDProducer(const edm::ParameterSet& iConfig);
    virtual ~PATJetPUIDProducer() {;}

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    edm::EDGetTokenT<JetView> srcToken;
    const int value_;
};

PATJetPUIDProducer::PATJetPUIDProducer(const edm::ParameterSet& iConfig) :
  srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  value_(iConfig.exists("value") ? iConfig.getParameter<int>("value") : 7)
{
  produces<edm::ValueMap<int>>("fullId");
}

void PATJetPUIDProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);

  std::vector<int> ids;
  for (size_t i=0; i<in->size(); i++)
    ids.push_back(value_);

  //TODO: add in value maps to event
  auto out = std::make_unique<edm::ValueMap<int>>();
  edm::ValueMap<int>::Filler filler(*out);
  filler.insert(in, ids.begin(), ids.end());
  filler.fill();
  iEvent.put(std::move(out), "fullId");
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetPUIDProducer);
