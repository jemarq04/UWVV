///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectUserCandEmbedder                                              //
//                                                                           //
//    Takes two collections of PAT objects and embeds one in the other as    //
//    UserCands                                                              //
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
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/Utilities/interface/transform.h"

typedef reco::Candidate Cand;
typedef reco::CandidatePtr CandPtr;
typedef edm::View<reco::Candidate> CandView;


template<class T>
class PATObjectUserCandEmbedder : public edm::stream::EDProducer<>
{

public:
  explicit PATObjectUserCandEmbedder(const edm::ParameterSet& iConfig);
  virtual ~PATObjectUserCandEmbedder() {};

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  const std::string label_;

  const edm::EDGetTokenT<edm::View<T> > srcToken_;
  const edm::EDGetTokenT<CandView> srcToEmbedToken_;
};


template<class T>
PATObjectUserCandEmbedder<T>::PATObjectUserCandEmbedder(const edm::ParameterSet& iConfig) :
  label_(iConfig.getParameter<std::string>("label")),
  srcToken_(consumes<edm::View<T> >(iConfig.getParameter<edm::InputTag>("src"))),
  srcToEmbedToken_(consumes<CandView>(iConfig.getParameter<edm::InputTag>("srcToEmbed")))
{
  produces<std::vector<T> >();
}


template<class T>
void PATObjectUserCandEmbedder<T>::produce(edm::Event& iEvent,
                                           const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T> > in;
  std::unique_ptr<std::vector<T> > out(new std::vector<T>);
  iEvent.getByToken(srcToken_, in);

  edm::Handle<CandView> toEmbed;
  iEvent.getByToken(srcToEmbedToken_, toEmbed);

  for(size_t i = 0; i < in->size(); ++i)
    out->push_back(in->at(i)); // copy, moving ownership

  for(size_t j = 0; j < toEmbed->size(); ++j)
    {
      const CandPtr& ptr = toEmbed->ptrAt(j);
      const std::string name = label_ + std::to_string(j);
      for(auto& objOut : *out)
        objOut.addUserCand(name, ptr);
    }

  iEvent.put(std::move(out));
}


typedef PATObjectUserCandEmbedder<pat::Electron> PATElectronUserCandEmbedder;
typedef PATObjectUserCandEmbedder<pat::Muon> PATMuonUserCandEmbedder;
typedef PATObjectUserCandEmbedder<pat::CompositeCandidate> PATCompositeCandidateUserCandEmbedder;

DEFINE_FWK_MODULE(PATElectronUserCandEmbedder);
DEFINE_FWK_MODULE(PATMuonUserCandEmbedder);
DEFINE_FWK_MODULE(PATCompositeCandidateUserCandEmbedder);
