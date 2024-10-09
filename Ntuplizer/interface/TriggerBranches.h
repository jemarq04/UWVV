#ifndef UWVV_Ntuplizer_TriggerBranches_h
#define UWVV_Ntuplizer_TriggerBranches_h

// STL
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

// CMSSW
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"

// ROOT
#include "TTree.h"

// UWVV
#include "UWVV/Ntuplizer/interface/TriggerPathInfo.h"


namespace uwvv
{

  // Single Branch
  class TriggerBranch
  {
  public:
    TriggerBranch(const std::string& name, 
                  const std::vector<std::string>& pathExps,
                  TTree* const tree,
                  bool checkPrescale,
                  bool ignoreMissing);
    ~TriggerBranch(){;}

    void setup(const edm::TriggerNames& names);

    void fill(const edm::Handle<edm::TriggerResults>& results,
              const edm::Handle<pat::PackedTriggerPrescales>& prescales);

  private:
    const std::string name;
    const bool checkPrescale;
    const bool ignoreMissing;

    bool pass;
    unsigned prescale;

    std::vector<TriggerPathInfo> paths;
  };


  // Collection of branches
  class TriggerBranches
  {
   public:
    TriggerBranches(edm::ConsumesCollector cc, 
                    const edm::ParameterSet& config,
                    TTree* const tree);
    ~TriggerBranches() {;}

    void setEvent(const edm::Event& event);

    void fill();    

   private:
    const edm::EDGetTokenT<edm::TriggerResults> resultsToken;
    edm::Handle<edm::TriggerResults> results;
    const edm::EDGetTokenT<pat::PackedTriggerPrescales> prescalesToken;
    edm::Handle<pat::PackedTriggerPrescales> prescales;

    edm::ParameterSetID id;

    std::unordered_map<std::string, std::unique_ptr<TriggerBranch> > branches;

    bool isValid;

    const bool checkPrescale;
  };

} // namespace

#endif // header guard
