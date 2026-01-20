///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    Plugin to combine PAT objects into pat::CompositeCandidates made of    //
//    reco::ShallowCloneCandidates of the originals                          //
//                                                                           //
//    Nate Woods, U. Wisconsin                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "FWCore/Framework/interface/MakerMacros.h"
#include "CommonTools/CandAlgos/interface/CandCombiner.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "CommonTools/UtilAlgos/interface/StringCutObjectSelector.h"

namespace reco {
  namespace modules {

    typedef CandCombiner<StringCutObjectSelector<reco::Candidate, true>,
                         AnyPairSelector,
                         combiner::helpers::ShallowClone,
                         pat::CompositeCandidateCollection>
        PATCandViewShallowCloneCombiner;

    DEFINE_FWK_MODULE(PATCandViewShallowCloneCombiner);

  }  // namespace modules
}  // namespace reco
