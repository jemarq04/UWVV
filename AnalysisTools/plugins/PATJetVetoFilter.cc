//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetPUIDProducer.cc                                                 //
//                                                                          //
//    Embed TEMPORARY passing PUID userFloat                                //
//                                                                          //
//    Author: Justin Marquez, U. Wisconsin                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <string>
#include <vector>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"

#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "UWVV/Utilities/interface/helpers.h"
#include "correction.h"

using pat::Jet, pat::JetCollection;
typedef edm::View<Jet> JetView;

using pat::Muon, pat::MuonCollection;
typedef edm::View<Muon> MuonView;

class PATJetVetoFilter : public edm::stream::EDFilter<>
{
  public:
    explicit PATJetVetoFilter(const edm::ParameterSet& iConfig);
    virtual ~PATJetVetoFilter() {;}

  private:
    bool filter(edm::Event& iEvent, const edm::EventSetup& iSetup);

    edm::EDGetTokenT<JetView> jetSrcToken_;
    edm::EDGetTokenT<MuonView> muonSrcToken_;
    std::string vetoFileName_;
    std::unique_ptr<correction::CorrectionSet> vetoFile_;
};

PATJetVetoFilter::PATJetVetoFilter(const edm::ParameterSet& iConfig) :
  jetSrcToken_(consumes<JetView>(iConfig.getParameter<edm::InputTag>("jets"))),
  muonSrcToken_(consumes<MuonView>(iConfig.getParameter<edm::InputTag>("muons"))),
  vetoFileName_(iConfig.getParameter<std::string>("vetoFile"))
{
  try{
    vetoFile_ = correction::CorrectionSet::from_file(vetoFileName_);
    if (vetoFile_ == nullptr) throw cms::Exception("Invalid JSON file");
  }
  catch (...){
    throw cms::Exception("Invalid JSON file") << "Filename: " << vetoFileName_;
  }
}

bool PATJetVetoFilter::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<JetView> jets;
  iEvent.getByToken(jetSrcToken_, jets);

  edm::Handle<MuonView> muons;
  iEvent.getByToken(muonSrcToken_, muons);

  for (JetView::const_iterator ijet = jets->begin(); ijet != jets->end(); ijet++)
  {
    bool jetID    = ijet->userFloat("idTight") > 0.5;
    float jetpt   = ijet->pt();
    float jetCEMF = ijet->chargedEmEnergyFraction();
    float jetNEMF = ijet->neutralEmEnergyFraction();

    // Jet must pass loose selections
    if (jetpt < 15 || !jetID || jetCEMF+jetNEMF < 0.9) continue;

    for (MuonView::const_iterator imu = muons->begin(); imu != muons->end(); imu++)
    {
      if (!imu->isPFMuon()) continue;

      // Jet must not be within 0.2 deltaR of PF muon
      if (reco::deltaR(ijet->p4(), imu->p4()) < 0.2) continue;
    }

    // Now, apply veto to jets passing above selections
    float output = vetoFile_->begin()->second->evaluate({"jetvetomap", ijet->eta(), ijet->phi()});
    if (std::fabs(output) > 1e-6)
      return false; // if a jet failes the veto, the event is discarded
  }

  return true;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetVetoFilter);
