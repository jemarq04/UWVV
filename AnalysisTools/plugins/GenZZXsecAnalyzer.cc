///////////////////////////////////////////////////////////////////////////////
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// system includes
#include <memory>
#include <iostream>

// CMS includes
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDAnalyzer.h"

#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

#include "UWVV/DataFormats/interface/DressedGenParticle.h"

typedef pat::CompositeCandidate CCand;
typedef edm::View<CCand> CCandView;
typedef std::vector<CCand> CCandCollection;
using reco::GenParticle, reco::GenParticleCollection;

class GenZZXsecAnalyzer : public edm::stream::EDAnalyzer<>
{
  public:
    explicit GenZZXsecAnalyzer(const edm::ParameterSet &iConfig);
    virtual ~GenZZXsecAnalyzer(){};

  private:
    void beginStream(edm::StreamID) override;
    virtual void analyze(edm::Event &iEvent, const edm::EventSetup &iSetup);
    void endStream() override;

    edm::EDGetTokenT<GenParticleCollection> eSrc_, mSrc_;
    edm::EDGetTokenT<CCandView> candSrc_;
    const bool isDressed_;

    int numEvents_;
    double sumWeightsOnShell_;
    double sumWeightsFiducial_;
};

GenZZXsecAnalyzer::GenZZXsecAnalyzer(const edm::ParameterSet &iConfig) :
  eSrc_(consumes<GenParticleCollection>(iConfig.getParameter<edm::InputTag>("electrons"))),
  mSrc_(consumes<GenParticleCollection>(iConfig.getParameter<edm::InputTag>("muons"))),
  candSrc_(consumes<CCandView>(iConfig.getParameter<edm::InputTag>("ZZ"))),
  isDressed_(iConfig.exists("dressed") ? iConfig.getParameter<bool>("dressed") : false)
{
}

void GenZZXsecAnalyzer::analyze(edm::Event &iEvent, const edm::EventSetup &iSetup)
{
  edm::Handle<GenParticleCollection> electrons;
  iEvent.getByToken(eSrc_, electrons);

  edm::Handle<GenParticleCollection> muons;
  iEvent.getByToken(mSrc_, muons);

  edm::Handle<CCandView> cands;
  iEvent.getByToken(candSrc_, cands);

  numEvents_++;
}

void GenZZXsecAnalyzer::beginStream(edm::StreamID){
  numEvents_ = 0;
  sumWeightsOnShell_ = 0.0;
  sumWeightsFiducial_ = 0.0;
}

void GenZZXsecAnalyzer::endStream(){
  std::cout << std::endl;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GenZZXsecAnalyzer);
