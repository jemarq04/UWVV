#include "UWVV/Ntuplizer/interface/EventInfo.h"


using namespace uwvv;

template<class T>
EventInfoHolder<T>::EventInfoHolder(edm::ConsumesCollector& cc,
                                    const edm::InputTag& primaryTag,
                                    const edm::ParameterSet& moreTags) :
  primary_(setupData(cc,primaryTag,moreTags))
{
}


template<class T>
DatumPtr<T>&
EventInfoHolder<T>::setupData(edm::ConsumesCollector& cc,
                              const edm::InputTag& primaryTag,
                              const edm::ParameterSet& moreTags)
{
  data_ = std::map<std::string, DatumPtr<T> >();
  data_[""] = std::make_unique<EventDatum<T> >(cc, primaryTag);

  for(auto&& collection : moreTags.getParameterNames())
    data_[collection] = std::make_unique<EventDatum<T> >(cc, moreTags.getParameter<edm::InputTag>(collection));

  return data_[""];
}


template<class T>
void EventInfoHolder<T>::setEvent(const edm::Event& event)
{
  for(auto&& d : data_)
    d.second->setEvent(event);
}


EventInfo::EventInfo(edm::ConsumesCollector cc,
                     const edm::ParameterSet& config) :
  vertices_(cc, config.getParameter<edm::InputTag>("vtxSrc"),
            config.exists("vtxExtra") ?
            config.getParameter<edm::ParameterSet>("vtxExtra") :
            edm::ParameterSet()),
  electrons_(cc, config.getParameter<edm::InputTag>("eSrc"),
             config.exists("eExtra") ?
             config.getParameter<edm::ParameterSet>("eExtra") :
             edm::ParameterSet()),
  muons_(cc, config.getParameter<edm::InputTag>("mSrc"),
         config.exists("mExtra") ?
         config.getParameter<edm::ParameterSet>("mExtra") :
         edm::ParameterSet()),
  taus_(cc, config.getParameter<edm::InputTag>("tSrc"),
        config.exists("tExtra") ?
        config.getParameter<edm::ParameterSet>("tExtra") :
        edm::ParameterSet()),
  photons_(cc, config.getParameter<edm::InputTag>("gSrc"),
           config.exists("gExtra") ?
           config.getParameter<edm::ParameterSet>("gExtra") :
           edm::ParameterSet()),
  jets_(cc, config.getParameter<edm::InputTag>("jSrc"),
        config.exists("jExtra") ?
        config.getParameter<edm::ParameterSet>("jExtra") :
        edm::ParameterSet()),
  pfCands_(cc, config.getParameter<edm::InputTag>("pfCandSrc"),
           config.exists("pfCandExtra") ?
           config.getParameter<edm::ParameterSet>("pfCandExtra") :
           edm::ParameterSet()),
  mets_(cc, config.getParameter<edm::InputTag>("metSrc"),
        config.exists("metExtra") ?
        config.getParameter<edm::ParameterSet>("metExtra") :
        edm::ParameterSet()),
  puInfo_(cc, config.getParameter<edm::InputTag>("puSrc"),
          config.exists("puExtra") ?
          config.getParameter<edm::ParameterSet>("puExtra") :
          edm::ParameterSet()),
  genEventInfo_(cc, config.getParameter<edm::InputTag>("genEventInfoSrc"),
                config.exists("genEventInfoExtra") ?
                config.getParameter<edm::ParameterSet>("genEventInfoExtra") :
                edm::ParameterSet()),
  lheEventInfo_(cc, config.getParameter<edm::InputTag>("lheEventInfoSrc"),
                config.exists("lheEventInfoExtra") ?
                config.getParameter<edm::ParameterSet>("lheEventInfoExtra") :
                edm::ParameterSet()),
  //l1ECALPrefiring
  prefweight_(cc, config.getParameter<edm::InputTag>("prefweight"),
          config.exists("prefweightExtra") ?
          config.getParameter<edm::ParameterSet>("prefweightExtra") :
          edm::ParameterSet()),
  prefweightup_(cc, config.getParameter<edm::InputTag>("prefweightup"),
          config.exists("prefweightupExtra") ?
          config.getParameter<edm::ParameterSet>("prefweightupExtra") :
          edm::ParameterSet()),
  prefweightdown_(cc, config.getParameter<edm::InputTag>("prefweightdown"),
          config.exists("prefweightdownExtra") ?
          config.getParameter<edm::ParameterSet>("prefweightdownExtra") :
          edm::ParameterSet()),
  genJets_(cc, config.getParameter<edm::InputTag>("genJetSrc"),
           config.exists("genJetExtra") ?
           config.getParameter<edm::ParameterSet>("genJetExtra") :
           edm::ParameterSet()),
  genParticles_(cc, config.getParameter<edm::InputTag>("genParticleSrc"),
                config.exists("genParticleExtra") ?
                config.getParameter<edm::ParameterSet>("genParticleExtra") :
                edm::ParameterSet()),
  initialStates_(cc, config.getParameter<edm::InputTag>("initialStateSrc"),
                 config.exists("initialStateExtra") ?
                 config.getParameter<edm::ParameterSet>("initialStateExtra") :
                 edm::ParameterSet()),
  genInitialStates_(cc, config.getParameter<edm::InputTag>("genInitialStateSrc"),
                    config.exists("genInitialStateExtra") ?
                    config.getParameter<edm::ParameterSet>("genInitialStateExtra") :
                    edm::ParameterSet())
{
}


void EventInfo::setEvent(const edm::Event& event)
{
  vertices_.setEvent(event);
  electrons_.setEvent(event);
  muons_.setEvent(event);
  taus_.setEvent(event);
  photons_.setEvent(event);
  jets_.setEvent(event);
  pfCands_.setEvent(event);
  mets_.setEvent(event);
  puInfo_.setEvent(event);
  genEventInfo_.setEvent(event);
  lheEventInfo_.setEvent(event);
  prefweight_.setEvent(event);
  prefweightup_.setEvent(event);
  prefweightdown_.setEvent(event);
  genJets_.setEvent(event);
  genParticles_.setEvent(event);
  initialStates_.setEvent(event);
  genInitialStates_.setEvent(event);

  currentEvent_ = &event;
}
