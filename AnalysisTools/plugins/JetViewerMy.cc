// -*- C++ -*-
//
// Package:    demo/DemoA
// Class:      DemoA
//
/**\class DemoA DemoA.cc demo/DemoA/plugins/DemoA.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  He He
//         Created:  Wed, 20 May 2020 14:34:07 GMT
//
//


// system include files
#include <memory>
#include<string>
#include<vector>
#include<stdio.h>


#include "FWCore/Framework/interface/EventSetup.h"


#include "DataFormats/PatCandidates/interface/Jet.h"

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.


//using reco::TrackCollection;
typedef pat::Jet Jet;
typedef edm::View<Jet> JetView;

class JetViewerMy : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit JetViewerMy(const edm::ParameterSet&);
      ~JetViewerMy();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


   private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
      edm::EDGetTokenT<JetView> srcToken_;  //used to select what jet to read from configuration file
  int PUid;
  int evtcount=0;
  int jetcount=0;
  std::string jetTag_;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
JetViewerMy::JetViewerMy(const edm::ParameterSet& iConfig)
 :
  srcToken_(consumes<JetView>(iConfig.getParameter<edm::InputTag>("src"))),
  jetTag_(iConfig.getParameter<std::string>("tag" ))

{
   //now do what ever initialization is needed

}


JetViewerMy::~JetViewerMy()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called for each event  ------------
void
JetViewerMy::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   evtcount++;
    Handle<JetView> jets;
    iEvent.getByToken(srcToken_, jets);
    printf("====================InputTag(/Stage) %s=========================================\n",jetTag_.c_str());
    printf("evt#   pt     eta    phi    PUid0 PUidnew jet#\n");
    jetcount=0;       
    for(JetView::const_iterator jet = jets->begin();
        jet != jets->end();
        ++jet) {
      
      jetcount++;
      try{
      PUid=jet->userInt("pileupJetIdUpdated:fullId");
      printf("%3d %7.2f %6.2f %6.2f %5d %5d %7d\n",evtcount,jet->pt(),jet->eta(),jet->phi(),jet->userInt("pileupJetId:fullId"),PUid,jetcount);}
      catch (...){
	printf("%3d %7.2f %6.2f %6.2f %5d %5d %7d\n",evtcount,jet->pt(),jet->eta(),jet->phi(),jet->userInt("pileupJetId:fullId"),-1,jetcount);
      }

      // int charge = itTrack->charge();
    }


#ifdef THIS_IS_AN_EVENT_EXAMPLE
   Handle<ExampleData> pIn;
   iEvent.getByLabel("example",pIn);
#endif

#ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
#endif
}


// ------------ method called once each job just before starting event loop  ------------
void
JetViewerMy::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void
JetViewerMy::endJob()
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
JetViewerMy::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(JetViewerMy);
