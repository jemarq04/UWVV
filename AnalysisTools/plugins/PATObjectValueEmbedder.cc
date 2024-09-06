///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectValueEmbedder                                                 //
//                                                                           //
//    Takes a collection of PAT objects and some ints, bools, float, and     //
//    doubles, and embeds them as userInts/userFloats in the objects         //
//                                                                           //
//    Nate Woods and Kenneth Long, U. Wisconsin                              //
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
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Utilities/interface/transform.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"


template<class T>
class PATObjectValueEmbedder : public edm::stream::EDProducer<>
{

public:
  explicit PATObjectValueEmbedder(const edm::ParameterSet& iConfig);
  virtual ~PATObjectValueEmbedder() {};

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);
  void embedValue(T& object, const int value, const std::string& label) const;
  void embedValue(T& object, const bool value, const std::string& label) const;
  void embedValue(T& object, const double value, const std::string& label) const;
  void embedValue(T& object, const float value, const std::string& label) const;
  template<typename V>
  void embedAll(std::vector<T>& objects, const std::vector<V>& values,
                const std::vector<std::string>& labels) const;

  template<typename V>
  void retrieveValues(std::vector<V>& toFill, 
                      const std::vector<edm::EDGetTokenT<V> >& tokens, 
                      const edm::Event& iEvent) const;

  const bool replaceFlag_; //get BadPFMuonFilter and BadChargedCandidateFilter flags directly from miniAOD
  const edm::EDGetTokenT<edm::View<T> > srcToken_;
  edm::EDGetTokenT<edm::TriggerResults > trigToken_;
  const std::vector<edm::EDGetTokenT<int> > intTokens_;
  const std::vector<edm::EDGetTokenT<bool> > boolTokens_;
  const std::vector<edm::EDGetTokenT<double> > doubleTokens_;
  const std::vector<edm::EDGetTokenT<float> > floatTokens_;
  const std::vector<std::string> intLabels_;
  const std::vector<std::string> boolLabels_;
  const std::vector<std::string> doubleLabels_;
  const std::vector<std::string> floatLabels_;
  
};


template<class T>
PATObjectValueEmbedder<T>::PATObjectValueEmbedder(const edm::ParameterSet& iConfig) :
  replaceFlag_(iConfig.exists("replaceFlag") ? iConfig.getParameter<bool>("replaceFlag") : false),
  srcToken_(consumes<edm::View<T> >(iConfig.getParameter<edm::InputTag>("src"))),
  intTokens_(edm::vector_transform(iConfig.exists("intSrc") ?
                                   iConfig.getParameter<std::vector<edm::InputTag> >("intSrc") :
                                   std::vector<edm::InputTag>(),
                                   [this](edm::InputTag const& tag){return consumes<int>(tag);})),
  boolTokens_(edm::vector_transform(iConfig.exists("boolSrc") ?
                                    iConfig.getParameter<std::vector<edm::InputTag> >("boolSrc") : 
                                    std::vector<edm::InputTag>(),
                                    [this](edm::InputTag const& tag){return consumes<bool>(tag);})),
  doubleTokens_(edm::vector_transform(iConfig.exists("doubleSrc") ?
                                      iConfig.getParameter<std::vector<edm::InputTag> >("doubleSrc") :
                                      std::vector<edm::InputTag>(),
                                      [this](edm::InputTag const& tag){return consumes<double>(tag);})),
  floatTokens_(edm::vector_transform(iConfig.exists("floatSrc") ?
                                     iConfig.getParameter<std::vector<edm::InputTag> >("floatSrc") :
                                     std::vector<edm::InputTag>(),
                                     [this](edm::InputTag const& tag){return consumes<float>(tag);})),
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

  if (replaceFlag_){
    trigToken_ = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults","","PAT"));
  }

  if(intTokens_.size() != intLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each int you want to embed" 
      << "Given: intLabels_.size() == " << intLabels_.size()
      << "; intTokens_.size() == " << intTokens_.size()
      << std::endl;

  if(boolTokens_.size() != boolLabels_.size() && !replaceFlag_)
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each bool you want to embed" 
      << "Given: boolLabels_.size() == " << boolLabels_.size()
      << "; boolTokens_.size() == " << boolTokens_.size()
      << std::endl;

  if(doubleTokens_.size() != doubleLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each double you want to embed" 
      << "Given: doubleLabels_.size() == " << doubleLabels_.size()
      << "; doubleTokens_.size() == " << doubleTokens_.size()
      << std::endl;

  if(floatTokens_.size() != floatLabels_.size())
    throw cms::Exception("InvalidParams")
      << "You must supply exactly one label for each float you want to embed" 
      << "Given: floatLabels_.size() == " << floatLabels_.size()
      << "; floatTokens_.size() == " << floatTokens_.size()
      << std::endl;
}


template<class T>
void PATObjectValueEmbedder<T>::produce(edm::Event& iEvent,
                                        const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T> > in;
  std::unique_ptr<std::vector<T> > out(new std::vector<T>);
  iEvent.getByToken(srcToken_, in);
  for(size_t i = 0; i < in->size(); ++i)
    out->push_back(in->at(i));
  
  std::vector<int> ints;
  retrieveValues(ints, intTokens_, iEvent);
  embedAll(*out, ints, intLabels_);

  std::vector<bool> bools;
  retrieveValues(bools, boolTokens_, iEvent);
  embedAll(*out, bools, boolLabels_);

  std::vector<bool> bools2;
  bools2.clear();

  if (replaceFlag_){ //handle special case BadMuonFilters.py in UL, in this case variable bools should be empty 
  edm::Handle<edm::TriggerResults> trigResults;
  iEvent.getByToken(trigToken_, trigResults);
  const edm::TriggerNames& trigNames = iEvent.triggerNames(*trigResults);
  std::string pathName1 = "Flag_BadPFMuonFilter";
  std::string pathName2 = "Flag_BadChargedCandidateFilter";
  bool passTrig1=trigResults->accept(trigNames.triggerIndex(pathName1));
  bools2.push_back(passTrig1);
  bool passTrig2=trigResults->accept(trigNames.triggerIndex(pathName2));
  bools2.push_back(passTrig2);
  embedAll(*out, bools2, boolLabels_);
  }
  
  std::vector<double> doubles;
  retrieveValues(doubles, doubleTokens_, iEvent);
  embedAll(*out, doubles, doubleLabels_);

  std::vector<float> floats;
  retrieveValues(floats, floatTokens_, iEvent);
  embedAll(*out, floats, floatLabels_);

  iEvent.put(std::move(out));
}

template<class T>
void PATObjectValueEmbedder<T>::embedValue(T& object,
                                           const double value, 
                                           const std::string& label) const
{
    object.addUserFloat(label, value);
}

template<class T>
void PATObjectValueEmbedder<T>::embedValue(T& object,
                                           const float value, 
                                           const std::string& label) const
{
    object.addUserFloat(label, value);
}

template<class T>
void PATObjectValueEmbedder<T>::embedValue(T& object,
                                           const int value, 
                                           const std::string& label) const
{
    object.addUserInt(label, value);
}

template<class T>
void PATObjectValueEmbedder<T>::embedValue(T& object,
                                           const bool value, 
                                           const std::string& label) const
{
    object.addUserFloat(label, value);
}

template<class T>
template<typename V>
void PATObjectValueEmbedder<T>::retrieveValues(std::vector<V>& toFill,
                                               const std::vector<edm::EDGetTokenT<V> >& tokens,
                                               const edm::Event& iEvent) const
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
void PATObjectValueEmbedder<T>::embedAll(std::vector<T>& objects, 
                                         const std::vector<V>& values,
                                         const std::vector<std::string>& labels) const
{
  for(auto& obj : objects)
    {
      for(size_t i = 0; i < values.size(); ++i)
        {
          embedValue(obj, values.at(i), labels.at(i));
        }
    }
}

typedef PATObjectValueEmbedder<pat::Electron> PATElectronValueEmbedder;
typedef PATObjectValueEmbedder<pat::Muon> PATMuonValueEmbedder;
typedef PATObjectValueEmbedder<pat::Tau> PATTauValueEmbedder;
typedef PATObjectValueEmbedder<pat::Jet> PATJetValueEmbedder;
typedef PATObjectValueEmbedder<pat::CompositeCandidate> PATCompositeCandidateValueEmbedder;

DEFINE_FWK_MODULE(PATElectronValueEmbedder);
DEFINE_FWK_MODULE(PATMuonValueEmbedder);
DEFINE_FWK_MODULE(PATTauValueEmbedder);
DEFINE_FWK_MODULE(PATJetValueEmbedder);
DEFINE_FWK_MODULE(PATCompositeCandidateValueEmbedder);
