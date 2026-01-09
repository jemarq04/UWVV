#ifndef UWVV_Ntuplizer_EventInfo_h
#define UWVV_Ntuplizer_EventInfo_h

#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Provenance/interface/EventID.h"
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"



namespace uwvv
{

  template<class T> class EventDatum
  {
   public:
    EventDatum(edm::ConsumesCollector& cc, const edm::InputTag& tag) :
      token_(cc.consumes<T>(tag)),
      isValid_(false)
        {;}
    ~EventDatum() {;}

    void setEvent(const edm::Event& event)
    {
      currentEvent_ = &event;
      isValid_ = false;
    }

    const edm::Handle<T>& get()
    {
      if(isValid_)
        return handle_;

      currentEvent_->getByToken(token_, handle_);
      isValid_ = true;
      return handle_;
    }

   private:
    const edm::EDGetTokenT<T> token_;
    edm::Handle<T> handle_;
    const edm::Event* currentEvent_;
    bool isValid_;
  };

  template<class T> using DatumPtr = std::unique_ptr<EventDatum<T> >;

  template<class T> class EventInfoHolder
  {
   public:
    EventInfoHolder(edm::ConsumesCollector& cc, const edm::InputTag& primaryTag,
                    const edm::ParameterSet& moreTags);
    ~EventInfoHolder() {;}

    void setEvent(const edm::Event& event);

    const edm::Handle<T>& get() {return primary_->get();}
    const edm::Handle<T>& get(const std::string& item) {return data_.at(item)->get();}

   private:
    DatumPtr<T>& setupData(edm::ConsumesCollector& cc,
                           const edm::InputTag& primaryTag,
                           const edm::ParameterSet& moreTags);

    std::map<std::string, DatumPtr<T> > data_;
    DatumPtr<T>& primary_;
  };


  class EventInfo
  {
   public:
    EventInfo(edm::ConsumesCollector cc, const edm::ParameterSet& config);
    ~EventInfo() {;}

    void setEvent(const edm::Event& event);

    const edm::EventID id() const {return currentEvent_->id();}

    const edm::Ptr<reco::Vertex> pv()
    {
      return (nVertices() ? vertices()->ptrAt(0) : edm::Ptr<reco::Vertex>(NULL, 0));
    }
    size_t nVertices() {return vertices()->size();}
    size_t nVertices(const std::string& collection) {return vertices(collection)->size();}
    const edm::Handle<edm::View<reco::Vertex> >& vertices() {return vertices_.get();}
    const edm::Handle<edm::View<reco::Vertex> >& vertices(const std::string& collection) {return vertices_.get(collection);}
    const edm::Handle<edm::View<pat::Electron> >& electrons() {return electrons_.get();}
    const edm::Handle<edm::View<pat::Electron> >& electrons(const std::string& collection) {return electrons_.get(collection);}
    const edm::Handle<edm::View<pat::Muon> >& muons() {return muons_.get();}
    const edm::Handle<edm::View<pat::Muon> >& muons(const std::string& collection) {return muons_.get(collection);}
    const edm::Handle<edm::View<pat::Tau> >& taus() {return taus_.get();}
    const edm::Handle<edm::View<pat::Tau> >& taus(const std::string& collection) {return taus_.get(collection);}
    const edm::Handle<edm::View<pat::Photon> >& photons() {return photons_.get();}
    const edm::Handle<edm::View<pat::Photon> >& photons(const std::string& collection) {return photons_.get(collection);}
    const edm::Handle<edm::View<pat::Jet> >& jets() {return jets_.get();}
    const edm::Handle<edm::View<pat::Jet> >& jets(const std::string& collection) {return jets_.get(collection);}
    const edm::Handle<edm::View<pat::PackedCandidate> >& pfCands() {return pfCands_.get();}
    const edm::Handle<edm::View<pat::PackedCandidate> >& pfCands(const std::string& collection) {return pfCands_.get(collection);}
    const pat::MET& met() {return mets_.get()->front();}
    const pat::MET& met(const std::string& collection) {return mets_.get(collection)->front();}
    const edm::Handle<pat::METCollection>& mets() {return mets_.get();}
    const edm::Handle<pat::METCollection>& mets(const std::string& which) {return mets_.get(which);}
    const edm::Handle<std::vector<PileupSummaryInfo> >& puInfo() {return puInfo_.get();}
    const edm::Handle<std::vector<PileupSummaryInfo> >& puInfo(const std::string& collection) {return puInfo_.get(collection);}
    const edm::Handle<GenEventInfoProduct>& genEventInfo() {return genEventInfo_.get();}
    const edm::Handle<GenEventInfoProduct>& genEventInfo(const std::string& collection) {return genEventInfo_.get(collection);}
    const edm::Handle<LHEEventProduct>& lheEventInfo() {return lheEventInfo_.get();}
    const edm::Handle<LHEEventProduct>& lheEventInfo(const std::string& collection) {return lheEventInfo_.get(collection);}

    const edm::Handle<double>& prefweight(){return prefweight_.get();};
    const edm::Handle<double>& prefweightup() {return prefweightup_.get();}
    const edm::Handle<double>& prefweightdown() {return prefweightdown_.get();}

    const edm::Handle<edm::View<reco::GenJet> >& genJets() {return genJets_.get();}
    const edm::Handle<edm::View<reco::GenJet> >& genJets(const std::string& collection) {return genJets_.get(collection);}
    const edm::Handle<edm::View<reco::GenParticle> >& genParticles() {return genParticles_.get();}
    const edm::Handle<edm::View<reco::GenParticle> >& genParticles(const std::string& collection) {return genParticles_.get(collection);}
    const edm::Handle<edm::View<pat::CompositeCandidate> >& initialStates() {return initialStates_.get();}
    const edm::Handle<edm::View<pat::CompositeCandidate> >& initialStates(const std::string& collection) {return initialStates_.get(collection);}
    const edm::Handle<edm::View<pat::CompositeCandidate> >& genInitialStates() {return genInitialStates_.get();}
    const edm::Handle<edm::View<pat::CompositeCandidate> >& genInitialStates(const std::string& collection) {return genInitialStates_.get(collection);}

   private:
    const edm::Event* currentEvent_;
    EventInfoHolder<edm::View<reco::Vertex> > vertices_;
    EventInfoHolder<edm::View<pat::Electron> > electrons_;
    EventInfoHolder<edm::View<pat::Muon> > muons_;
    EventInfoHolder<edm::View<pat::Tau> > taus_;
    EventInfoHolder<edm::View<pat::Photon> > photons_;
    EventInfoHolder<edm::View<pat::Jet> > jets_;
    EventInfoHolder<edm::View<pat::PackedCandidate> > pfCands_;
    EventInfoHolder<pat::METCollection> mets_;
    EventInfoHolder<std::vector<PileupSummaryInfo> > puInfo_;
    EventInfoHolder<GenEventInfoProduct> genEventInfo_;
    EventInfoHolder<LHEEventProduct> lheEventInfo_;
    EventInfoHolder< double > prefweight_;
    EventInfoHolder< double > prefweightup_;
    EventInfoHolder< double > prefweightdown_;
    EventInfoHolder<edm::View<reco::GenJet> > genJets_;
    EventInfoHolder<edm::View<reco::GenParticle> > genParticles_;
    EventInfoHolder<edm::View<pat::CompositeCandidate> > initialStates_;
    EventInfoHolder<edm::View<pat::CompositeCandidate> > genInitialStates_;
  };

} // namespace

#endif // header guard
