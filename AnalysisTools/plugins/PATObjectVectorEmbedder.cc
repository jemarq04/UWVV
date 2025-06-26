///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectVectorEmbedder                                                //
//                                                                           //
//    Takes a collection of PAT objects and some vectors of ints, bools,     //
//    floats, and doubles, and embeds them as userData in the objects        //
//                                                                           //
//    Justin Marquez, U. Wisconsin                                           //
//     (adapted from PATObjectValueEmbedder)                                 //
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
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Utilities/interface/transform.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"


template<class T>
class PATObjectVectorEmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit PATObjectVectorEmbedder(const edm::ParameterSet& iConfig);
    virtual ~PATObjectVectorEmbedder() {};

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    template<typename V>
    void embedAll(std::vector<T>& objects, const std::vector<V>& values,
          const std::vector<std::string>& labels) const;

    template<typename V>
    void retrieveValues(std::vector<V>& toFill,
          const std::vector<edm::EDGetTokenT<V> >& tokens,
          const edm::Event& iEvent) const;

    const edm::EDGetTokenT<edm::View<T> > srcToken_;
    const std::vector<edm::EDGetTokenT<std::vector<int> > > intVecTokens_;
    const std::vector<edm::EDGetTokenT<std::vector<bool> > > boolVecTokens_;
    const std::vector<edm::EDGetTokenT<std::vector<double> > > doubleVecTokens_;
    const std::vector<edm::EDGetTokenT<std::vector<float> > > floatVecTokens_;
    const std::vector<std::string> intLabels_;
    const std::vector<std::string> boolLabels_;
    const std::vector<std::string> doubleLabels_;
    const std::vector<std::string> floatLabels_;

};


template<class T>
PATObjectVectorEmbedder<T>::PATObjectVectorEmbedder(const edm::ParameterSet& iConfig) :
  srcToken_(consumes<edm::View<T> >(iConfig.getParameter<edm::InputTag>("src"))),
  intVecTokens_(edm::vector_transform(iConfig.exists("intSrc") ?
      iConfig.getParameter<std::vector<edm::InputTag> >("intSrc") :
      std::vector<edm::InputTag>(),
      [this](edm::InputTag const& tag){return consumes<std::vector<int> >(tag);})),
  boolVecTokens_(edm::vector_transform(iConfig.exists("boolSrc") ?
      iConfig.getParameter<std::vector<edm::InputTag> >("boolSrc") :
      std::vector<edm::InputTag>(),
      [this](edm::InputTag const& tag){return consumes<std::vector<bool> >(tag);})),
  doubleVecTokens_(edm::vector_transform(iConfig.exists("doubleSrc") ?
      iConfig.getParameter<std::vector<edm::InputTag> >("doubleSrc") :
      std::vector<edm::InputTag>(),
      [this](edm::InputTag const& tag){return consumes<std::vector<double> >(tag);})),
  floatVecTokens_(edm::vector_transform(iConfig.exists("floatSrc") ?
      iConfig.getParameter<std::vector<edm::InputTag> >("floatSrc") :
      std::vector<edm::InputTag>(),
      [this](edm::InputTag const& tag){return consumes<std::vector<float> >(tag);})),
  intLabels_(iConfig.exists("intLabels") ?
      iConfig.getParameter<std::vector<std::string> >("intLabels") :
      std::vector<std::string>()),
  boolLabels_(iConfig.exists("boolLabels") ?
      iConfig.getParameter<std::vector<std::string> >("boolLabels") :
      std::vector<std::string>()),
  doubleLabels_(iConfig.exists("doubleLabels") ?
      iConfig.getParameter<std::vector<std::string> >("doubleLabels") :
      std::vector<std::string>()),
  floatLabels_(iConfig.exists("floatLabels") ?
      iConfig.getParameter<std::vector<std::string> >("floatLabels") :
      std::vector<std::string>())
{
  produces<std::vector<T> >();

  if (intVecTokens_.size() != intLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each int you want to embed"
      << "Given: intLabels_.size() == " << intLabels_.size()
      << "; intVecTokens_.size() == " << intVecTokens_.size()
      << std::endl;

  if (boolVecTokens_.size() != boolLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each bool you want to embed"
      << "Given: boolLabels_.size() == " << boolLabels_.size()
      << "; boolVecTokens_.size() == " << boolVecTokens_.size()
      << std::endl;

  if (doubleVecTokens_.size() != doubleLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each double you want to embed"
      << "Given: doubleLabels_.size() == " << doubleLabels_.size()
      << "; doubleVecTokens_.size() == " << doubleVecTokens_.size()
      << std::endl;

  if (floatVecTokens_.size() != floatLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each float you want to embed"
      << "Given: floatLabels_.size() == " << floatLabels_.size()
      << "; floatVecTokens_.size() == " << floatVecTokens_.size()
      << std::endl;
}


template<class T>
void PATObjectVectorEmbedder<T>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T> > in;
  iEvent.getByToken(srcToken_, in);

  std::unique_ptr<std::vector<T> > out(new std::vector<T>);
  for(size_t i = 0; i < in->size(); ++i)
    out->push_back(in->at(i));

  std::vector<std::vector<int> > intVecs;
  retrieveValues(intVecs, intVecTokens_, iEvent);
  embedAll(*out, intVecs, intLabels_);

  std::vector<std::vector<bool> > boolVecs;
  retrieveValues(boolVecs, boolVecTokens_, iEvent);
  embedAll(*out, boolVecs, boolLabels_);

  std::vector<std::vector<double> > doubleVecs;
  retrieveValues(doubleVecs, doubleVecTokens_, iEvent);
  embedAll(*out, doubleVecs, doubleLabels_);

  std::vector<std::vector<float> > floatVecs;
  retrieveValues(floatVecs, floatVecTokens_, iEvent);
  embedAll(*out, floatVecs, floatLabels_);

  iEvent.put(std::move(out));
}

template<class T>
template<typename V>
void PATObjectVectorEmbedder<T>::retrieveValues(std::vector<V>& toFill,
    const std::vector<edm::EDGetTokenT<V> >& tokens, const edm::Event& iEvent) const
{
  toFill.clear();

  for(auto& t : tokens)
  {
    edm::Handle<V> h;
    iEvent.getByToken(t, h);
    toFill.push_back(*h);
  }
}

template<class T>
template<typename V>
void PATObjectVectorEmbedder<T>::embedAll(std::vector<T>& objects,
    const std::vector<V>& values, const std::vector<std::string>& labels) const
{
  for (auto& obj : objects)
    for (size_t i = 0; i < values.size(); ++i)
      obj.addUserData(labels.at(i), values.at(i));
}

typedef PATObjectVectorEmbedder<pat::Electron> PATElectronVectorEmbedder;
typedef PATObjectVectorEmbedder<pat::Muon> PATMuonVectorEmbedder;
typedef PATObjectVectorEmbedder<pat::Tau> PATTauVectorEmbedder;
typedef PATObjectVectorEmbedder<pat::Jet> PATJetVectorEmbedder;
typedef PATObjectVectorEmbedder<pat::CompositeCandidate> PATCompositeCandidateVectorEmbedder;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronVectorEmbedder);
DEFINE_FWK_MODULE(PATMuonVectorEmbedder);
DEFINE_FWK_MODULE(PATTauVectorEmbedder);
DEFINE_FWK_MODULE(PATJetVectorEmbedder);
DEFINE_FWK_MODULE(PATCompositeCandidateVectorEmbedder);
