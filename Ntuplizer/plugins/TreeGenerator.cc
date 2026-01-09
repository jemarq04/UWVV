/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//    TreeGenerator                                                        //
//                                                                         //
//    A builder of ntuples from composite candidates.                      //
//                                                                         //
//    Nate Woods, U. Wisconsin                                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


//STL
#include <memory>
#include <type_traits>

// CMSSW
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

// ROOT
#include "TTree.h"

// UWVV
#include "UWVV/Ntuplizer/interface/BranchManager.h"
#include "UWVV/Ntuplizer/interface/EventInfo.h"
#include "UWVV/Ntuplizer/interface/TriggerBranches.h"
#include "UWVV/DataFormats/interface/DressedGenParticle.h"


using namespace uwvv;

template<class T>
class TreeGenerator : public edm::one::EDAnalyzer<edm::one::SharedResources>
{
  // If this is a particle candidate, we can make branches directly from it
  // Otherwise, assume it specifies and composite candidate
  typedef typename std::conditional<std::is_base_of<reco::Candidate, T>::value,
                                    T, pat::CompositeCandidate>::type Cand;

 public:
  explicit TreeGenerator(const edm::ParameterSet&);
  virtual ~TreeGenerator() {;}

 private:
  virtual void analyze(edm::Event const& iEvent, edm::EventSetup const& iConfig) override;

  TTree* const makeTree() const;

  const edm::EDGetTokenT<edm::View<Cand> > candToken;

  const std::string ntupleName;

  TTree* const tree;
  EventInfo evtInfo;

  std::unique_ptr<BranchManager<T> > branches;
  std::unique_ptr<TriggerBranches> filterBranches;
  std::unique_ptr<TriggerBranches> triggerBranches;
};


template<class T>
TreeGenerator<T>::TreeGenerator(const edm::ParameterSet& config) :
  candToken(consumes<edm::View<Cand> >(config.getParameter<edm::InputTag>("src"))),
  ntupleName(config.exists("ntupleName") ?
             config.getParameter<std::string>("ntupleName") : "ntuple"),
  tree(makeTree()),
  evtInfo(consumesCollector(), config.getParameter<edm::ParameterSet>("eventParams"))
{
  usesResource("TFileService");

  const edm::ParameterSet& branchParams = config.getParameter<edm::ParameterSet>("branches");
  branches =
    std::unique_ptr<BranchManager<T> >(new BranchManager<T>("", tree, branchParams));

  const edm::ParameterSet& triggers = config.getParameter<edm::ParameterSet>("triggers");
  triggerBranches = std::unique_ptr<TriggerBranches>(new TriggerBranches(consumesCollector(),
                                                                         triggers, tree));
  const edm::ParameterSet& filters = config.getParameter<edm::ParameterSet>("filters");
  filterBranches = std::unique_ptr<TriggerBranches>(new TriggerBranches(consumesCollector(),
                                                                         filters, tree));
}


template<class T>
TTree* const TreeGenerator<T>::makeTree() const
{
  edm::Service<TFileService> FS;

  return FS->make<TTree>(ntupleName.c_str(), ntupleName.c_str());
}


template<class T> void
TreeGenerator<T>::analyze(const edm::Event &event,
                          const edm::EventSetup &setup)
{

  edm::Handle<edm::View<Cand> > cands;
  event.getByToken(candToken, cands);

  evtInfo.setEvent(event);
  triggerBranches->setEvent(event);
  filterBranches->setEvent(event);

  for(size_t i = 0; i < cands->size(); ++i)
    {
      branches->fill(cands->ptrAt(i), evtInfo);
      triggerBranches->fill();
      filterBranches->fill();

      tree->Fill();
    }
}


typedef TreeGenerator<CompositeDaughter<CompositeDaughter<pat::Electron, pat::Electron>,
                                        CompositeDaughter<pat::Electron, pat::Electron>
                                        >
                      > TreeGeneratorEEEE;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<pat::Electron, pat::Electron>,
                                        CompositeDaughter<pat::Muon, pat::Muon>
                                        >
                      > TreeGeneratorEEMuMu;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<pat::Muon, pat::Muon>,
                                        CompositeDaughter<pat::Muon, pat::Muon>
                                        >
                      > TreeGeneratorMuMuMuMu;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<pat::Electron, pat::Electron>,
                                        pat::Electron
                                        >
                      > TreeGeneratorEEE;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<pat::Muon, pat::Muon>,
                                        pat::Muon
                                        >
                      > TreeGeneratorMuMuMu;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<pat::Electron, pat::Electron>,
                                        pat::Muon
                                        >
                      > TreeGeneratorEEMu;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<pat::Muon, pat::Muon>,
                                        pat::Electron
                                        >
                      > TreeGeneratorEMuMu;
typedef TreeGenerator<CompositeDaughter<pat::Electron, pat::Electron> > TreeGeneratorEE;
typedef TreeGenerator<CompositeDaughter<pat::Muon, pat::Muon> > TreeGeneratorMuMu;
typedef TreeGenerator<pat::Electron> TreeGeneratorE;
typedef TreeGenerator<pat::Muon> TreeGeneratorMu;

typedef TreeGenerator<CompositeDaughter<CompositeDaughter<reco::GenParticle, reco::GenParticle>,
                                        CompositeDaughter<reco::GenParticle, reco::GenParticle>
                                        >
                      > GenTreeGeneratorZZ;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<reco::GenParticle, reco::GenParticle>,
                                        reco::GenParticle
                                        >
                      > GenTreeGeneratorWZ;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<DressedGenParticle, DressedGenParticle>,
                                        CompositeDaughter<DressedGenParticle, DressedGenParticle>
                                        >
                      > GenDressedTreeGeneratorZZ;
typedef TreeGenerator<CompositeDaughter<CompositeDaughter<DressedGenParticle, DressedGenParticle>,
                                        DressedGenParticle
                                        >
                      > GenDressedTreeGeneratorWZ;


#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(TreeGeneratorEEEE);
DEFINE_FWK_MODULE(TreeGeneratorEEMuMu);
DEFINE_FWK_MODULE(TreeGeneratorMuMuMuMu);
DEFINE_FWK_MODULE(TreeGeneratorEEE);
DEFINE_FWK_MODULE(TreeGeneratorMuMuMu);
DEFINE_FWK_MODULE(TreeGeneratorEEMu);
DEFINE_FWK_MODULE(TreeGeneratorEMuMu);
DEFINE_FWK_MODULE(TreeGeneratorEE);
DEFINE_FWK_MODULE(TreeGeneratorMuMu);
DEFINE_FWK_MODULE(TreeGeneratorE);
DEFINE_FWK_MODULE(TreeGeneratorMu);

DEFINE_FWK_MODULE(GenTreeGeneratorZZ);
DEFINE_FWK_MODULE(GenDressedTreeGeneratorWZ);
DEFINE_FWK_MODULE(GenDressedTreeGeneratorZZ);
DEFINE_FWK_MODULE(GenTreeGeneratorWZ);
