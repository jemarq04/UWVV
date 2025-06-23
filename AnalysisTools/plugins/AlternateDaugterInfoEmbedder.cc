///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    AlternateDaughterInfoEmbedder                                          //
//                                                                           //
//    Given a collection of pat::CompositeCandidates made of two two-object  //
//    CompositeCandidates, embeds some information about the alternate       //
//    dilepton pairings.                                                     //
//                                                                           //
//    Nate Woods, U. Wisconsin                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


// system includes
#include <memory>
#include <vector>
#include <string>
#include <iostream>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Math/interface/deltaR.h"


typedef pat::CompositeCandidate CCand;
typedef pat::CompositeCandidateCollection CCandCollection;
typedef edm::View<CCand> CCandView;

class AlternateDaughterInfoEmbedder : public edm::stream::EDProducer<>
{

public:
  explicit AlternateDaughterInfoEmbedder(const edm::ParameterSet& iConfig);
  virtual ~AlternateDaughterInfoEmbedder() {};

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  const std::vector<std::string>
  makePairNames(const std::vector<std::string>& names) const;

  const edm::EDGetTokenT<CCandView> srcToken;

  const std::vector<std::string> names;
  const std::vector<std::string> pairNames;
  const size_t nDaughters;

  const std::string fsrLabel;

  // have to do some stupid crap to cast to the right pat object type to get
  // user cands
  template<class T>
  const edm::Ptr<reco::Candidate>
  getFSRCand(const T& obj) const;

  typedef const edm::Ptr<reco::Candidate> (FType) (const reco::Candidate* const);
  std::vector<std::function<FType> > fsrGetters;
};


AlternateDaughterInfoEmbedder::AlternateDaughterInfoEmbedder(const edm::ParameterSet& iConfig) :
  srcToken(consumes<CCandView>(iConfig.getParameter<edm::InputTag>("src"))),
  names(iConfig.getParameter<std::vector<std::string> >("names")),
  pairNames(makePairNames(names)),
  nDaughters(names.size()),
  fsrLabel(iConfig.exists("fsrLabel") ?
      iConfig.getParameter<std::string>("fsrLabel") :
      "")
{
  if(nDaughters < 3)
    throw cms::Exception("InvalidChannel") << "AlternateDaughterEmbedder "
      << "can't do anything with fewer than 3 particles in the final state!";

  for(auto& name : names)
  {
    if (fsrLabel.empty())
      fsrGetters.push_back(std::function<FType>([](const reco::Candidate* const obj){return edm::Ptr<reco::Candidate>(NULL, 0);}));
    else if (name.find('e') != std::string::npos)
    {
      fsrGetters.push_back(std::function<FType>([this](const reco::Candidate* const obj)
        {
          const pat::Electron* e = dynamic_cast<const pat::Electron*>(obj->masterClone().get());
          if (e)
            return getFSRCand(*e);
          return edm::Ptr<reco::Candidate>(NULL, 0);
        }));
    }
    else
    {
      // this better be a muon...
      if (name.find('m') == std::string::npos)
        throw cms::Exception("BadInput")
          << "Alternate DaughterInfoEmbedder currently only supports e and mu"
          << std::endl;

      fsrGetters.push_back(std::function<FType>([this](const reco::Candidate* const obj)
        {
          const pat::Muon* m = dynamic_cast<const pat::Muon*>(obj->masterClone().get());
          if(m)
            return getFSRCand(*m);
          return edm::Ptr<reco::Candidate>(NULL, 0);
        }));
    }
  }

  produces<CCandCollection>();
}


void AlternateDaughterInfoEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<CCandView> in;
  iEvent.getByToken(srcToken, in);

  std::unique_ptr<CCandCollection> out(new CCandCollection);

  std::vector<const reco::Candidate*> daughters(nDaughters);

  for(size_t i = 0; i < in->size(); ++i)
  {
    edm::Ptr<CCand> cand = in->ptrAt(i);

    out->push_back(*cand);

    for(size_t iDau = 0; iDau < nDaughters-1; ++iDau)
      daughters[iDau] = cand->daughter(iDau/2)->daughter(iDau%2);

    if(nDaughters % 2)
    {
      daughters.at(nDaughters-1) = cand->daughter((nDaughters-1)/2);
    }
    else
    {
      daughters.at(nDaughters-1) = cand->daughter(nDaughters/2-1)->daughter(1);
    }

    size_t iName = 0;
    for(size_t d1 = 0; d1 < nDaughters - 1; ++d1)
    {
      edm::Ptr<reco::Candidate> d1FSR = fsrGetters.at(d1)(daughters.at(d1));
      for(size_t d2 = d1 + 1; d2 < nDaughters; ++d2)
      {
        edm::Ptr<reco::Candidate> d2FSR = fsrGetters.at(d2)(daughters.at(d2));

        const std::string& name = pairNames.at(iName);
        iName++;

        auto p4 = daughters.at(d1)->p4() + daughters.at(d2)->p4();

        if(fsrLabel.empty())
        {
          out->back().addUserFloat(name+"Mass", p4.M());
        }
        else
        {
          out->back().addUserFloat(name+"MassNoFSR", p4.M());

          if(d1FSR.isNonnull())
            p4 += d1FSR->p4();
          if(d2FSR.isNonnull())
            p4 += d2FSR->p4();

          out->back().addUserFloat(name+"Mass", p4.M());
        }

        out->back().addUserFloat(name+"DR",  deltaR(daughters.at(d1)->p4(), daughters.at(d2)->p4()));
        out->back().addUserFloat(name+"SS",  (daughters.at(d1)->charge() == daughters.at(d2)->charge() ? 1. : 0.));
      }
    }
  }

  iEvent.put(std::move(out));
}


const std::vector<std::string>
AlternateDaughterInfoEmbedder::makePairNames(const std::vector<std::string>& daughterNames) const
{
  if(daughterNames.size() < 2)
    return daughterNames;

  std::vector<std::string> out;
  for(size_t n1 = 0; n1 < daughterNames.size()-1; ++n1)
  {
    for(size_t n2 = n1+1; n2 < daughterNames.size(); ++n2)
    {
      if(daughterNames.at(n2) < daughterNames.at(n1))
        out.push_back(daughterNames.at(n2) + "_" + daughterNames.at(n1) + "_");
      else
        out.push_back(daughterNames.at(n1) + "_" + daughterNames.at(n2) + "_");
    }
  }

  return out;
}


template<class T>
const edm::Ptr<reco::Candidate>
AlternateDaughterInfoEmbedder::getFSRCand(const T& obj) const
{
  if (obj.hasUserCand(fsrLabel))
    return obj.userCand(fsrLabel);
  return edm::Ptr<reco::Candidate>(NULL, 0);
}


DEFINE_FWK_MODULE(AlternateDaughterInfoEmbedder);
