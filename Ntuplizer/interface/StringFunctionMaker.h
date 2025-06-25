#ifndef UWVV_Ntuplizer_StringFunctionMaker_h
#define UWVV_Ntuplizer_StringFunctionMaker_h


#include <functional>
#include <string>

// ROOT
#include "TMath.h"

// CMSSW
#include "DataFormats/Common/interface/Ptr.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"



namespace
{
  template<typename T> T convertFromFloat(float x)
    {
      return T(x);
    }

  template<> int convertFromFloat(float x)
    {
      return TMath::Nint(x);
    }

  template<> unsigned convertFromFloat(float x)
    {
      return TMath::Nint(fabs(x));
    }

  template<> unsigned long long convertFromFloat(float x)
    {
      return lrint(fabs(x));
    }
  template<> std::string convertFromFloat(float x)
    {
      return std::to_string(x);
    }
}


namespace uwvv
{

  class StringFunctionMaker
  {
   public:
    template<typename Return, class Obj, class... OtherArgs>
      static std::function<Return(const edm::Ptr<Obj>, OtherArgs...)>
      makeStringFunction(const std::string& fString)
    {
      StringObjectFunction<Obj, true> calculator(fString);
      std::function<Return(const edm::Ptr<Obj>, OtherArgs...)>
        out([calculator](const edm::Ptr<Obj>& obj, OtherArgs... otherArgs)
            {return ::convertFromFloat<Return>(calculator(*obj));});
      return out;
    }
  };

}

#endif // header guard
