///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectValueMapProducer.cc                                           //
//                                                                           //
//    Embed TEMPORARY passing PUID userFloat                                 //
//                                                                           //
//    Author: Justin Marquez, U. Wisconsin                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <string>
#include <vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

template <typename T>
class PATObjectValueMapProducer : public edm::stream::EDProducer<>
{
  public:
    explicit PATObjectValueMapProducer(const edm::ParameterSet& iConfig);
    virtual ~PATObjectValueMapProducer() {;}

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    const edm::EDGetTokenT<edm::View<T> > srcToken;
    const std::vector<int> intVals_;
    const std::vector<int> boolVals_;
    const std::vector<double> doubleVals_;
    const std::vector<double> floatVals_;
    const std::vector<std::string> intLabels_;
    const std::vector<std::string> boolLabels_;
    const std::vector<std::string> doubleLabels_;
    const std::vector<std::string> floatLabels_;
};

template <typename T>
PATObjectValueMapProducer<T>::PATObjectValueMapProducer(const edm::ParameterSet& iConfig) :
  srcToken(consumes<edm::View<T> >(iConfig.getParameter<edm::InputTag>("src"))),
  intVals_(iConfig.exists("intVals") ?
      iConfig.getParameter<std::vector<int>>("intVals") :
      std::vector<int>()),
  boolVals_(iConfig.exists("boolVals") ?
      iConfig.getParameter<std::vector<int>>("boolVals") :
      std::vector<int>()),
  doubleVals_(iConfig.exists("doubleVals") ?
      iConfig.getParameter<std::vector<double>>("doubleVals") :
      std::vector<double>()),
  floatVals_(iConfig.exists("floatVals") ?
      iConfig.getParameter<std::vector<double>>("floatVals") :
      std::vector<double>()),
  intLabels_(iConfig.exists("intLabels") ?
      iConfig.getParameter<std::vector<std::string>>("intLabels") :
      std::vector<std::string>()),
  boolLabels_(iConfig.exists("boolLabels") ?
      iConfig.getParameter<std::vector<std::string>>("boolLabels") :
      std::vector<std::string>()),
  doubleLabels_(iConfig.exists("doubleLabels") ?
      iConfig.getParameter<std::vector<std::string>>("doubleLabels") :
      std::vector<std::string>()),
  floatLabels_(iConfig.exists("floatLabels") ?
      iConfig.getParameter<std::vector<std::string>>("floatLabels") :
      std::vector<std::string>())
{
  for (auto &l : intLabels_)
    produces<edm::ValueMap<int>>(l);
  for (auto &l : boolLabels_)
    produces<edm::ValueMap<bool>>(l);
  for (auto &l : doubleLabels_)
    produces<edm::ValueMap<double>>(l);
  for (auto &l : floatLabels_)
    produces<edm::ValueMap<float>>(l);
  
  if (intVals_.size() != intLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each int you want to embed" 
      << "Given: intLabels_.size() == " << intLabels_.size()
      << "; intVals_.size() == " << intVals_.size()
      << std::endl;

  if (boolVals_.size() != boolLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each bool you want to embed" 
      << "Given: boolLabels_.size() == " << boolLabels_.size()
      << "; boolVals_.size() == " << boolVals_.size()
      << std::endl;

  if (doubleVals_.size() != doubleLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each double you want to embed" 
      << "Given: doubleLabels_.size() == " << doubleLabels_.size()
      << "; doubleVals_.size() == " << doubleVals_.size()
      << std::endl;

  if (floatVals_.size() != floatLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each float you want to embed" 
      << "Given: floatLabels_.size() == " << floatLabels_.size()
      << "; floatVals_.size() == " << floatVals_.size()
      << std::endl;
}

template <typename T>
void PATObjectValueMapProducer<T>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T> > in;
  iEvent.getByToken(srcToken, in);
  
  std::vector<size_t> lengths = {intLabels_.size(), boolLabels_.size(), doubleLabels_.size(), floatLabels_.size()};
  for (size_t i=0; i<(*std::max_element(lengths.begin(), lengths.end())); i++)
  {
    std::vector<int> intSrc;
    std::vector<bool> boolSrc;
    std::vector<double> doubleSrc;
    std::vector<float> floatSrc;

    if (i < intVals_.size()){
      for (size_t j=0; j<in->size(); j++)
        intSrc.push_back(intVals_[i]);
      auto out = std::make_unique<edm::ValueMap<int>>();
      edm::ValueMap<int>::Filler filler(*out);
      filler.insert(in, intSrc.begin(), intSrc.end());
      filler.fill();
      iEvent.put(std::move(out), intLabels_[i]);
    }
    if (i < boolVals_.size()){
      for (size_t j=0; j<in->size(); j++)
        boolSrc.push_back(boolVals_[i] != 0);
      auto out = std::make_unique<edm::ValueMap<bool>>();
      edm::ValueMap<bool>::Filler filler(*out);
      filler.insert(in, boolSrc.begin(), boolSrc.end());
      filler.fill();
      iEvent.put(std::move(out), boolLabels_[i]);
    }
    if (i < doubleVals_.size()){
      for (size_t j=0; j<in->size(); j++)
        doubleSrc.push_back(doubleVals_[i]);
      auto out = std::make_unique<edm::ValueMap<double>>();
      edm::ValueMap<double>::Filler filler(*out);
      filler.insert(in, doubleSrc.begin(), doubleSrc.end());
      filler.fill();
      iEvent.put(std::move(out), doubleLabels_[i]);
    }
    if (i < floatVals_.size()){
      for (size_t j=0; j<in->size(); j++)
        floatSrc.push_back(floatVals_[i]);
      auto out = std::make_unique<edm::ValueMap<float>>();
      edm::ValueMap<float>::Filler filler(*out);
      filler.insert(in, floatSrc.begin(), floatSrc.end());
      filler.fill();
      iEvent.put(std::move(out), floatLabels_[i]);
    }
  }
}

typedef PATObjectValueMapProducer<pat::Electron> PATElectronValueMapProducer;
typedef PATObjectValueMapProducer<pat::Muon> PATMuonValueMapProducer;
typedef PATObjectValueMapProducer<pat::Tau> PATTauValueMapProducer;
typedef PATObjectValueMapProducer<pat::Jet> PATJetValueMapProducer;
typedef PATObjectValueMapProducer<pat::CompositeCandidate> PATCompositeCandidateValueMapProducer;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronValueMapProducer);
DEFINE_FWK_MODULE(PATMuonValueMapProducer);
DEFINE_FWK_MODULE(PATTauValueMapProducer);
DEFINE_FWK_MODULE(PATJetValueMapProducer);
DEFINE_FWK_MODULE(PATCompositeCandidateValueMapProducer);
