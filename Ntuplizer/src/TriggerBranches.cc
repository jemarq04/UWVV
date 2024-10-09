#include "UWVV/Ntuplizer/interface/TriggerBranches.h"



using namespace uwvv;

// Single branch implementation

TriggerBranch::TriggerBranch(const std::string& name, 
                             const std::vector<std::string>& pathExps,
                             TTree* const tree,
                             bool checkPrescale,
                             bool ignoreMissing) :
  name(name),
  checkPrescale(checkPrescale),
  ignoreMissing(ignoreMissing)
{
  if(!pathExps.size())
    throw cms::Exception("BadTriggerPath")
      << "Trigger branch must contain at least one path." << std::endl;
  
  for(auto& expr : pathExps)
    paths.push_back(TriggerPathInfo(expr, ignoreMissing));
  
  tree->Branch((name+"Pass").c_str(), &pass);
  if(checkPrescale)
    tree->Branch((name+"Prescale").c_str(), &prescale);
}


void TriggerBranch::setup(const edm::TriggerNames& names)
{
  for(auto& path : paths)
    path.setup(names);
}


void TriggerBranch::fill(const edm::Handle<edm::TriggerResults>& results,
                         const edm::Handle<pat::PackedTriggerPrescales>& prescales)
{
  if(checkPrescale)
    {
      // don't need to check for validity (paths do that)
      prescale = paths.at(0).prescale(*prescales);
      pass = false;
      for(auto& path : paths)
        {
          if(path.prescale(*prescales) != prescale)
            {
              if(checkPrescale)
                throw cms::Exception("InvalidPrescale")
                  << "ERROR: trying to take OR of paths with different prescales "
                  << "(" << paths.at(0).name() << " and " << path.name() << ")"
                  << std::endl;
            }

          pass |= path.pass(*results);
        }
    }
  else
    {
      pass = false;
      for(auto& path : paths)
        {
          pass = path.pass(*results);
          if(pass) break;
        }
    }
}


// Branch collection implementation

TriggerBranches::TriggerBranches(edm::ConsumesCollector cc, 
                                 const edm::ParameterSet& config,
                                 TTree* const tree) :
  resultsToken(cc.consumes<edm::TriggerResults>(config.getParameter<edm::InputTag>("trigResultsSrc"))),
  prescalesToken(cc.consumes<pat::PackedTriggerPrescales>(config.exists("trigPrescaleSrc") ?
                                                          config.getParameter<edm::InputTag>("trigPrescaleSrc") :
                                                          edm::InputTag("patTrigger"))),
  isValid(false),
  checkPrescale(config.exists("checkPrescale") ?
                config.getParameter<bool>("checkPrescale") :
                true)
{
  std::vector<std::string> names = 
    config.getParameter<std::vector<std::string> >("trigNames");
  bool ignoreMissing(config.getUntrackedParameter<bool>("ignoreMissing", false));
  
  for(auto& name : names)
    {
      std::vector<std::string> paths = 
        config.getParameter<std::vector<std::string> >(name + "Paths");
      
      branches[name] = std::unique_ptr<TriggerBranch>(new TriggerBranch(name, paths, tree, checkPrescale, ignoreMissing));
    }
}


void TriggerBranches::setEvent(const edm::Event& event)
{
  event.getByToken(resultsToken, results);
  if(checkPrescale)
    event.getByToken(prescalesToken, prescales);

  const edm::TriggerNames& names = event.triggerNames(*results);

  if((!isValid) || id != names.parameterSetID())
    {
      id = names.parameterSetID();

      for(auto& b : branches)
        b.second->setup(names);

      isValid = true;
    }
}


void TriggerBranches::fill()
{
  if(!isValid)
    throw cms::Exception("UninitializedTriggerBranches")
      << "ERROR: attempt to use uninitialized TriggerBranches object."
      << std::endl;

  for(auto& b : branches)
    b.second->fill(results, prescales);
}
