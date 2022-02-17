//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetIDEmbedder.cc                                                   //
//                                                                          //
//    Embed basic PF Jet IDs as userFloats                                  //
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


typedef pat::Jet Jet;
typedef std::vector<Jet> VJet;
typedef edm::View<Jet> JetView;

class PATJetIDEmbedder : public edm::stream::EDProducer<>
{
 public:
  explicit PATJetIDEmbedder(const edm::ParameterSet& pset);
  virtual ~PATJetIDEmbedder() {;}

 private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  bool passTight(const Jet& jet) const;
  bool passPUID(const Jet& jet) const;

  edm::EDGetTokenT<JetView> srcToken;
  const int  setup_;
};


PATJetIDEmbedder::PATJetIDEmbedder(const edm::ParameterSet& pset) :
  srcToken(consumes<JetView>(pset.getParameter<edm::InputTag>("src"))),
  //Which year JET ID we need
  setup_(pset.exists("setup") ?
	   pset.getParameter<int>("setup") :
	   2016)
{
  produces<VJet>();
}


void PATJetIDEmbedder::produce(edm::Event& iEvent,
                               const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);

  std::unique_ptr<VJet> out(new VJet());

  for(size_t i = 0; i < in->size(); ++i)
    {
      out->push_back(in->at(i)); // copies, transfers ownership

      Jet& jet = out->back();

      jet.addUserFloat("idTight", float(passTight(jet)));
      jet.addUserFloat("idPU", float(passPUID(jet)));
    }

  iEvent.put(std::move(out));
}

bool PATJetIDEmbedder::passTight(const Jet& jet) const
{
  float NHF  = jet.neutralHadronEnergyFraction();
  float NEMF = jet.neutralEmEnergyFraction();
  float CHF  = jet.chargedHadronEnergyFraction();
  float CEMF = jet.chargedEmEnergyFraction();
  int NumConst = jet.chargedMultiplicity()+jet.neutralMultiplicity();
  int NumNeutralParticles = jet.neutralMultiplicity();
  float CHM  = jet.chargedMultiplicity();

  float absEta = std::abs(jet.eta());

  bool JetID = false;

  if ( setup_ == 2016 )
  {// Tight jet ID https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVRun2016
    JetID = ((NHF<0.90 && NEMF<0.90 && NumConst>1) && ((absEta<=2.4 && CHF>0 && CHM>0 && CEMF<0.99) || absEta>2.4) && absEta<=2.7) ||
            ( NHF<0.98 && NEMF>0.01 && NumNeutralParticles>2 && absEta>2.7 && absEta<=3.0 ) ||
            ( NEMF<0.90 && NumNeutralParticles>10 && absEta >3.0 );
    //JetID = ((NHF<0.99 && NEMF<0.99 && NumConst>1) && ((absEta<=2.4 && CHF>0 && CHM>0 && CEMF<0.99) || absEta>2.4) && absEta<=2.7) ||
    //        ( NHF<0.98 && NEMF>0.01 && NumNeutralParticles>2 && absEta>2.7 && absEta<=3.0 ) ||
    //        ( NEMF<0.90 && NumNeutralParticles>10 && absEta >3.0 );
  
}
  else if ( setup_ == 2017 )
  {// Tight jet ID https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVRun2017 without JetIDLepVeto
   JetID  = ((NHF<0.90 && NEMF<0.90 && NumConst>1) && ((absEta<=2.4 && CHF>0 && CHM>0) || absEta>2.4) && absEta<=2.7) ||
            ( NEMF<0.99 && NEMF>0.02 && NumNeutralParticles>2 && absEta>2.7 && absEta<=3.0 ) ||
            ( NEMF<0.90 && NHF>0.02 && NumNeutralParticles>10 && absEta>3.0 );
  }
  else if ( setup_ == 2018) 
  {// Tight jet ID https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetID13TeVRun2018 without JetIDLepVeto
    JetID = ( CHM>0 && CHF>0 && NumConst>1 && NEMF<0.9 && NHF < 0.9 && absEta<=2.6) ||
            ( CHM>0 && NEMF<0.99 && NHF < 0.9 && absEta>2.6 && absEta<=2.7) ||
            ( NEMF>0.02 && NEMF<0.99 && NumNeutralParticles>2 && absEta>2.7 && absEta<=3.0 ) ||
            ( NEMF<0.90 && NHF>0.2 && NumNeutralParticles>10 && absEta>3.0 );
  }
  else
  {
    throw cms::Exception("JetID") << "Jet ID is not defined for the given setup (" << setup_ << ")!";
  }
  return JetID;
}
bool PATJetIDEmbedder::passPUID(const Jet& jet) const
{
  if(!jet.hasUserFloat("pileupJetId:fullDiscriminant"))
    return false;

  float mva = jet.userFloat("pileupJetId:fullDiscriminant");

  float absEta = std::abs(jet.eta());

  if(jet.pt() > 20.)
    {
      if(absEta > 3. && mva <= -0.45) return false;
      if(absEta > 2.75 && mva <= 0.55) return false;
      if(absEta > 2.5 && mva <= -0.6) return false;
      if(mva <= -0.63) return false;
    }
  else
    {
      if(absEta > 3. && mva <= -0.95) return false;
      if(absEta > 2.75 && mva <= -0.94) return false;
      if(absEta > 2.5 && mva <= -0.96) return false;
      if(mva <= -0.95) return false;
    }

  return true;
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetIDEmbedder);
