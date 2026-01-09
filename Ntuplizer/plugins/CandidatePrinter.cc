
// STL
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

// CMSSW
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Candidate/interface/ShallowCloneCandidate.h"

// UWVV
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"



class CandidatePrinter : public edm::one::EDAnalyzer<>
{

  typedef pat::CompositeCandidate CCand;

public:
  explicit CandidatePrinter(const edm::ParameterSet& config);
  ~CandidatePrinter() {;}

private:
  virtual void analyze(edm::Event const& evt,
                       edm::EventSetup const& setup);

  void printCandInfo(const reco::Candidate* cand,
                     size_t level) const;
  void printDaughterInfo(const CCand* cand,
                         size_t level) const;

  const edm::EDGetTokenT<edm::View<CCand> > candToken;

};


CandidatePrinter::CandidatePrinter(const edm::ParameterSet& config) :
  candToken(consumes<edm::View<CCand> >(config.getParameter<edm::InputTag>("src")))
{
}


void CandidatePrinter::analyze(edm::Event const& event, edm::EventSetup const& setup)
{
  edm::Handle<edm::View<CCand> > candsIn;
  event.getByToken(candToken, candsIn);

  if(candsIn->size())
    std::cout << event.id().run() << ":" << event.id().luminosityBlock() <<":"
              << event.id().event() << ":"
              << "(" << candsIn->size() << " candidates)" << std::endl;

  for(size_t i = 0; i < candsIn->size(); ++i)
    {
      CCand cand(candsIn->at(i));
      printCandInfo(&cand, 0);
    }

  std::cout << std::endl;
}


void CandidatePrinter::printCandInfo(const reco::Candidate* cand,
                                     size_t level) const
{
  if(!cand)
    {
      std::cout << "    ... but there's nothing here" << std::endl;
      return;
    }

  for(size_t nPads = 0; nPads < level; ++nPads)
    std::cout << "    ";

  std::cout << "id: " << cand->pdgId() << "m: " << cand->mass()
            << " pt: " << cand->pt()
            << " eta: " << cand->eta() << " phi: " << cand->phi()
            << " ((" << cand->numberOfDaughters() << "))" << std::endl;

  if(cand->numberOfDaughters())
    {
      const CCand* castCand = (cand->hasMasterClone() ?
                               dynamic_cast<const CCand*>(cand->masterClone().get()) :
                               dynamic_cast<const CCand*>(cand));
      if(castCand)
        printDaughterInfo(castCand, level+1);
      else
        std::cout << "Didn't get anything from the cast" << std::endl;
    }

}


void CandidatePrinter::printDaughterInfo(const CCand* cand,
                                         size_t level) const
{
  for(size_t i = 0; i < cand->numberOfDaughters(); ++i)
    {
       printCandInfo(cand->daughter(i), level);
    }
}



DEFINE_FWK_MODULE(CandidatePrinter);
