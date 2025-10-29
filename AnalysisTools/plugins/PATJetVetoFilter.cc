//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetVetoFilter.cc                                                   //
//                                                                          //
//    Author: Justin Marquez, U. Wisconsin                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <string>
#include <vector>
#include <fstream>

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

class PATJetVetoFilter : public edm::stream::EDFilter<>
{
  public:
    explicit PATJetVetoFilter(const edm::ParameterSet& iConfig);
    virtual ~PATJetVetoFilter() {;}

  private:
    bool filter(edm::Event& iEvent, const edm::EventSetup& iSetup);

    edm::EDGetTokenT<JetView> jetSrcToken_;
    std::string vetoFileName_;
    std::unique_ptr<correction::CorrectionSet> vetoFile_;
};

PATJetVetoFilter::PATJetVetoFilter(const edm::ParameterSet& iConfig) :
  jetSrcToken_(consumes<JetView>(iConfig.getParameter<edm::InputTag>("jets"))),
  vetoFileName_(iConfig.getParameter<std::string>("vetoFile"))
{
  std::ifstream checkfile(vetoFileName_);
  if (!checkfile.good()) vetoFileName_ = vetoFileName_.substr(vetoFileName_.find("/UWVV/") + 6);
  else checkfile.close();

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

  for (JetView::const_iterator ijet = jets->begin(); ijet != jets->end(); ijet++)
  {
    bool jetID    = ijet->userFloat("idTightLepVeto") > 0.5;
    float jetpt   = ijet->pt();
    float jetCEMF = ijet->chargedEmEnergyFraction();
    float jetNEMF = ijet->neutralEmEnergyFraction();

    // Jet must pass loose selections
    if (jetpt < 15 || !jetID || jetCEMF+jetNEMF < 0.9) continue;

    // Now, apply veto to jets passing above selections
    float output = vetoFile_->begin()->second->evaluate({"jetvetomap", ijet->eta(), ijet->phi()});
    if (std::fabs(output) > 0.0)
      return false; // if a jet failes the veto, the event is discarded
  }

  return true;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetVetoFilter);
