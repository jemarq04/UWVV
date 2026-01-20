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

class PATMuonCorrector : public edm::stream::EDProducer<> {
public:
  explicit PATMuonCorrector(const edm::ParameterSet&);
  ~PATMuonCorrector() {
    if (corrector_ != nullptr)
      delete corrector_;
  }

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  double getPtScale(const Muon& muon);
  double getPtSmear(const Muon& muon, double pt_scale, int event, int lumi);
  double getPtScaleVar(const Muon& muon, double corr_pt, std::string var);
  double getPtSmearVar(const Muon& muon, double pt_scale, double corr_pt, std::string var);

  edm::EDGetTokenT<MuonView> srcToken_;
  const bool isMC_;
  const double minPt_;
  std::string scaleFileName_;
  const bool hasSeed_;
  const ULong64_t seed_;
  MuonScaRe* corrector_;
};

PATMuonCorrector::PATMuonCorrector(const edm::ParameterSet& iConfig)
    : srcToken_(consumes<MuonView>(iConfig.exists("src") ? iConfig.getParameter<edm::InputTag>("src")
                                                         : edm::InputTag("slimmedMuons"))),
      isMC_(iConfig.getParameter<bool>("isMC")),
      minPt_(iConfig.exists("minPt") ? iConfig.getParameter<double>("minPt") : 26.0),
      scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
      hasSeed_(iConfig.exists("seed")),
      seed_(hasSeed_ ? iConfig.getParameter<ULong64_t>("seed") : 0) {
  std::ifstream checkfile(scaleFileName_);
  if (!checkfile.good())
    scaleFileName_ = scaleFileName_.substr(scaleFileName_.find("/UWVV/") + 6);
  else
    checkfile.close();
  try {
    corrector_ = new MuonScaRe(scaleFileName_, minPt_);
    //if (hasSeed_) corrector_->setSeed(seed_); //removed from NATModules, commented out for now
  } catch (...) {
    throw cms::Exception("InvalidFile") << "Cannot find muon correction file: " << scaleFileName_ << std::endl;
  }

  produces<MuonCollection>();
}

void PATMuonCorrector::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<MuonView> muonsIn;
  iEvent.getByToken(srcToken_, muonsIn);

  std::unique_ptr<MuonCollection> out(new MuonCollection());

  for (MuonView::const_iterator mi = muonsIn->begin(); mi != muonsIn->end(); mi++) {
    out->push_back(*mi);  // copy muon to save correctly in event
    Muon& mu = out->back();

    double uncorr_pt = mu.pt();
    double pt_scale = getPtScale(mu);
    double corr_pt =
        isMC_ ? getPtSmear(mu, pt_scale, (int)iEvent.id().event(), (int)iEvent.id().luminosityBlock()) : pt_scale;

    mu.addUserFloat("uncorrected_pt", uncorr_pt);
    mu.addUserFloat("ptScaleFactor", corr_pt / uncorr_pt);
    if (isMC_) {
      mu.addUserFloat("scaleUp_pt", getPtScaleVar(mu, corr_pt, "up"));
      mu.addUserFloat("scaleDn_pt", getPtScaleVar(mu, corr_pt, "dn"));
      mu.addUserFloat("smearUp_pt", getPtSmearVar(mu, pt_scale, corr_pt, "up"));
      mu.addUserFloat("smearDn_pt", getPtSmearVar(mu, pt_scale, corr_pt, "dn"));
    }
    mu.setP4(reco::Particle::PolarLorentzVector(corr_pt, mu.eta(), mu.phi(), mu.mass()));
  }

  iEvent.put(std::move(out));
}

double PATMuonCorrector::getPtScale(const Muon& muon) {
  return corrector_->pt_scale(!isMC_, muon.pt(), muon.eta(), muon.phi(), muon.charge());
}

double PATMuonCorrector::getPtSmear(const Muon& muon, double corr_pt, int event, int lumi) {
  return corrector_->pt_resol(
      corr_pt,
      muon.eta(),
      muon.phi(),
      muon.innerTrack().isNonnull() ? muon.innerTrack()->hitPattern().trackerLayersWithMeasurement() : 0,
      event,
      lumi);
}

double PATMuonCorrector::getPtScaleVar(const Muon& muon, double corr_pt, std::string var) {
  return corrector_->pt_scale_var(corr_pt, muon.eta(), muon.phi(), muon.charge(), var);
}

double PATMuonCorrector::getPtSmearVar(const Muon& muon, double pt_scale, double corr_pt, std::string var) {
  return corrector_->pt_resol_var(pt_scale, corr_pt, muon.eta(), var);
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATMuonCorrector);
