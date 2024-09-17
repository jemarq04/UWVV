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
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/Ref.h"


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
  typedef edm::Association<reco::GenJetCollection> MatchMap;
  edm::EDGetTokenT<JetView> srcToken;
  edm::EDGetTokenT<MatchMap> matchToken_;
  bool domatch_;
  const int  setup_;
  int PUid;
  int evtcount = 0;
  int jetcount = 0;
  
};


PATJetIDEmbedder::PATJetIDEmbedder(const edm::ParameterSet& pset) :
  srcToken(consumes<JetView>(pset.getParameter<edm::InputTag>("src"))),
  matchToken_(consumes<MatchMap>(edm::InputTag("patJetGenJetMatch"))),
  domatch_(pset.exists("domatch") ? pset.getParameter<bool>("domatch") : false),
  //Which year JET ID we need
  setup_(pset.exists("setup") ? pset.getParameter<int>("setup") : 2022)
{
  produces<VJet>();
}


void PATJetIDEmbedder::produce(edm::Event& iEvent,
                               const edm::EventSetup& iSetup)
{
  //evtcount++;
  //printf("====================RECO vs Gen jet Information=========================================\n");
  //printf("evt#   pt     eta    phi    pt     eta    phi    PUid0 PUidnew jet#\n");
  //jetcount = 0;
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<MatchMap> match;
  if (domatch_){
  iEvent.getByToken(matchToken_, match);}

  std::unique_ptr<VJet> out(new VJet());

  for(size_t i = 0; i < in->size(); ++i)
    {
      out->push_back(in->at(i)); // copies, transfers ownership

      Jet& jet = out->back();

      jet.addUserFloat("idTight", float(passTight(jet)));
      jet.addUserFloat("idPU", float(passPUID(jet)));

      if (domatch_){
        //jetcount++;
        edm::Ref<JetView> jetRef(in, i);
        const auto genMatched = (*match)[jetRef];
        PUid = jetRef->userInt("pileupJetIdUpdated:fullId");
        if (genMatched.isNonnull()){
          //printf("%3d %7.2f %6.2f %6.2f %7.2f %6.2f %6.2f %5d %5d %7d\n", 
         //evtcount, jetRef->pt(), jetRef->eta(), jetRef->phi(), genMatched->pt(), genMatched->eta(), genMatched->phi(),jetRef->userInt("pileupJetId:fullId"), PUid, jetcount);
          jet.addUserFloat("genjetMatched", 1.);
        }
        else{
          //printf("%3d %7.2f %6.2f %6.2f %7.2f %6.2f %6.2f %5d %5d %7d\n", 
         //evtcount, jetRef->pt(), jetRef->eta(), jetRef->phi(), -1.,-1.,-1.,jetRef->userInt("pileupJetId:fullId"), PUid, jetcount);
          jet.addUserFloat("genjetMatched", 0.);
        }
      }
    }

  iEvent.put(std::move(out));
}

bool PATJetIDEmbedder::passTight(const Jet& jet) const
{
  float NHF  = jet.neutralHadronEnergyFraction();
  float NEMF = jet.neutralEmEnergyFraction();
  float CHF  = jet.chargedHadronEnergyFraction();
  //float CEMF = jet.chargedEmEnergyFraction();
  int NumConst = jet.chargedMultiplicity()+jet.neutralMultiplicity();
  int NumNeutralParticles = jet.neutralMultiplicity();
  float CHM  = jet.chargedMultiplicity();

  float absEta = std::abs(jet.eta());

  bool JetID = false;

  if (setup_ == 2022){
    //https://twiki.cern.ch/twiki/bin/view/CMS/JetID13p6TeV#Recommendations_for_the_13_6_AN1 (assuming AK4CHS)
    JetID = (absEta <= 2.6 && NHF < 0.99 && NEMF < 0.90 && NumConst > 1 && CHF > 0.01 && CHM > 0) ||
            (absEta > 2.6 && absEta <= 2.7 && NHF < 0.9 && NEMF < 0.99 && CHM > 0) ||
            (absEta > 2.7 && absEta <= 3.0 && NHF < 0.99 && NEMF < 0.99 && NumNeutralParticles > 1) ||
            (absEta > 3.0 && NEMF < 0.4 && NumNeutralParticles > 10);
  }
  else
    throw cms::Exception("JetID") << "Jet ID is not defined for the given setup (" << setup_ << ")!";
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
