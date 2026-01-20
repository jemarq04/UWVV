///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectExpressionEmbedder                                            //
//                                                                           //
//    Takes a string function and a collection of PAT objects and embeds     //
//    the results of the function in the objects as userFloats.              //
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
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Utilities/interface/transform.h"

template <class T>
class PATObjectExpressionEmbedder : public edm::stream::EDProducer<> {
public:
  explicit PATObjectExpressionEmbedder(const edm::ParameterSet& iConfig);
  virtual ~PATObjectExpressionEmbedder() {};

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);
  void embedValue(T& object, float value, const std::string& label) const;

  const edm::EDGetTokenT<edm::View<T> > srcToken_;
  const std::vector<std::string> labels_;
  const std::vector<StringObjectFunction<T, true> > functions_;
};

template <class T>
PATObjectExpressionEmbedder<T>::PATObjectExpressionEmbedder(const edm::ParameterSet& iConfig)
    : srcToken_(consumes<edm::View<T> >(iConfig.getParameter<edm::InputTag>("src"))),
      labels_(iConfig.getUntrackedParameter<std::vector<std::string> >("labels")),
      functions_(
          edm::vector_transform(iConfig.getUntrackedParameter<std::vector<std::string> >("functions"),
                                [this](const std::string& expr) { return StringObjectFunction<T, true>(expr); })) {
  if (labels_.size() != functions_.size())
    throw cms::Exception("InvalidParams") << "Must have exactly one label for each expression.";

  produces<std::vector<T> >();
}

template <class T>
void PATObjectExpressionEmbedder<T>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  std::unique_ptr<std::vector<T> > out = std::make_unique<std::vector<T> >();
  edm::Handle<edm::View<T> > in;
  iEvent.getByToken(srcToken_, in);

  for (size_t i = 0; i < in->size(); ++i) {
    out->push_back(in->at(i));

    for (size_t j = 0; j < labels_.size(); ++j)
      embedValue(out->back(), functions_.at(j)(out->back()), labels_.at(j));
  }

  iEvent.put(std::move(out));
}

template <class T>
void PATObjectExpressionEmbedder<T>::embedValue(T& object, float value, const std::string& label) const {
  object.addUserFloat(label, value);
}

typedef PATObjectExpressionEmbedder<pat::Electron> PATElectronExpressionEmbedder;
typedef PATObjectExpressionEmbedder<pat::Muon> PATMuonExpressionEmbedder;
typedef PATObjectExpressionEmbedder<pat::Tau> PATTauExpressionEmbedder;
typedef PATObjectExpressionEmbedder<pat::Jet> PATJetExpressionEmbedder;
typedef PATObjectExpressionEmbedder<pat::CompositeCandidate> PATCompositeCandidateExpressionEmbedder;

DEFINE_FWK_MODULE(PATElectronExpressionEmbedder);
DEFINE_FWK_MODULE(PATMuonExpressionEmbedder);
DEFINE_FWK_MODULE(PATTauExpressionEmbedder);
DEFINE_FWK_MODULE(PATJetExpressionEmbedder);
DEFINE_FWK_MODULE(PATCompositeCandidateExpressionEmbedder);
