///////////////////////////////////////////////////////////////////////////////
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// system includes
#include <memory>
#include <iostream>

// CMS includes
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "UWVV/DataFormats/interface/DressedGenParticle.h"

using pat::CompositeCandidate;
typedef edm::View<CompositeCandidate> CCandView;
using reco::GenParticle, reco::GenParticleCollection;

class GenZZXsecAnalyzer : public edm::one::EDAnalyzer<>
{
  public:
    explicit GenZZXsecAnalyzer(const edm::ParameterSet &iConfig);
    virtual ~GenZZXsecAnalyzer(){};

  private:
    void beginJob() override;
    void analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) override;
    void endJob() override;

    double getPrimaryZMassDifference(const CompositeCandidate& cand);
    bool passOnShellCut(const CompositeCandidate& cand);
    bool passFiducialCuts(const CompositeCandidate& cand);

    edm::EDGetTokenT<CCandView> candSrc_;
    edm::EDGetTokenT<GenEventInfoProduct> genSrc_;
    std::vector<std::string> daughterNames_;
    std::string label_;
    const bool isDressed_;

    int numEventsOnShell_;
    int numEventsFiducial_;
    double sumWeightsOnShell_;
    double sumWeightsFiducial_;
};

GenZZXsecAnalyzer::GenZZXsecAnalyzer(const edm::ParameterSet &iConfig) :
  candSrc_(consumes<CCandView>(iConfig.getParameter<edm::InputTag>("src"))),
  genSrc_(consumes<GenEventInfoProduct>(edm::InputTag("generator"))),
  daughterNames_(iConfig.getParameter<std::vector<std::string>>("names")),
  label_(iConfig.exists("label") ? iConfig.getParameter<std::string>("label") : "xsec"),
  isDressed_(iConfig.exists("dressed") ? iConfig.getParameter<bool>("dressed") : false)
{
}

void GenZZXsecAnalyzer::analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup)
{
  edm::Handle<CCandView> cands;
  iEvent.getByToken(candSrc_, cands);

  edm::Handle<GenEventInfoProduct> genEvent;
  iEvent.getByToken(genSrc_, genEvent);

  if (!cands->size()) return;

  size_t bestCandIdx = 0;
  double bestZMassDifference = getPrimaryZMassDifference(cands->at(0));
  if (isDressed_){//determine best candidate
    for (size_t i=1; i<cands->size(); i++){
      double zMassDifference = getPrimaryZMassDifference(cands->at(i));
      if (zMassDifference < bestZMassDifference){
        bestCandIdx = i;
        bestZMassDifference = zMassDifference;
      }
    }
  }

  if (!passOnShellCut(cands->at(bestCandIdx))) return;
  numEventsOnShell_++;
  sumWeightsOnShell_ += genEvent->weight();

  if (!passFiducialCuts(cands->at(bestCandIdx))) return;
  numEventsFiducial_++;
  sumWeightsFiducial_ += genEvent->weight();
}

double GenZZXsecAnalyzer::getPrimaryZMassDifference(const CompositeCandidate& cand){
  double z1diff = std::abs(cand.userFloat((daughterNames_[0] + "_" + daughterNames_[1] + "_Mass").c_str())-91.1876);
  double z2diff = std::abs(cand.userFloat((daughterNames_[2] + "_" + daughterNames_[3] + "_Mass").c_str())-91.1876);

  return z1diff < z2diff? z1diff : z2diff;
}

bool GenZZXsecAnalyzer::passOnShellCut(const CompositeCandidate& cand){
  double z1mass = cand.userFloat((daughterNames_[0] + "_" + daughterNames_[1] + "_Mass").c_str());
  double z2mass = cand.userFloat((daughterNames_[2] + "_" + daughterNames_[3] + "_Mass").c_str());

  return (z1mass > 60 && z1mass < 120) && (z2mass > 60 && z2mass < 120);
}

bool GenZZXsecAnalyzer::passFiducialCuts(const CompositeCandidate& cand){
  std::vector<const reco::Candidate*> daughters;
  for (size_t i=0; i<cand.numberOfDaughters(); i++){
    if (cand.daughter(i)->pdgId() == 23){
      for (size_t j=0; j<cand.daughter(i)->numberOfDaughters(); j++){
        int pdgId = cand.daughter(i)->daughter(j)->pdgId();
        if (std::abs(pdgId) == 11 || std::abs(pdgId) == 13)
          daughters.push_back(cand.daughter(i)->daughter(j));
      }
    }
  }

  //pt cuts: at least one lepton with pt above 20 
  //         with at least one other lepton with pt above 10
  bool passPtCut = false;
  for (int i=0; i<4; i++){
    if (daughters[i]->pt() < 20.0)
      continue;
    bool pass = false;
    for (int j=0; j<4; j++){
      if (j == i) continue;

      pass = daughters[j]->pt() > 10.0;
      if (pass) break;
    }
    if (pass){
      passPtCut = true;
      break;
    }
  }
  if (!passPtCut) return false;

  //eta cut
  for (int i=0; i<4; i++)
    if (std::abs(daughters[i]->eta()) > 2.5)
      return false;

  //QCD veto
  for (int i=0; i<3; i++)
    for (int j=i+1; j<4; j++)
      if (cand.userFloat((daughterNames_[i] + "_" + daughterNames_[j] + "_SS").c_str()) < 0.5 && 
          cand.userFloat((daughterNames_[i] + "_" + daughterNames_[j] + "_Mass").c_str()) < 4.0)
        return false;

  return true;
}

void GenZZXsecAnalyzer::beginJob(){
  numEventsOnShell_ = 0;
  numEventsFiducial_ = 0;
  sumWeightsOnShell_ = 0.0;
  sumWeightsFiducial_ = 0.0;
}

void GenZZXsecAnalyzer::endJob(){
  std::cout << "=== Gen ZZ Xsec Analyzer ===" << std::endl;
  std::cout << "Dressed: " << isDressed_ << std::endl;
  std::cout << "On Shell " << label_ << ": " << sumWeightsOnShell_/numEventsOnShell_   << " (" << numEventsOnShell_ << ")" << std::endl;
  std::cout << "Fiducial " << label_ << ": " << sumWeightsFiducial_/numEventsFiducial_ << " (" << numEventsFiducial_<< ")" << std::endl;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GenZZXsecAnalyzer);
