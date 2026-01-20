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

#include <memory>
#include <string>
#include <vector>
#include <cmath>      // std::sqrt, std::abs, std::sin
#include <algorithm>  // std::max
#include <fstream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "TRandom3.h"
#include "correction.h"

using pat::Jet, pat::JetCollection;
typedef edm::View<Jet> JetView;

class PATJetSmearing : public edm::stream::EDProducer<> {
public:
  explicit PATJetSmearing(const edm::ParameterSet& iConfig);
  virtual ~PATJetSmearing() { ; }

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  void scaleJetP4(Jet& jet, double scale);

  edm::EDGetTokenT<JetView> srcToken;
  edm::EDGetTokenT<double> rhoToken;
  std::string scaleFileName_, config_, algo_;
  std::unique_ptr<correction::CorrectionSet> scaleFile_;
  const bool systematics_;

  std::string jerName_, jersfName_;
};

PATJetSmearing::PATJetSmearing(const edm::ParameterSet& iConfig)
    : srcToken(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
      rhoToken(consumes<double>(iConfig.getParameter<edm::InputTag>("rhoSrc"))),
      scaleFileName_(iConfig.getParameter<std::string>("scaleFile")),
      config_(iConfig.getParameter<std::string>("config")),
      algo_(iConfig.exists("algo") ? iConfig.getParameter<std::string>("algo") : "AK4PFPuppi"),
      systematics_(iConfig.exists("systematics") ? iConfig.getParameter<bool>("systematics") : false) {
  std::ifstream checkfile(scaleFileName_);
  if (!checkfile.good())
    scaleFileName_ = scaleFileName_.substr(scaleFileName_.find("/UWVV/") + 6);
  else
    checkfile.close();

  try {
    scaleFile_ = correction::CorrectionSet::from_file(scaleFileName_);
    if (scaleFile_ == nullptr)
      throw cms::Exception("Invalid JSON file") << scaleFileName_;
  } catch (...) {
    throw cms::Exception("Invalid JSON file") << "Filepath: " << scaleFileName_;
  }

  jerName_ = config_ + "_MC_PtResolution_" + algo_;
  jersfName_ = config_ + "_MC_ScaleFactor_" + algo_;
  auto it = scaleFile_->begin();
  for (; it != scaleFile_->end(); it++)
    if (it->first == jerName_)
      break;
  if (it == scaleFile_->end())
    throw cms::Exception("Invalid JER config") << "Config: " << jerName_;
  for (it = scaleFile_->begin(); it != scaleFile_->end(); it++)
    if (it->first == jersfName_)
      break;
  if (it == scaleFile_->end())
    throw cms::Exception("Invalid JERSF config") << "Config: " << jerName_;

  produces<JetCollection>();
  if (systematics_) {
    produces<JetCollection>("jerUp");
    produces<JetCollection>("jerDown");
  }
}

void PATJetSmearing::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<double> rho;
  iEvent.getByToken(rhoToken, rho);

  std::unique_ptr<JetCollection> out(new JetCollection());
  std::unique_ptr<JetCollection> outUp(new JetCollection());
  std::unique_ptr<JetCollection> outDn(new JetCollection());

  TRandom3 rand;
  rand.SetSeed(iEvent.id().event() + iEvent.id().run() + iEvent.id().luminosityBlock());

  for (size_t i = 0; i < in->size(); ++i) {
    const Jet& jet = in->at(i);
    out->push_back(jet);
    if (systematics_) {
      outUp->push_back(jet);
      outDn->push_back(jet);
    }

    float pt = jet.pt();
    float eta = jet.eta();
    const reco::GenJet* gen = jet.genJet();
    float genpt = (gen != nullptr) ? gen->pt() : -1.0;

    double reso = scaleFile_->at(jerName_)->evaluate({eta, pt, *rho});
    double scale = scaleFile_->at(jersfName_)->evaluate({eta, pt, "nom"});
    double jerCorr = 1.0;

    double gaus = 0.0;
    if (eta < 2.5 || eta > 3.0) {
      if (genpt > 0.0)
        jerCorr = std::max(0.0, 1.0 + (scale - 1.0) * (pt - genpt) / pt);
      else {
        gaus = rand.Gaus(0.0, reso);
        double varp = std::max(scale * scale - 1.0, 0.0);
        jerCorr = std::max(0.0, 1.0 + gaus * std::sqrt(varp));
      }
    }

    scaleJetP4(out->back(), jerCorr);
    out->back().addUserFloat("jerCorrInverse", 1. / jerCorr);

    if (systematics_) {
      // JER Uncertainty
      double scaleUp = scaleFile_->at(jersfName_)->evaluate({eta, pt, "up"});
      double scaleDn = scaleFile_->at(jersfName_)->evaluate({eta, pt, "down"});
      double jerCorrUp = 1.0;
      double jerCorrDn = 1.0;

      if (eta < 2.5 || eta > 3.0) {
        if (genpt > 0.0) {
          jerCorrUp = std::max(0.0, 1.0 + (scaleUp - 1.0) * (pt - genpt) / pt);
          jerCorrDn = std::max(0.0, 1.0 + (scaleDn - 1.0) * (pt - genpt) / pt);
        } else {
          double varpUp = std::max(scaleUp * scaleUp - 1.0, 0.0);
          double varpDn = std::max(scaleDn * scaleDn - 1.0, 0.0);
          jerCorrUp = std::max(0.0, 1.0 + gaus * std::sqrt(varpUp));
          jerCorrDn = std::max(0.0, 1.0 + gaus * std::sqrt(varpDn));
        }
      }

      scaleJetP4(outUp->back(), jerCorrUp);
      outUp->back().addUserFloat("jerCorrInverse", 1. / jerCorrUp);

      scaleJetP4(outDn->back(), jerCorrDn);
      outDn->back().addUserFloat("jerCorrInverse", 1. / jerCorrDn);
    }
  }

  iEvent.put(std::move(out));
  if (systematics_) {
    iEvent.put(std::move(outUp), "jerUp");
    iEvent.put(std::move(outDn), "jerDown");
  }
}

void PATJetSmearing::scaleJetP4(Jet& jet, double scale) {
  const auto p4 = jet.p4();
  jet.setP4(reco::Particle::LorentzVector(p4.px() * scale, p4.py() * scale, p4.pz() * scale, p4.energy() * scale));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetSmearing);
