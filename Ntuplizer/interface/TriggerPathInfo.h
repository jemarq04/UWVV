#ifndef UWVV_Ntuplizer_TriggerPathInfo_h
#define UWVV_Ntuplizer_TriggerPathInfo_h

#include <string>

#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/PackedTriggerPrescales.h"


namespace uwvv
{

  class TriggerPathInfo
  {
   public:
    TriggerPathInfo(const std::string& nameExp, bool ignoreMissing);
    ~TriggerPathInfo() {;}

    // Set bit, name
    void setup(const edm::TriggerNames& names);

    bool pass(const edm::TriggerResults& results) const;

    double
    prescale(const pat::PackedTriggerPrescales& prescales) const;

    std::string& name() {return name_;}

   private:
    // for attempted uninitialized use
    void explode() const
    {
      throw cms::Exception("UninitializedTriggerInfo")
        << "ERROR: attempt to use trigger info before it's set up." 
        << std::endl;
    }

    // reg exp for trigger name
    const std::string nameExp_;
    // name and bit of the actual trigger (may change)
    std::string name_;
    size_t bit_;

    // have we set up the name and bit?
    bool isValid_;
    // Set a path to false if it is missing from the dataset
    bool ignoreMissing_;
  };

} // namespace


#endif // header guard
