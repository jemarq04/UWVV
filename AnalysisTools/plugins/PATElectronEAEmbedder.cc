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
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "CommonTools/Egamma/interface/EffectiveAreas.h"


class PATElectronEAEmbedder : public edm::stream::EDProducer<>
{
public:
  explicit PATElectronEAEmbedder(const edm::ParameterSet&);
  virtual ~PATElectronEAEmbedder() {}

private:
  // Methods
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);
  float getEA(const edm::Ptr<pat::Electron>& elec) const;

  // Data
  edm::EDGetTokenT<edm::View<pat::Electron> > electronCollectionToken_;
  const std::string label_; // label for the embedded userfloat
  const std::string filename_; //filename for effective area
  EffectiveAreas effectiveAreas_;
};


// Constructors and destructors

PATElectronEAEmbedder::PATElectronEAEmbedder(const edm::ParameterSet& iConfig):
  electronCollectionToken_(consumes<edm::View<pat::Electron> >(iConfig.exists("src") ? 
                                                               iConfig.getParameter<edm::InputTag>("src") :
                                                               edm::InputTag("slimmedElectrons"))),
  label_(iConfig.exists("label") ?
         iConfig.getParameter<std::string>("label") :
         std::string("EffectiveArea")),
  effectiveAreas_((iConfig.getParameter<edm::FileInPath>("configFile")).fullPath())
{
  produces<std::vector<pat::Electron> >();
}


void PATElectronEAEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::unique_ptr<std::vector<pat::Electron> > out = 
    std::unique_ptr<std::vector<pat::Electron> >(new std::vector<pat::Electron>);

  edm::Handle<edm::View<pat::Electron> > electronsIn;

  iEvent.getByToken(electronCollectionToken_, electronsIn);

  for(edm::View<pat::Electron>::const_iterator ei = electronsIn->begin();
      ei != electronsIn->end(); ei++) // loop over electrons
    {
      const edm::Ptr<pat::Electron> eptr(electronsIn, ei - electronsIn->begin());

      out->push_back(*ei); // copy electron to save correctly in event

      float ea;
      ea = getEA(eptr);
      
      out->back().addUserFloat(label_, ea);
    }

  iEvent.put(std::move(out));
}

float PATElectronEAEmbedder::getEA(const edm::Ptr<pat::Electron>& elec) const
{
  float abseta = fabs(elec->eta());
  return effectiveAreas_.getEffectiveArea(abseta);
}

DEFINE_FWK_MODULE(PATElectronEAEmbedder);
