#ifndef UWVV_Utilities_helpers_h
#define UWVV_Utilities_helpers_h


#include <string>

#include "TLorentzVector.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

namespace uwvv
{

  namespace helpers
  {
    // Get an object's four-momentum with FSR included (if any).
    // Note: p4WithoutFSR<pat::CompositeCandidate> is defined in helpers.cc
    template<class T>
    math::XYZTLorentzVector p4WithoutFSR(const T& cand)
    {
      // if it's not a composite, FSR isn't included by definition
      if(cand.numberOfDaughters() == 0)
        return cand.p4();

      const pat::CompositeCandidate& ccand = dynamic_cast<const pat::CompositeCandidate&>(cand);
      //return p4WithoutFSR(ccand);
      math::XYZTLorentzVector out = p4WithoutFSR(*(ccand.daughter(0)->masterClone().get()));
      if(ccand.numberOfDaughters() >= 2)
        out += p4WithoutFSR(*(ccand.daughter(1)->masterClone().get()));

      return out;
    }

    template<class T>
    math::XYZTLorentzVector p4WithoutFSR(const edm::Ptr<T>& cand)
    {
      return p4WithoutFSR(*cand);
    }


    float zMassDistance(const float m);

    float zMassDistance(const TLorentzVector& v);

    float zMassDistance(const math::XYZTLorentzVector& v);

    // Return true if the first daughter is
    // farther from the nominal Z mass than the second daughter
    bool zsNeedReorder(const edm::Ptr<pat::CompositeCandidate>& cand);

    // Check if any final daughters of mother are within dR of cand.
    // "final daughters" means it checks the daughters of daughters if
    // applicable
    bool overlapWithAnyDaughter(const reco::Candidate& cand,
                                const reco::Candidate& mother, float dR);
    
    // Return the jet collection cleaned from the initial state objects. 
    // Collection should be embedded into the initial state as userData.
    const edm::PtrVector<pat::Jet>* getCleanedJetCollection(const pat::CompositeCandidate& cand, 
        const std::string& variation, std::string collectionName="cleanedJets");
  } // namespace helpers

} // namespace uwvv



#endif // header guard
