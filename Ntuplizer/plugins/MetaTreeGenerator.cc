/////////////////////////////////////////////////////////////////////////////    
//                                                                         //    
//    MetaTreeGenerator                                                    //    
//                                                                         //    
//    A builder of meta-info ntuples                                       //
//                                                                         //    
//    Nate Woods, U. Wisconsin                                             //    
//                                                                         //    
/////////////////////////////////////////////////////////////////////////////    


//STL
#include <memory>

// CMSSW
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

// ROOT
#include "TTree.h"
#include "TDirectory.h"
#include "TObjString.h"

// UWVV
#include "UWVV/Ntuplizer/interface/EventInfo.h"


using namespace uwvv;

class MetaTreeGenerator : public edm::one::EDAnalyzer<edm::one::SharedResources, 
                                                      edm::one::WatchLuminosityBlocks>
{
 public:
  explicit MetaTreeGenerator(const edm::ParameterSet&);
  virtual ~MetaTreeGenerator() {;}

 private:
  virtual void analyze(const edm::Event& iEvent, 
                       const edm::EventSetup& iConfig) override;
  virtual void beginLuminosityBlock(const edm::LuminosityBlock& iLumi,
                                    const edm::EventSetup& iSetup);
  virtual void endLuminosityBlock(const edm::LuminosityBlock& iLumi,
                                  const edm::EventSetup& iSetup);

  TTree* const makeTree();

  TTree* const tree;
  EventInfo evtInfo;
  const std::string datasetName;

  unsigned runBranch;
  unsigned lumiBranch;
  unsigned neventsBranch;
  float summedWeightsBranch;
};


MetaTreeGenerator::MetaTreeGenerator(const edm::ParameterSet& config) :
  tree(makeTree()),
  evtInfo(consumesCollector(), config.getParameter<edm::ParameterSet>("eventParams")),
  datasetName(config.exists("datasetName") ?
             config.getParameter<std::string>("datasetName") : "unknown"),
  runBranch(0),
  lumiBranch(0),
  neventsBranch(0),
  summedWeightsBranch(0.)
{
  usesResource("TFileService");
  edm::Service<TFileService> FS;
  auto dir = FS->mkdir("datasetName");
  dir.make<TObjString>(datasetName.c_str());
}


TTree* const MetaTreeGenerator::makeTree()
{
  edm::Service<TFileService> FS;

  TTree* const t = FS->make<TTree>("metaInfo", "metaInfo");

  t->Branch("run", &runBranch);
  t->Branch("lumi", &lumiBranch);
  t->Branch("nevents", &neventsBranch);
  t->Branch("summedWeights", &summedWeightsBranch);
  
  return t;
}


void
MetaTreeGenerator::beginLuminosityBlock(const edm::LuminosityBlock& iLumi,
                                        const edm::EventSetup& iSetup)
{
  runBranch = iLumi.run();
  lumiBranch = iLumi.luminosityBlock();
  neventsBranch = 0;
  summedWeightsBranch = 0.;
}


void
MetaTreeGenerator::endLuminosityBlock(const edm::LuminosityBlock& iLumi,
                                      const edm::EventSetup& iSetup)
{
  tree->Fill();
}


void MetaTreeGenerator::analyze(const edm::Event &event,
                                const edm::EventSetup &setup)
{
  evtInfo.setEvent(event);

  ++neventsBranch;
  summedWeightsBranch += (evtInfo.genEventInfo().isValid() ? 
                          evtInfo.genEventInfo()->weight() :
                          0.);
}


#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_FWK_MODULE(MetaTreeGenerator);
