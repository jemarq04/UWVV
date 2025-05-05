// PATMuonIDEmbedder.cc
// Embeds muons ids as userInts for later
// via Devin Taylor, U. Wisconsin
// with modifications by K. Long, U. Wisconsin

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

using pat::Muon, pat::MuonCollection;
typedef edm::View<Muon> MuonView;
typedef reco::Muon RecoMuon;

class PATMuonIDEmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit PATMuonIDEmbedder(const edm::ParameterSet&);
    ~PATMuonIDEmbedder() {}

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  private:
    virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

    bool isWZTightMuon(const Muon& patMu, const reco::Vertex& pv);
    bool isMediumMuonICHEP(const RecoMuon& recoMu);
    bool isWZMediumMuon(const Muon& patMu, const reco::Vertex& pv);
    bool isWZTightMuonNoIso(const Muon& patMu, const reco::Vertex& pv);
    bool isWZLooseMuon(const Muon& patMu, const reco::Vertex& pv);
    bool isWZLooseMuonNoIso(const Muon& patMu, const reco::Vertex& pv);
    bool isSoftMuonICHEP(const RecoMuon& recoMu, const reco::Vertex& pv);

    edm::EDGetTokenT<MuonView> srcToken_;
    edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
};

PATMuonIDEmbedder::PATMuonIDEmbedder(const edm::ParameterSet& iConfig):
  srcToken_(consumes<MuonView>(iConfig.getParameter<edm::InputTag>("src"))),
  vertexToken_(consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertexSrc")))
{
  produces<MuonCollection>();
}

void PATMuonIDEmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  edm::Handle<MuonView> muonsIn;
  iEvent.getByToken(srcToken_, muonsIn);

  edm::Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vertexToken_, vertices);

  const reco::Vertex& pv = *vertices->begin();
  std::unique_ptr<MuonCollection> out(new MuonCollection());

  for (MuonView::const_iterator imu = muonsIn->begin(); imu != muonsIn->end(); imu++)
  {
    out->push_back(*imu);
    Muon &mu = out->back();

    mu.addUserInt("isTightMuon", mu.isTightMuon(pv));
    mu.addUserInt("isMediumMuonICHEP", isMediumMuonICHEP(mu));
    mu.addUserInt("isWZMediumMuon", isWZMediumMuon(mu, pv));
    mu.addUserInt("isWZTightMuon", isWZTightMuon(mu, pv));
    mu.addUserInt("isWZTightMuonNoIso", isWZTightMuonNoIso(mu, pv));
    mu.addUserInt("isWZLooseMuon", isWZLooseMuon(mu, pv));
    mu.addUserInt("isWZLooseMuonNoIso", isWZLooseMuonNoIso(mu, pv));
    mu.addUserInt("isSoftMuon", mu.isSoftMuon(pv));
    mu.addUserInt("isSoftMuonICHEP", isSoftMuonICHEP(mu,pv));
    mu.addUserInt("isHighPtMuon", mu.isHighPtMuon(pv));
    mu.addUserFloat("segmentCompatibility", muon::segmentCompatibility(mu));
    mu.addUserInt("isGoodMuon", muon::isGoodMuon(mu, muon::TMOneStationTight));
    mu.addUserInt("highPurityTrack", mu.innerTrack().isNonnull()? mu.innerTrack()->quality(reco::TrackBase::highPurity) : 0);
  }

  iEvent.put(std::move(out));
}

// ICHEP short term IDs
// https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideMuonIdRun2#Short_Term_Instructions_for_ICHE
bool PATMuonIDEmbedder::isMediumMuonICHEP(const RecoMuon & recoMu)
{
  bool goodGlob = recoMu.isGlobalMuon() &&
    recoMu.globalTrack()->normalizedChi2() < 3 &&
    recoMu.combinedQuality().chi2LocalPosition < 12 &&
    recoMu.combinedQuality().trkKink < 20;
  bool isMedium = muon::isLooseMuon(recoMu) &&
    recoMu.innerTrack()->validFraction() > 0.49 &&
    muon::segmentCompatibility(recoMu) > (goodGlob ? 0.303 : 0.451);
  return isMedium;
}


bool PATMuonIDEmbedder::isWZLooseMuon(const Muon& patMu, const reco::Vertex& pv)
{
  reco::MuonPFIsolation pfIsoDB04 = patMu.pfIsolationR04();
  float muIso = (pfIsoDB04.sumChargedHadronPt
      + std::max(0., pfIsoDB04.sumNeutralHadronEt
        + pfIsoDB04.sumPhotonEt
        - 0.5*pfIsoDB04.sumPUPt)
      ) / patMu.pt();
  return isWZLooseMuonNoIso(patMu, pv) && muIso < 0.4;
}


bool PATMuonIDEmbedder::isWZLooseMuonNoIso(const Muon& patMu, const reco::Vertex& pv)
{
  return isMediumMuonICHEP(patMu) &&
    std::abs(patMu.innerTrack()->dxy(pv.position())) < 0.02 &&
    std::abs(patMu.innerTrack()->dz(pv.position())) < 0.1 &&
    patMu.trackIso()/patMu.pt() < 0.4;
}


bool PATMuonIDEmbedder::isWZTightMuon(const Muon& patMu, const reco::Vertex& pv)
{
  reco::MuonPFIsolation pfIsoDB04 = patMu.pfIsolationR04();
  float muIso = (pfIsoDB04.sumChargedHadronPt
      + std::max(0., pfIsoDB04.sumNeutralHadronEt
        + pfIsoDB04.sumPhotonEt
        - 0.5*pfIsoDB04.sumPUPt)
      ) / patMu.pt();
  return isWZTightMuonNoIso(patMu, pv) && muIso < 0.15;
}


bool PATMuonIDEmbedder::isWZTightMuonNoIso(const Muon& patMu, const reco::Vertex& pv)
{
  return patMu.isTightMuon(pv) &&
    std::abs(patMu.innerTrack()->dxy(pv.position())) < 0.02 &&
    std::abs(patMu.innerTrack()->dz(pv.position())) < 0.1;
}


bool PATMuonIDEmbedder::isWZMediumMuon(const Muon& patMu, const reco::Vertex& pv)
{
  reco::MuonPFIsolation pfIsoDB04 = patMu.pfIsolationR04();
  float muIso = (pfIsoDB04.sumChargedHadronPt
      + std::max(0., pfIsoDB04.sumNeutralHadronEt
        + pfIsoDB04.sumPhotonEt
        - 0.5*pfIsoDB04.sumPUPt)
      ) / patMu.pt();
  return isWZTightMuonNoIso(patMu, pv) && muIso < 0.40;
}


bool PATMuonIDEmbedder::isSoftMuonICHEP(const RecoMuon & recoMu, const reco::Vertex& pv)
{
  bool soft = muon::isGoodMuon(recoMu, muon::TMOneStationTight) &&
    recoMu.innerTrack()->hitPattern().trackerLayersWithMeasurement() > 5 &&
    recoMu.innerTrack()->hitPattern().pixelLayersWithMeasurement() > 0 &&
    fabs(recoMu.innerTrack()->dxy(pv.position())) < 0.3 &&
    fabs(recoMu.innerTrack()->dz(pv.position())) < 20.;
  return soft;
}


void PATMuonIDEmbedder::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(PATMuonIDEmbedder);
