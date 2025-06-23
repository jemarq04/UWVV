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
#include <fstream>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/Common/interface/View.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "PhysicsTools/NATModules/interface/MuonScaRe.h"

using pat::Muon, pat::MuonCollection;
typedef edm::View<Muon> MuonView;

class PATMuonCorrector : public edm::stream::EDProducer<>
{
  public:
    explicit PATMuonCorrector(const edm::ParameterSet&);
    ~PATMuonCorrector() {if (corrector_ != nullptr) delete corrector_;}


  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    double getCorrectedPt(const Muon& muon, std::string var="nom");

    edm::EDGetTokenT<MuonView> srcToken_;
    const bool isMC_;
    const double maxPt_;
    std::string scaleFileName_;
    const bool hasSeed_;
    const ULong64_t seed_;
    MuonScaRe *corrector_;
};

PATMuonCorrector::PATMuonCorrector(const edm::ParameterSet& iConfig):
  srcToken_(consumes<MuonView>(iConfig.exists("src") ?
      iConfig.getParameter<edm::InputTag>("src") :
      edm::InputTag("slimmedMuons"))),
  isMC_(iConfig.getParameter<bool>("isMC")),
  maxPt_(iConfig.exists("maxPt") ? iConfig.getParameter<double>("maxPt") : 200.0),
  scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
  hasSeed_(iConfig.exists("seed")),
  seed_(hasSeed_? iConfig.getParameter<ULong64_t>("seed") : 0)
{
  std::ifstream checkfile(scaleFileName_);
  if (!checkfile.good()) scaleFileName_ = scaleFileName_.substr(scaleFileName_.find("/UWVV/") + 6);
  else checkfile.close();
  try{
    corrector_ = new MuonScaRe(scaleFileName_);
    if (hasSeed_) corrector_->setSeed(seed_);
  }
  catch(...){
    throw cms::Exception("InvalidFile") << "Cannot find muon correction file: "
      << scaleFileName_ << std::endl;
  }

  produces<MuonCollection>();
}

void PATMuonCorrector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<MuonView> muonsIn;
  iEvent.getByToken(srcToken_, muonsIn);

  std::unique_ptr<MuonCollection> out(new MuonCollection());

  for(MuonView::const_iterator mi = muonsIn->begin(); mi != muonsIn->end(); mi++)
  {
    out->push_back(*mi); // copy muon to save correctly in event
    Muon& mu = out->back();

    double uncorr_pt = mu.pt();
    double corr_pt   = getCorrectedPt(mu);

    mu.addUserFloat("uncorrected_pt", uncorr_pt);
    mu.addUserFloat("ptScaleFactor", corr_pt/uncorr_pt);
    /* TODO: Updated muon corrections removed 'syst' and 'stat' variations on k_data, which is
     *  necessary for calculating any of these variations for MC at the moment. Once this is fixed,
     *  these can be added back in.
    if (isMC_){
      mu.addUserFloat("syst_pt", getCorrectedPt(mu, "syst"));
      mu.addUserFloat("stat_pt", getCorrectedPt(mu, "stat"));
    }
     */
    mu.setP4(reco::Particle::PolarLorentzVector(corr_pt, mu.eta(), mu.phi(), mu.mass()));
  }

  iEvent.put(std::move(out));
}

double PATMuonCorrector::getCorrectedPt(const Muon& muon, std::string var){
  if (muon.pt() > maxPt_)
    return muon.pt();

  double corr_pt = corrector_->pt_scale(!isMC_, muon.pt(), muon.eta(), muon.phi(), muon.charge(), var);
  if (isMC_)
    corr_pt = corrector_->pt_resol(corr_pt, muon.eta(), muon.innerTrack().isNonnull()? muon.innerTrack()->hitPattern().trackerLayersWithMeasurement() : 0, var);

  return corr_pt;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATMuonCorrector);
