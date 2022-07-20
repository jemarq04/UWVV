//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetEnergyScaleShifter.cc                                           //
//                                                                          //
//    Copies a jet collection to two new collections with the energy scale  //
//    shifted up and down by 1sigma.                                        //
//                                                                          //
//    Author: Nate Woods, U. Wisconsin                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include<memory>
#include<string>
#include<vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"
#include "FWCore/Framework/interface/ESHandle.h"

typedef pat::Jet Jet;
typedef std::vector<Jet> VJet;
typedef edm::View<Jet> JetView;


class PATJetEnergyScaleShifter : public edm::stream::EDProducer<>
{
 public:
  explicit PATJetEnergyScaleShifter(const edm::ParameterSet& pset);
  virtual ~PATJetEnergyScaleShifter() {;}

 private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  edm::EDGetTokenT<JetView> srcToken;
};


PATJetEnergyScaleShifter::PATJetEnergyScaleShifter(const edm::ParameterSet& pset) :
  srcToken(consumes<JetView>(pset.getParameter<edm::InputTag>("src")))
{
  produces<VJet>("jesUp");
  produces<VJet>("jesDown");
}


void PATJetEnergyScaleShifter::produce(edm::Event& iEvent,
                                       const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);

  edm::ESHandle<JetCorrectorParametersCollection> jecParams;
  iSetup.get<JetCorrectionsRecord>().get("AK4PFchs", jecParams);
  const JetCorrectorParameters & param = (*jecParams)["Uncertainty"];
  JetCorrectionUncertainty jecUnc(param);

  std::unique_ptr<VJet> outUp(new VJet());
  std::unique_ptr<VJet> outDn(new VJet());

  for(size_t i = 0; i < in->size(); ++i)
    {
      const Jet& jet = in->at(i);
      outUp->push_back(jet); // copies, transfers ownership
      outDn->push_back(jet);

      jecUnc.setJetEta(jet.eta());
      jecUnc.setJetPt(jet.pt());
      float unc = jecUnc.getUncertainty(true);

      outUp->back().setP4(math::PtEtaPhiMLorentzVector(jet.pt()*(1.+unc), jet.eta(),
                                                       jet.phi(), jet.mass()));
      outDn->back().setP4(math::PtEtaPhiMLorentzVector(jet.pt()*(1.-unc), jet.eta(),
                                                       jet.phi(), jet.mass()));
    }

  iEvent.put(std::move(outUp), "jesUp");
  iEvent.put(std::move(outDn), "jesDown");
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetEnergyScaleShifter);
