
// STL
#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

// CMSSW
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Candidate/interface/ShallowCloneCandidate.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"

#include "UWVV/DataFormats/interface/DressedGenParticle.h"



template<class T>
class PATObjectPrinter : public edm::one::EDAnalyzer<>
{
public:
  explicit PATObjectPrinter(const edm::ParameterSet& config);
  ~PATObjectPrinter() {;}

private:
  virtual void analyze(edm::Event const& evt,
                       edm::EventSetup const& setup);

  void printCandInfo(const T& cand) const;

  const edm::EDGetTokenT<edm::View<T> > candToken;

  std::string getObjectName() const {return "Candidate";}

  void anythingElse(const T& cand) const {}
};


template<class T>
PATObjectPrinter<T>::PATObjectPrinter(const edm::ParameterSet& config) :
  candToken(consumes<edm::View<T> >(config.getParameter<edm::InputTag>("src")))
{
}


template<class T>
void PATObjectPrinter<T>::analyze(edm::Event const& event, edm::EventSetup const& setup)
{
  edm::Handle<edm::View<T> > candsIn;
  event.getByToken(candToken, candsIn);

  if(candsIn->size())
    std::cout << event.id().run() << ":" << event.id().luminosityBlock() <<":"
              << event.id().event() << ":"
              << "(" << candsIn->size() << " " << getObjectName() << "s)" << std::endl;
  else
    std::cout << "    ... but the collection is empty." << std::endl;

  for(size_t i = 0; i < candsIn->size(); ++i)
    printCandInfo(candsIn->at(i));

  std::cout << std::endl;
}


template<class T>
void PATObjectPrinter<T>::printCandInfo(const T& cand) const
{
  std::cout << "id: " << cand.pdgId() << " m: " << cand.mass()
            << " pt: " << cand.pt()
            << " eta: " << cand.eta() << " phi: " << cand.phi();
  anythingElse(cand);

  std::cout << std::endl;
}


template<>
std::string PATObjectPrinter<pat::Jet>::getObjectName() const
{
  return "jet";
}


template<>
std::string PATObjectPrinter<pat::Electron>::getObjectName() const
{
  return "electron";
}


template<>
std::string PATObjectPrinter<pat::Muon>::getObjectName() const
{
  return "muon";
}


template<>
void PATObjectPrinter<pat::Electron>::anythingElse(const pat::Electron& cand) const
{
  float dxyz = std::abs(cand.dB(pat::Electron::PV3D));
  std::cout << " SIP3D: " << dxyz / cand.edB(pat::Electron::PV3D);
  float dxy = std::abs(cand.dB(pat::Electron::PV2D));
  std::cout << " PVDXY: " << dxy;
  std::cout << " PVDZ: " << std::sqrt(dxyz*dxyz - dxy*dxy);
  std::cout << " SCEta: " << cand.superCluster()->eta();
}


template<>
void PATObjectPrinter<pat::Jet>::anythingElse(const pat::Jet& cand) const
{
  if(cand.hasUserFloat("idLoose"))
    std::cout << " Loose ID: " << cand.userFloat("idLoose");

  if(cand.hasUserInt("pileupJetIdUpdated:fullId"))
    std::cout << " PUID: " << cand.userInt("pileupJetIdUpdated:fullId");
}


template<>
void PATObjectPrinter<DressedGenParticle>::anythingElse(const DressedGenParticle& cand) const
{
  std::cout << " nPho: " << cand.numAssociated() << " undressedPt: " << cand.undressedPt();
}


template<>
void PATObjectPrinter<pat::CompositeCandidate>::anythingElse(const pat::CompositeCandidate& cand) const
{
  std::cout << " nDau: " << cand.numberOfDaughters();

  for(size_t i = 0; i < cand.numberOfDaughters(); ++i)
    {
      std::cout << " dau" << i << " ID: " << cand.daughter(i)->pdgId()
                << " Pt: " << cand.daughter(i)->pt();
    }
}



typedef PATObjectPrinter<pat::Jet> JetPrinter;
typedef PATObjectPrinter<pat::Electron> ElectronPrinter;
typedef PATObjectPrinter<pat::Muon> MuonPrinter;
typedef PATObjectPrinter<DressedGenParticle> DressedGenParticlePrinter;
typedef PATObjectPrinter<pat::CompositeCandidate> CompositeCandidatePrinter;


DEFINE_FWK_MODULE(JetPrinter);
DEFINE_FWK_MODULE(ElectronPrinter);
DEFINE_FWK_MODULE(MuonPrinter);
DEFINE_FWK_MODULE(DressedGenParticlePrinter);
DEFINE_FWK_MODULE(CompositeCandidatePrinter);
