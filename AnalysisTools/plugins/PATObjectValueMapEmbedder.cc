///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectValueMapEmbedder                                              //
//                                                                           //
//    Takes a value map and a collection of PAT objects and embeds the       //
//    values from the map as userfloats/ints/bool in the objects             //
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
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Utilities/interface/transform.h"


template<typename T>
class PATObjectValueMapEmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit PATObjectValueMapEmbedder(const edm::ParameterSet& iConfig);
    virtual ~PATObjectValueMapEmbedder() {};

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    const edm::EDGetTokenT<edm::View<T>> srcToken_;
    const std::vector<std::string> intLabels_;
    const std::vector<std::string> boolLabels_;
    const std::vector<std::string> doubleLabels_;
    const std::vector<std::string> floatLabels_;
    const std::vector<edm::EDGetTokenT<edm::ValueMap<int>>> intTokens_;
    const std::vector<edm::EDGetTokenT<edm::ValueMap<bool>>> boolTokens_;
    const std::vector<edm::EDGetTokenT<edm::ValueMap<double>>> doubleTokens_;
    const std::vector<edm::EDGetTokenT<edm::ValueMap<float>>> floatTokens_;
};


template<typename T>
PATObjectValueMapEmbedder<T>::PATObjectValueMapEmbedder(const edm::ParameterSet& iConfig) :
  srcToken_(consumes<edm::View<T> >(iConfig.getParameter<edm::InputTag>("src"))),
  intLabels_(iConfig.getUntrackedParameter<std::vector<std::string>>("intLabels", std::vector<std::string>())),
  boolLabels_(iConfig.getUntrackedParameter<std::vector<std::string>>("boolLabels", std::vector<std::string>())),
  doubleLabels_(iConfig.getUntrackedParameter<std::vector<std::string>>("doubleLabels", std::vector<std::string>())),
  floatLabels_(iConfig.getUntrackedParameter<std::vector<std::string>>("floatLabels", std::vector<std::string>())),
  intTokens_(edm::vector_transform(iConfig.getUntrackedParameter<std::vector<edm::InputTag> >(
    "intVals", std::vector<edm::InputTag>()), 
      [this](edm::InputTag const& tag){return consumes<edm::ValueMap<int> >(tag);})),
  boolTokens_(edm::vector_transform(iConfig.getUntrackedParameter<std::vector<edm::InputTag> >(
    "boolVals", std::vector<edm::InputTag>()), 
      [this](edm::InputTag const& tag){return consumes<edm::ValueMap<bool> >(tag);})),
  doubleTokens_(edm::vector_transform(iConfig.getUntrackedParameter<std::vector<edm::InputTag> >(
    "doubleVals", std::vector<edm::InputTag>()), 
      [this](edm::InputTag const& tag){return consumes<edm::ValueMap<double> >(tag);})),
  floatTokens_(edm::vector_transform(iConfig.getUntrackedParameter<std::vector<edm::InputTag> >(
    "floatVals", std::vector<edm::InputTag>()), 
      [this](edm::InputTag const& tag){return consumes<edm::ValueMap<float> >(tag);}))
{
  produces<std::vector<T> >();

  if (intTokens_.size() != intLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each int you want to embed" 
      << "Given: intLabels_.size() == " << intLabels_.size()
      << "; intTokens_.size() == " << intTokens_.size()
      << std::endl;

  if (boolTokens_.size() != boolLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each bool you want to embed" 
      << "Given: boolLabels_.size() == " << boolLabels_.size()
      << "; boolTokens_.size() == " << boolTokens_.size()
      << std::endl;

  if (doubleTokens_.size() != doubleLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each double you want to embed" 
      << "Given: doubleLabels_.size() == " << doubleLabels_.size()
      << "; doubleTokens_.size() == " << doubleTokens_.size()
      << std::endl;

  if (floatTokens_.size() != floatLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each float you want to embed" 
      << "Given: floatLabels_.size() == " << floatLabels_.size()
      << "; floatTokens_.size() == " << floatTokens_.size()
      << std::endl;
}


template<typename T>
void PATObjectValueMapEmbedder<T>::produce(edm::Event& iEvent,
                                           const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T> > in;
  iEvent.getByToken(srcToken_, in);

  std::unique_ptr<std::vector<T> > out(new std::vector<T>);
  for (size_t i = 0; i < in->size(); i++) 
    out->push_back(in->at(i));

  std::vector<size_t> lengths = {doubleLabels_.size(), floatLabels_.size(), boolLabels_.size(), intLabels_.size()};
  for (size_t i = 0; i < (*std::max_element(lengths.begin(), lengths.end())); i++)
  {
    edm::Handle<edm::ValueMap<int> > intVals;
    edm::Handle<edm::ValueMap<bool> > boolVals;
    edm::Handle<edm::ValueMap<double> > doubleVals;
    edm::Handle<edm::ValueMap<float> > floatVals;
    if (i < intLabels_.size())
      iEvent.getByToken(intTokens_[i], intVals);
    if (i < boolLabels_.size())
      iEvent.getByToken(boolTokens_[i], boolVals);
    if (i < doubleLabels_.size())
      iEvent.getByToken(doubleTokens_[i], doubleVals);
    if (i < floatLabels_.size())
      iEvent.getByToken(floatTokens_[i], floatVals);

    for(size_t j = 0; j < in->size(); ++j)
    {
      T& obj = out->at(j);
      edm::Ptr<T> t = in->ptrAt(j);

      if (i < intLabels_.size())
        obj.addUserInt(intLabels_[i], (*intVals)[t]);
      if (i < boolLabels_.size())
        obj.addUserFloat(boolLabels_[i], (*boolVals)[t]);
      if (i < doubleLabels_.size())
        obj.addUserFloat(doubleLabels_[i], (*doubleVals)[t]);
      if (i < floatLabels_.size())
        obj.addUserFloat(floatLabels_[i], (*floatVals)[t]);
    }
  }
  iEvent.put(std::move(out));
}


typedef PATObjectValueMapEmbedder<pat::Electron> PATElectronValueMapEmbedder;
typedef PATObjectValueMapEmbedder<pat::Muon> PATMuonValueMapEmbedder;
typedef PATObjectValueMapEmbedder<pat::Tau> PATTauValueMapEmbedder;
typedef PATObjectValueMapEmbedder<pat::Jet> PATJetValueMapEmbedder;
typedef PATObjectValueMapEmbedder<pat::CompositeCandidate> PATCompositeCandidateValueMapEmbedder;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronValueMapEmbedder);
DEFINE_FWK_MODULE(PATMuonValueMapEmbedder);
DEFINE_FWK_MODULE(PATTauValueMapEmbedder);
DEFINE_FWK_MODULE(PATJetValueMapEmbedder);
DEFINE_FWK_MODULE(PATCompositeCandidateValueMapEmbedder);
