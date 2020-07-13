//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetSmearing.cc                                                     //
//                                                                          //
//    Apply energy smearing to PF jets. Outputs a collection of jets with   //
//    the smearing applied and with a correction that undoes the smearing   //
//    stored as a userFloat                                                 //
//    To get the original 4-momentum back:                                  //
//        jet.p4() * jet.userFloat("jerCorrInverse")                        //
//                                                                          //
//    If systematic shifts are also requested (systematics=cms.bool(True),  //
//    also produces one collection with the smearing shifted up by one      //
//    sigma, and another with the smearing shifted down by one sigma, for   //
//    systematics. Labels for these are "jerUp" and "jerDown".              //
//                                                                          //
//    Obviously, this only makes sense for MC                               //
//                                                                          //
//    Author: Nate Woods, U. Wisconsin                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


#include<memory>
#include<string>
#include<vector>
#include<cmath> // std::sqrt, std::abs, std::sin
#include<algorithm> // std::max

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "JetMETCorrections/Modules/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetResolutionObject.h"
#include "DataFormats/Math/interface/LorentzVector.h"

#include "TRandom3.h"


typedef pat::Jet Jet;
typedef std::vector<Jet> VJet;
typedef edm::View<Jet> JetView;

class PATJetSmearing : public edm::stream::EDProducer<>
{
 public:
  explicit PATJetSmearing(const edm::ParameterSet& pset);
  virtual ~PATJetSmearing() {;}

 private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  edm::EDGetTokenT<JetView> srcToken;
  edm::EDGetTokenT<double> rhoToken;

  const bool systematics;
};


PATJetSmearing::PATJetSmearing(const edm::ParameterSet& pset) :
  srcToken(consumes<JetView>(pset.getParameter<edm::InputTag>("src"))),
  rhoToken(consumes<double>(pset.getParameter<edm::InputTag>("rhoSrc"))),
  systematics(pset.exists("systematics") ?
              pset.getParameter<bool>("systematics") : false)
{
  produces<VJet>();
  if(systematics)
    {
      produces<VJet>("jerUp");
      produces<VJet>("jerDown");
    }
}


void PATJetSmearing::produce(edm::Event& iEvent,
                             const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<double> rho;
  iEvent.getByToken(rhoToken, rho);

  std::unique_ptr<VJet> out(new VJet());
  std::unique_ptr<VJet> outUp(new VJet());
  std::unique_ptr<VJet> outDn(new VJet());

  JME::JetResolutionScaleFactor resSF =
    JME::JetResolutionScaleFactor::get(iSetup, "AK4PFchs");
  JME::JetResolution resPt = JME::JetResolution::get(iSetup, "AK4PFchs_pt");

  for(size_t i = 0; i < in->size(); ++i)
    {
      const Jet& jet = in->at(i);
      
      float pt = jet.pt();
      float eta = jet.eta();
      float phi = jet.phi();
      //std::cout<<"jet.pt: "<<jet.pt()<<' '<<systematics<<std::endl;
      //std::cout<<"===============================================before setting parameters===================="<<std::endl;
      JME::JetParameters params;
      params.setJetPt(pt).setJetEta(eta).setRho(*rho);
      //std::cout<<"===============================================after setting parameters===================="<<std::endl;
      float relPtErr = resPt.getResolution(params);

      JME::JetParameters paramsSF;
      paramsSF.setJetPt(pt).setJetEta(eta).setRho(*rho);
      
      float sf = resSF.getScaleFactor(paramsSF);
      //std::cout<<"===============================================after something===================="<<std::endl;            
      float sfUp = systematics ? resSF.getScaleFactor(paramsSF, Variation::UP) : 0.;
      float sfDn = systematics ? resSF.getScaleFactor(paramsSF, Variation::DOWN) : 0.;

      double ptJER, ptJERUp, ptJERDn;
      const reco::GenJet* gen = jet.genJet();

      if(gen && reco::deltaR(eta, phi, gen->eta(), gen->phi()) < 0.2 &&
         (std::abs(pt - gen->pt()) < 3. * relPtErr * pt))
        {
          float genPt = gen->pt();

          ptJER = std::max(float(0.), genPt + sf * (pt - genPt));
          ptJERUp = std::max(float(0.), genPt + sfUp * (pt - genPt));
          ptJERDn = std::max(float(0.), genPt + sfDn * (pt - genPt));
        }
      else
        {
          TRandom3 rand;
          rand.SetSeed(std::abs(static_cast<int>(std::sin(phi)*100000)));
          float smear = rand.Gaus(0.,1.);
          float sig = std::sqrt(sf * sf - 1.) * relPtErr * pt;
          float sigUp = std::sqrt(sfUp * sfUp - 1.) * relPtErr * pt;
          float sigDn = std::sqrt(sfDn * sfDn - 1.) * relPtErr * pt;
          ptJER = std::max(float(0.), smear * sig + pt);
          ptJERUp = std::max(float(0.), smear * sigUp + pt);
          ptJERDn = std::max(float(0.), smear * sigDn + pt);
        }

      float jerCorr = ptJER / pt;
      float jerCorrUp = ptJERUp / pt;
      float jerCorrDn = ptJERDn / pt;

      math::XYZTLorentzVector p4JER = jerCorr * jet.p4();
      math::XYZTLorentzVector p4JERUp = jerCorrUp * jet.p4();
      math::XYZTLorentzVector p4JERDn = jerCorrDn * jet.p4();

      out->push_back(jet);
      out->back().setP4(p4JER);
      out->back().addUserFloat("jerCorrInverse", 1./jerCorr);

      if(systematics)
        {
          outUp->push_back(jet);
          outUp->back().setP4(p4JERUp);
          outUp->back().addUserFloat("jerCorrInverse", 1./jerCorrUp);

          outDn->push_back(jet);
          outDn->back().setP4(p4JERDn);
          outDn->back().addUserFloat("jerCorrInverse", 1./jerCorrDn);
        }
    }

  iEvent.put(std::move(out));
  if(systematics)
    {
      iEvent.put(std::move(outUp), "jerUp");
      iEvent.put(std::move(outDn), "jerDown");
    }
}


#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetSmearing);
