//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    NEventFilter.cc                                                       //
//                                                                          //
//    Author: Justin Marquez, U. Wisconsin                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <string>
#include <vector>
#include <fstream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"

class NEventFilter : public edm::stream::EDFilter<> {
public:
  explicit NEventFilter(const edm::ParameterSet& iConfig);
  virtual ~NEventFilter() { ; }

private:
  bool filter(edm::Event& iEvent, const edm::EventSetup& iSetup);

  int numGroups_, group_;
  int numEvents_ = 0;
};

NEventFilter::NEventFilter(const edm::ParameterSet& iConfig)
    : numGroups_(iConfig.getParameter<int>("numGroups")), group_(iConfig.getParameter<int>("group")) {
  if (group_ < 0 || group_ >= numGroups_)
    throw cms::Exception("Invalid group number - must be zero-indexed");
}

bool NEventFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  numEvents_++;

  return numEvents_ % numGroups_ == group_;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(NEventFilter);
