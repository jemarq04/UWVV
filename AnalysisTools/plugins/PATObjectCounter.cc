///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectCounter                                                       //
//                                                                           //
//    Takes a collection of PAT objects and adss and int of the     //
//    number of objects passing the (otional) specified cut to the event     //
//                                                                           //
//    Kenneth Long, U. Wisconsin                                             //
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
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"


template<typename T>
class PATObjectCounter : public edm::stream::EDProducer<>
{
  public:
    explicit PATObjectCounter(const edm::ParameterSet& iConfig);
    virtual ~PATObjectCounter() {};

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    void printInfo(edm::Ptr<T> obj, int index=0);

    const edm::InputTag srcTag_;
    const edm::EDGetTokenT<edm::View<T> > srcToken_;
    const std::vector<std::string> cut_strings_;
    const std::vector<std::string> labels_;
    const bool verbose_, doPrintInfo_;
    std::vector<StringCutObjectSelector<T>> cuts_;
};


template<typename T>
PATObjectCounter<T>::PATObjectCounter(const edm::ParameterSet& iConfig) :
  srcTag_(iConfig.getParameter<edm::InputTag>("src")),
  srcToken_(consumes<edm::View<T> >(srcTag_)),
  cut_strings_(iConfig.exists("cuts") ?
             iConfig.getParameter<std::vector<std::string> >("cuts") :
             std::vector<std::string>()),
  labels_(iConfig.exists("labels") ?
             iConfig.getParameter<std::vector<std::string> >("labels") :
             std::vector<std::string>()),
  verbose_(iConfig.exists("verbose") ? 
             iConfig.getParameter<bool>("verbose") : 
             false),
  doPrintInfo_(iConfig.exists("printInfo") ?
             iConfig.getParameter<bool>("printInfo") :
             false)
{
  if (cut_strings_.size() != labels_.size())
      throw cms::Exception("InvalidParams")
          << "You must supply an equal number of labels and cuts" << std::endl
          << "Given: labels_->size() == " << labels_.size()
          << "; cut_strings_->size() == " << cut_strings_.size()
          << std::endl;

  for (size_t i=0; i<labels_.size(); i++)
  {
    produces<int>(labels_[i]);
    cuts_.push_back(cut_strings_[i]);
  }
}


template<typename T>
void PATObjectCounter<T>::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T> > in;
  iEvent.getByToken(srcToken_, in);
  
  for (size_t i = 0; i < cut_strings_.size(); i++) 
  {
    std::unique_ptr<int> num(new int(0));
    if (cut_strings_[i] != "")
    {
      for (size_t j = 0; j < in->size(); j++)
        if (cuts_[i](in->at(j)))
          (*num)++;
    }
    else *num = in->size();
    if (verbose_) std::cout << srcTag_.label() << " Count: " << labels_[i] << " = " << *num << std::endl;
    iEvent.put(std::move(num), labels_[i]);
  }

  if (doPrintInfo_)
  {
    for (size_t j=0; j<in->size(); j++)
    {
      edm::Ptr<T> obj(in, j);
      printInfo(obj, j);
    }
  }
}

template <typename T>
void PATObjectCounter<T>::printInfo(edm::Ptr<T> obj, int index){}
template <>
void PATObjectCounter<pat::Muon>::printInfo(edm::Ptr<pat::Muon> muon, int index){
  std::cout << "  Object " << index << std::endl;
  std::cout << "    pT = " << muon->pt() << std::endl;
  std::cout << "    eta = " << muon->eta() << std::endl;
  std::cout << "    phi = " << muon->phi() << std::endl;
  //std::cout << "    type = " << muon->type() << std::endl;
}
template <>
void PATObjectCounter<pat::Electron>::printInfo(edm::Ptr<pat::Electron> elec, int index){
  std::cout << "  Object " << index << std::endl;
  std::cout << "    pT = " << elec->pt() << std::endl;
  std::cout << "    eta = " << elec->eta() << std::endl;
  std::cout << "    phi = " << elec->phi() << std::endl;
}

typedef PATObjectCounter<pat::Electron> PATElectronCounter;
typedef PATObjectCounter<pat::Muon> PATMuonCounter;
typedef PATObjectCounter<pat::Tau> PATTauCounter;
typedef PATObjectCounter<pat::Jet> PATJetCounter;
typedef PATObjectCounter<pat::CompositeCandidate> PATCompositeCandidateCounter;

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronCounter);
DEFINE_FWK_MODULE(PATMuonCounter);
DEFINE_FWK_MODULE(PATTauCounter);
DEFINE_FWK_MODULE(PATJetCounter);
DEFINE_FWK_MODULE(PATCompositeCandidateCounter);
