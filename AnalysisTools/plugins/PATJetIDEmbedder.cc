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
    typedef edm::Association<reco::GenJetCollection> MatchMap;
    edm::EDGetTokenT<JetView> srcToken;
    edm::EDGetTokenT<MatchMap> matchToken_;
    bool domatch_;
    const int setup_;
    int evtcount = 0;
    int jetcount = 0;

};


PATJetIDEmbedder::PATJetIDEmbedder(const edm::ParameterSet& pset) :
  srcToken(consumes<JetView>(pset.getParameter<edm::InputTag>("src"))),
  matchToken_(consumes<MatchMap>(edm::InputTag("patJetGenJetMatch"))),
  domatch_(pset.exists("domatch") ? pset.getParameter<bool>("domatch") : false),
  //Which year JET ID we need
  setup_(pset.exists("setup") ? pset.getParameter<int>("setup") : 2016)
{
  produces<VJet>();
}


void PATJetIDEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  //printf("====================RECO vs Gen jet Information=========================================\n");
  //printf("evt#   pt     eta    phi    pt     eta    phi    idPU0 idPUnew jet#\n");
  //evtcount++;
  //jetcount = 0;
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<MatchMap> match;
  if (domatch_)
    iEvent.getByToken(matchToken_, match);

  std::unique_ptr<VJet> out(new VJet());

  for (size_t i = 0; i < in->size(); ++i)
  {
    out->push_back(in->at(i)); // copies, transfers ownership
    Jet& jet = out->back();

    int idPU = jet.userInt("pileupJetIdUpdated:fullId");
    jet.addUserFloat("idTight", float(passTight(jet)));
    jet.addUserFloat("idPU", float(idPU));

    if (domatch_){
      edm::Ref<JetView> jetRef(in, i);
      const auto genMatched = (*match)[jetRef];
      jet.addUserFloat("genjetMatched", genMatched.isNonnull()? 1. : 0.);

      /*
      jetcount++;
      if (genMatched.isNonnull()){
        printf("%3d %7.2f %6.2f %6.2f %7.2f %6.2f %6.2f %5d %5d %7d\n", 
          evtcount, jet.pt(), jet.eta(), jet.phi(), genMatched->pt(), genMatched->eta(), genMatched->phi(),jet.userInt("pileupJetId:fullId"), idPU, jetcount);
      }
      else{
        printf("%3d %7.2f %6.2f %6.2f %7.2f %6.2f %6.2f %5d %5d %7d\n", 
          evtcount, jet.pt(), jet.eta(), jet.phi(), -1.,-1.,-1.,jet.userInt("pileupJetId:fullId"), idPU, jetcount);
        jet.addUserFloat("genjetMatched", 0.);
      }
      */
    }
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
  {
    // Tight jet ID https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVUL#Recommendations_for_the_13_TeV_U
    JetID = (absEta <= 2.4 && NHF < 0.90 && NEMF < 0.90 && NumConst > 1 && CHF > 0 && CEMF > 0) ||
            (absEta > 2.4 && absEta <= 2.7 && NHF < 0.90 && NEMF < 0.99) ||
            (absEta > 2.7 && absEta <= 3.0 && NHF < 0.90 && NEMF > 0 && NEMF < 0.99 && NumNuetralParticles > 1) ||
            (absEta > 3.0 && NHF > 0.2 && NEMF < 0.90 && NumNeutralParticles>10);
  }
  else if ( setup_ == 2017 || setup_ == 2018)
  {
    // Tight jet ID https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVUL#Recommendations_for_the_13_T_AN1
    JetID = (absEta <= 2.4 && NHF < 0.90 && NEMF < 0.90 && NumConst > 1 && CHF > 0 && CEMF > 0) ||
            (absEta > 2.4 && absEta <= 2.7 && NHF < 0.90 && NEMF < 0.99 && CHM > 0) ||
            (absEta > 2.7 && absEta <= 3.0 && NEMF > 0.01 && NEMF < 0.99 && NumNeutralParticles > 1) ||
            (absEta > 3.0 && NHF > 0.2 && NEMF < 0.9 && NumNeutralParticles > 10);
  }
  else throw cms::Exception("JetID") << "Jet ID is not defined for the given setup (" << setup_ << ")!";

  return JetID;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetIDEmbedder);
