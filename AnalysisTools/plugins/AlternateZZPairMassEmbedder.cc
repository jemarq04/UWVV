///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    AlternateZZPairMassEmbedder                                            //
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
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/View.h"

using pat::CompositeCandidate, pat::CompositeCandidateCollection;
typedef edm::View<CompositeCandidate> CompositeCandidateView;

class AlternateZZPairMassEmbedder : public edm::stream::EDProducer<> {
public:
  explicit AlternateZZPairMassEmbedder(const edm::ParameterSet& iConfig);
  virtual ~AlternateZZPairMassEmbedder() {};

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  const edm::EDGetTokenT<CompositeCandidateView> candToken_;

  std::vector<std::string> names_;
};

AlternateZZPairMassEmbedder::AlternateZZPairMassEmbedder(const edm::ParameterSet& iConfig)
    : candToken_(consumes<CompositeCandidateView>(iConfig.getParameter<edm::InputTag>("src"))),
      names_(iConfig.getParameter<std::vector<std::string>>("names")) {
  produces<CompositeCandidateCollection>();
}

void AlternateZZPairMassEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<CompositeCandidateView> candsIn;
  iEvent.getByToken(candToken_, candsIn);

  std::unique_ptr<CompositeCandidateCollection> candsOut(new CompositeCandidateCollection());

  for (auto it = candsIn->begin(); it != candsIn->end(); it++) {
    candsOut->push_back(*it);
    CompositeCandidate& cand = candsOut->back();

    float alt_masses[2] = {0};
    if (cand.userFloat(names_[0] + "_" + names_[2] + "_SS") < 0.5) {
      alt_masses[0] = cand.userFloat(names_[0] + "_" + names_[2] + "_Mass");
      alt_masses[1] = cand.userFloat(names_[1] + "_" + names_[3] + "_Mass");
    } else if (cand.userFloat(names_[0] + "_" + names_[3] + "_SS") < 0.5) {
      alt_masses[0] = cand.userFloat(names_[0] + "_" + names_[3] + "_Mass");
      alt_masses[1] = cand.userFloat(names_[1] + "_" + names_[2] + "_Mass");
    } else
      throw cms::Exception("Configuration") << "Unexpected ordering of Z candidate leptons";

    float ZaMass, ZbMass;
    if (std::fabs(alt_masses[0] - 91.1876) < std::fabs(alt_masses[1] - 91.1876)) {
      ZaMass = alt_masses[0];
      ZbMass = alt_masses[1];
    } else {
      ZaMass = alt_masses[1];
      ZbMass = alt_masses[0];
    }

    cand.addUserFloat("ZaMass", ZaMass);
    cand.addUserFloat("ZbMass", ZbMass);
  }

  iEvent.put(std::move(candsOut));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(AlternateZZPairMassEmbedder);
