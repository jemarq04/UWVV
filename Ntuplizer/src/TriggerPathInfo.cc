#include "UWVV/Ntuplizer/interface/TriggerPathInfo.h"

#include <regex>

using namespace uwvv;


TriggerPathInfo::TriggerPathInfo(const std::string& nameExp, bool ignoreMissing) : 
  nameExp_(nameExp),
  name_(nameExp),
  bit_(999),
  isValid_(false),
  ignoreMissing_(ignoreMissing)
{;}


void TriggerPathInfo::setup(const edm::TriggerNames& names)
{
  isValid_ = false;
  std::regex re(nameExp_);
  bit_ = names.size();
  for(size_t i = 0; i < names.size(); ++i)
    {
      if(std::regex_match(names.triggerName(i), re))
        {
          // There can be only one
          if(bit_ != names.size())
            throw cms::Exception("BadTriggerExpression")
              << "Two path matched trigger path expression \""
              << nameExp_ << "\" (\"" << names.triggerName(bit_)
              << "\" and \"" << names.triggerName(i) << "\")" 
              << std::endl;
          bit_ = i;
          name_ = names.triggerName(i);
        }
    }

  // No match!
  if(bit_ == names.size())
    {
      if (!ignoreMissing_) 
        {
          throw cms::Exception("BadTriggerExpression")
          << "No path matched trigger path expression \""
          << nameExp_ << "\"."
          << std::endl;
        }
    }
  else
      isValid_ = true;
}


bool TriggerPathInfo::pass(const edm::TriggerResults& results) const
{
  if(!isValid_)
    {
      if (ignoreMissing_)
          return false;
      explode();
    }

  return results.accept(bit_);
}


double
TriggerPathInfo::prescale(const pat::PackedTriggerPrescales& prescales) const
{
  if(!isValid_)
    {
      if (ignoreMissing_)
          return 1;
      explode();
    }

  return prescales.getPrescaleForIndex<double>(bit_);
}

