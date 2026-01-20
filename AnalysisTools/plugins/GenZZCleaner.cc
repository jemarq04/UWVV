///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    GenZZCleaner                                                           //
//                                                                           //
//    Remove 4-object gen candidates which fail OSSF mass cuts or are the    //
//    wrong lepton combination.                                              //
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
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/View.h"

typedef reco::Candidate Cand;
typedef pat::CompositeCandidate CCand;
typedef edm::Ptr<CCand> CCandPtr;

class GenZZCleaner : public edm::stream::EDProducer<> {
public:
  explicit GenZZCleaner(const edm::ParameterSet& iConfig);
  virtual ~GenZZCleaner() {};

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  bool passOSSFCuts(const Cand* p1, const Cand* p2) const;

  const edm::EDGetTokenT<edm::View<CCand> > srcToken;
  const double l1PtCut;
  const double l2PtCut;
  const double l3PtCut;
  const double l4PtCut;
  const double etaCut;
  const double ossfMassCut;
  const double z1MassMin;
  const double z1MassMax;
  const double z2MassMin;
  const double z2MassMax;
};

GenZZCleaner::GenZZCleaner(const edm::ParameterSet& iConfig)
    : srcToken(consumes<edm::View<CCand> >(iConfig.getParameter<edm::InputTag>("src"))),
      l1PtCut(iConfig.exists("l1PtCut") ? iConfig.getParameter<double>("l1PtCut") : 20.),
      l2PtCut(iConfig.exists("l2PtCut") ? iConfig.getParameter<double>("l2PtCut") : 10.),
      l3PtCut(iConfig.exists("l3PtCut") ? iConfig.getParameter<double>("l3PtCut") : 5.),
      l4PtCut(iConfig.exists("l4PtCut") ? iConfig.getParameter<double>("l4PtCut") : 5.),
      etaCut(iConfig.exists("etaCut") ? iConfig.getParameter<double>("etaCut") : 2.5),
      ossfMassCut(iConfig.exists("ossfMassCut") ? iConfig.getParameter<double>("ossfMassCut") : 4.),
      z1MassMin(iConfig.exists("z1MassMin") ? iConfig.getParameter<double>("z1MassMin") : 40.),
      z1MassMax(iConfig.exists("z1MassMax") ? iConfig.getParameter<double>("z1MassMax") : 120.),
      z2MassMin(iConfig.exists("z2MassMin") ? iConfig.getParameter<double>("z2MassMin") : 4.),
      z2MassMax(iConfig.exists("z2MassMax") ? iConfig.getParameter<double>("z2MassMax") : 120.) {
  produces<std::vector<CCand> >();
}

void GenZZCleaner::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<edm::View<CCand> > in;
  iEvent.getByToken(srcToken, in);

  std::unique_ptr<std::vector<CCand> > out(new std::vector<CCand>);

  size_t bestCand = 9999;
  float bestDZ = 9999.;

  for (size_t i = 0; i < in->size(); ++i) {
    CCandPtr c = in->ptrAt(i);

    float mZ1 = c->daughter(0)->mass();
    float mZ2 = c->daughter(1)->mass();

    float dz1 = std::abs(mZ1 - 91.1876);
    float dz2 = std::abs(mZ2 - 91.1876);
    float betterDZ = (dz1 < dz2 ? dz1 : dz2);

    if (dz2 < dz1) {
      float temp = mZ1;
      mZ1 = mZ2;
      mZ2 = temp;
    }

    bool best = betterDZ < bestDZ;
    if (!best)
      continue;

    bestDZ = betterDZ;
    bestCand = 9999;  // don't use previous best even if this one fails

    if (mZ1 < z1MassMin || mZ1 > z1MassMax)
      continue;
    if (mZ2 < z2MassMin || mZ2 > z2MassMax)
      continue;

    std::vector<const Cand*> daughters;
    daughters.push_back(c->daughter(0)->daughter(0));
    daughters.push_back(c->daughter(0)->daughter(1));
    daughters.push_back(c->daughter(1)->daughter(0));
    daughters.push_back(c->daughter(1)->daughter(1));

    bool passl1Pt = false;
    bool passEta = true;
    size_t nPassl2Pt = 0;
    size_t nPassl3Pt = 0;
    bool passl4Pt = true;
    for (size_t d = 0; d < daughters.size(); ++d) {
      float pt = daughters.at(d)->pt();
      passl1Pt |= pt > l1PtCut;
      if (pt > l2PtCut) {
        nPassl3Pt++;
        nPassl2Pt++;
      } else if (pt > l3PtCut)
        nPassl3Pt++;

      passEta &= std::abs(daughters.at(d)->eta()) < etaCut;
      passl4Pt &= pt > l4PtCut;
    }

    best &= (passl1Pt && passl4Pt && passEta && nPassl2Pt >= 2 && nPassl3Pt >= 3);

    if (!best)
      continue;

    best &= (passOSSFCuts(daughters.at(0), daughters.at(2)) && passOSSFCuts(daughters.at(0), daughters.at(3)) &&
             passOSSFCuts(daughters.at(1), daughters.at(2)) && passOSSFCuts(daughters.at(1), daughters.at(3)));

    if (!best)
      continue;

    bestCand = i;
  }

  if (bestCand < in->size())
    out->push_back(in->at(bestCand));

  iEvent.put(std::move(out));
}

bool GenZZCleaner::passOSSFCuts(const Cand* p1, const Cand* p2) const {
  return p1->pdgId() != -1 * p2->pdgId() || (p1->p4() + p2->p4()).mass() > ossfMassCut;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GenZZCleaner);
