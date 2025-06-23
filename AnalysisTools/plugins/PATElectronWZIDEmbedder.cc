///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATElectronWZIDEmbedder                                                //
//                                                                           //
//    Embeds WW ID defined in section 5.2.1 of AN-15-299                     //
//                                                                           //
//    Devin Taylor, U. Wisconsin                                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

// system includes
#include <memory>
#include <vector>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

using pat::Electron, pat::ElectronCollection;
typedef edm::View<Electron> ElectronView;

using reco::Vertex;
typedef edm::View<Vertex> VertexView;

class PATElectronWZIDEmbedder : public edm::stream::EDProducer<>
{
  public:
    PATElectronWZIDEmbedder(const edm::ParameterSet& iConfig);
    virtual ~PATElectronWZIDEmbedder(){}

  private:
    void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    const edm::EDGetTokenT<ElectronView> srcToken_;
    const edm::EDGetTokenT<VertexView> vertexToken_;
    std::vector<std::string> pogIDNames_;
};

PATElectronWZIDEmbedder::PATElectronWZIDEmbedder(const edm::ParameterSet& iConfig):
  srcToken_(consumes<ElectronView>(iConfig.getParameter<edm::InputTag>("src"))),
  vertexToken_(consumes<VertexView>(iConfig.getParameter<edm::InputTag>("vertexSrc"))),
  pogIDNames_(iConfig.getUntrackedParameter<std::vector<std::string>>("pogIDs",
        std::vector<std::string>({"IsCBVIDTight", "IsCBVIDMedium",
      "IsCBVIDLoose", "IsCBVIDVeto", "IsCBVIDHLTSafe"})))
{
  produces<ElectronCollection>();
}

void PATElectronWZIDEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<ElectronView> in;
  iEvent.getByToken(srcToken_, in);

  edm::Handle<VertexView> vertices;
  iEvent.getByToken(vertexToken_, vertices);

  const Vertex& thePV = *vertices->begin();

  std::unique_ptr<ElectronCollection> out(new ElectronCollection());

  for (size_t i = 0; i < in->size(); ++i)
  {
    out->push_back(in->at(i));
    Electron& electron = out->back();

    double pt = electron.pt();
    double dEtaIn = std::abs(electron.deltaEtaSuperClusterTrackAtVtx());
    double dPhiIn = std::abs(electron.deltaPhiSuperClusterTrackAtVtx());
    double sigmaIEtaIEta = electron.sigmaIetaIeta();
    double hOverE = electron.hcalOverEcal();
    double oneOverEMinusOneOverP = abs((1.-electron.eSuperClusterOverP())*1./electron.ecalEnergy());
    double ecalPFClusterIso = electron.ecalPFClusterIso();
    double hcalPFClusterIso = electron.hcalPFClusterIso();
    double trackIso = electron.dr03TkSumPt();
    int missingHits = electron.gsfTrack()->hitPattern().numberOfAllHits(reco::HitPattern::MISSING_INNER_HITS);
    double dxy = std::abs(electron.gsfTrack()->dxy(thePV.position()));
    double dz = std::abs(electron.gsfTrack()->dz(thePV.position()));
    bool passConversionVeto = electron.passConversionVeto();

    bool passLoose = true;
    if (electron.isEB()) {
      if (!(dEtaIn < 0.01))
        passLoose = false;
      if (!(dPhiIn < 0.04))
        passLoose = false;
      if (!(sigmaIEtaIEta < 0.011))
        passLoose = false;
      if (!(hOverE < 0.08))
        passLoose = false;
      if (!(oneOverEMinusOneOverP < 0.01))
        passLoose = false;
      if (!(ecalPFClusterIso/pt < 0.45))
        passLoose = false;
      if (!(hcalPFClusterIso/pt < 0.25))
        passLoose = false;
      if (!(trackIso/pt < 0.2))
        passLoose = false;
      if (!(missingHits <= 2))
        passLoose = false;
      if (!(dxy < 0.1))
        passLoose = false;
      if (!(dz < 0.373))
        passLoose = false;
      if (!passConversionVeto)
        passLoose = false;
    }
    else if (electron.isEE()) {
      if (!(dEtaIn < 0.01))
        passLoose = false;
      if (!(dPhiIn < 0.08))
        passLoose = false;
      if (!(sigmaIEtaIEta < 0.031))
        passLoose = false;
      if (!(hOverE < 0.08))
        passLoose = false;
      if (!(oneOverEMinusOneOverP < 0.01))
        passLoose = false;
      if (!(ecalPFClusterIso/pt < 0.45))
        passLoose = false;
      if (!(hcalPFClusterIso/pt < 0.25))
        passLoose = false;
      if (!(trackIso/pt < 0.2))
        passLoose = false;
      if (!(missingHits <= 1))
        passLoose = false;
      if (!(dxy < 0.2))
        passLoose = false;
      if (!(dz < 0.602))
        passLoose = false;
      if (!passConversionVeto)
        passLoose = false;
    }
    else
      passLoose = false;

    electron.addUserInt("IsWWLoose", passLoose);
    for (auto& id : pogIDNames_){
        if (!electron.hasUserFloat(id.c_str())) continue;
        bool passesDXY = electron.isEB() ? dxy < 0.05 : dxy < 0.1;
        bool passesDZ = electron.isEB() ? dz < 0.1 : dz < 0.2;
        bool passesAll = electron.userFloat(id.c_str()) && passesDXY && passesDZ;
        electron.addUserFloat(id+"wIP", passesAll);
    }
  }

  iEvent.put(std::move(out));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATElectronWZIDEmbedder);
