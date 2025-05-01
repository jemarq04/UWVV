//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   PATElectronEAEmbedder.cc                                               //
//                                                                          //
//   Embeds electron effective areas using the EGamma POG recommendation.   //
//                                                                          //
//   Authors: Devin Taylor and Nate Woods, U. Wisconsin                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


// system includes
#include <memory>
#include <vector>
#include <iostream>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Egamma/interface/EffectiveAreas.h"

using pat::Electron, pat::ElectronCollection;
typedef edm::View<Electron> ElectronView;

class PATElectronEAEmbedder : public edm::stream::EDProducer<>
{
public:
  explicit PATElectronEAEmbedder(const edm::ParameterSet&);
  virtual ~PATElectronEAEmbedder() {}

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  edm::EDGetTokenT<ElectronView> srcToken_;
  const std::string label_; // label for the embedded userfloat
  const std::string filename_; //filename for effective area
  EffectiveAreas effectiveAreas_;
};


PATElectronEAEmbedder::PATElectronEAEmbedder(const edm::ParameterSet& iConfig) :
  srcToken_(consumes<ElectronView>(iConfig.exists("src") ? 
      iConfig.getParameter<edm::InputTag>("src") :
      edm::InputTag("slimmedElectrons"))),
  label_(iConfig.exists("label") ?
      iConfig.getParameter<std::string>("label") :
      "EffectiveArea"),
  effectiveAreas_((iConfig.getParameter<edm::FileInPath>("configFile")).fullPath())
{
  produces<ElectronCollection>();
}


void PATElectronEAEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<ElectronView> electronsIn;
  iEvent.getByToken(srcToken_, electronsIn);

  std::unique_ptr<ElectronCollection> out(new ElectronCollection());

  for(ElectronView::const_iterator ei = electronsIn->begin(); ei != electronsIn->end(); ei++)
  {
    out->push_back(*ei); // copy electron to save correctly in event
    out->back().addUserFloat(label_, effectiveAreas_.getEffectiveArea(fabs(ei->eta())));
  }

  iEvent.put(std::move(out));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronEAEmbedder);
