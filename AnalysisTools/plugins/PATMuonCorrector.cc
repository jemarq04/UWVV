//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   PATMuonCorrector.cc                                                    //
//                                                                          //
//   Applies scale/smear corrections to muons.                              //
//                                                                          //
//   Author: Justin Marquez, U. Wisconsin                                   //
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
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "TRandom3.h"
#include "correction.h"
#include "PhysicsTools/NATModules/interface/MuonScaRe.h"

class PATMuonCorrector : public edm::stream::EDProducer<>
{
public:
  explicit PATMuonCorrector(const edm::ParameterSet&);
  ~PATMuonCorrector() {}


private:
  // Methods
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  double getCorrectedPt(const edm::Ptr<pat::Muon>& muon, std::string var="nom");

  // Data
  edm::EDGetTokenT<edm::View<pat::Muon> > muonCollectionToken_;

  bool isMC_;
  double maxPt_;
  std::string scaleFileName_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_;
  MuonScaRe corrector_;
};


// Constructors and destructors

PATMuonCorrector::PATMuonCorrector(const edm::ParameterSet& iConfig):
  muonCollectionToken_(consumes<edm::View<pat::Muon> >(iConfig.exists("src") ?
        iConfig.getParameter<edm::InputTag>("src") :
        edm::InputTag("slimmedMuons"))),
  isMC_(iConfig.getParameter<bool>("isMC")),
  maxPt_(iConfig.exists("maxPt") ? iConfig.getParameter<double>("maxPt") : 200.0),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
  corrector_(scaleFileName_)
{
  produces<std::vector<pat::Muon> >();
}


void PATMuonCorrector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::unique_ptr<std::vector<pat::Muon> > out = std::make_unique<std::vector<pat::Muon> >();

  edm::Handle<edm::View<pat::Muon> > muonsIn;
  iEvent.getByToken(muonCollectionToken_, muonsIn);

  for(edm::View<pat::Muon>::const_iterator mi = muonsIn->begin();
      mi != muonsIn->end(); mi++) // loop over muons
  {
    const edm::Ptr<pat::Muon> mptr(muonsIn, mi - muonsIn->begin());
    out->push_back(*mi); // copy muon to save correctly in event
    
    double uncorr_pt = mi->pt();
    double corr_pt   = getCorrectedPt(mptr);
    double syst_pt   = getCorrectedPt(mptr, "syst");
    double stat_pt   = getCorrectedPt(mptr, "stat");

    out->back().setP4(reco::Particle::PolarLorentzVector(corr_pt, mi->eta(), mi->phi(), mi->mass()));
    out->back().addUserFloat("uncorrected_pt", uncorr_pt);
    out->back().addUserFloat("ptScaleFactor", corr_pt/uncorr_pt);
    out->back().addUserFloat("syst_pt", syst_pt);
    out->back().addUserFloat("stat_pt", stat_pt);
  }

  iEvent.put(std::move(out));
}

double PATMuonCorrector::getCorrectedPt(const edm::Ptr<pat::Muon>& muon, std::string var){
  if (muon->pt() > maxPt_)
    return muon->pt();

  double corr_pt = corrector_.pt_scale(!isMC_, muon->pt(), muon->eta(), muon->phi(), muon->charge(), var);
  if (isMC_) corr_pt = corrector_.pt_resol(corr_pt, muon->eta(), muon->innerTrack()->hitPattern().trackerLayersWithMeasurement(), var);

  return corr_pt;
}

//define this as a plug-in
DEFINE_FWK_MODULE(PATMuonCorrector);
