//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//    PATJetPUSFEmbedder.cc                                                 //
//                                                                          //
//    Store jet PU id SF multiplication factor in event                     //
//                                                                          //
//    Modified from PATJetIDEmbedder.cc                                     //
//    which was from Author: Nate Woods, U. Wisconsin                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/Common/interface/Association.h"
#include "DataFormats/Common/interface/Ref.h"

// ROOT includes
#include "TH2F.h"
#include "TFile.h"

typedef pat::Jet Jet;
typedef std::vector<Jet> VJet;
typedef edm::View<Jet> JetView;

class PATJetPUSFEmbedder : public edm::stream::EDProducer<>
{
public:
  explicit PATJetPUSFEmbedder(const edm::ParameterSet &pset);
  virtual ~PATJetPUSFEmbedder() { ; }

private:
  virtual void produce(edm::Event &iEvent, const edm::EventSetup &iSetup);

  typedef edm::Association<reco::GenJetCollection> MatchMap;
  edm::EDGetTokenT<JetView> srcToken;
  edm::EDGetTokenT<MatchMap> matchToken_;
  std::unique_ptr<TH2F> jetPUSF_;
  std::unique_ptr<TH2F> jetPUeff_;
  std::unique_ptr<TFile> sfFile_;
  std::unique_ptr<TFile> effFile_;
  bool domatch_;
  std::string sfFileN_;
  std::string effFileN_;
  const int setup_;
  int PUid;
  int evtcount = 0;
  int jetcount = 0;
};

PATJetPUSFEmbedder::PATJetPUSFEmbedder(const edm::ParameterSet &pset) : srcToken(consumes<JetView>(pset.getParameter<edm::InputTag>("src"))),
                                                                    matchToken_(consumes<MatchMap>(edm::InputTag("patJetGenJetMatch2"))),
                                                                    domatch_(pset.exists("domatch") ? pset.getParameter<bool>("domatch") : false),
                                                                    sfFileN_(pset.exists("jsfFile") ? pset.getParameter<std::string>("jsfFile") : "sfFileNone"),
                                                                    effFileN_(pset.exists("jeffFile") ? pset.getParameter<std::string>("jeffFile") : "sfFileNone"),
                                                                    // Which year JET ID we need
                                                                    setup_(pset.exists("setup") ? pset.getParameter<int>("setup") : 2022)
{
  std::string notSF = "sfFileNone";
  if (domatch_ && sfFileN_ != notSF) // if sfFile and effFile names are input, also the hists name should be there
  {

    std::string baseName = sfFileN_;
    std::ifstream checkfile(baseName);
    if (!checkfile.good())
      baseName = baseName.substr(baseName.find("UWVV/") + 5);
    sfFile_ = std::unique_ptr<TFile>(new TFile(baseName.c_str()));
    jetPUSF_ = std::unique_ptr<TH2F>((sfFile_->IsOpen() && !sfFile_->IsZombie()) ? (TH2F *)(sfFile_->Get(pset.getParameter<std::string>("SFhistName").c_str())->Clone()) : new TH2F("h", "h", 1, 0., 1., 1, 0., 1.));
    if (sfFile_->IsZombie())
      throw cms::Exception("InvalidFile")
          << "Scale factor file " << sfFileN_
          << " does not exist!" << std::endl;

    std::string baseName2 = effFileN_;
    std::ifstream checkfile2(baseName2);
    if (!checkfile2.good())
      baseName2 = baseName2.substr(baseName2.find("UWVV/") + 5);
    effFile_ = std::unique_ptr<TFile>(new TFile(baseName2.c_str()));
    jetPUeff_ = std::unique_ptr<TH2F>((effFile_->IsOpen() && !effFile_->IsZombie()) ? (TH2F *)(effFile_->Get(pset.getParameter<std::string>("effhistName").c_str())->Clone()) : new TH2F("h", "h", 1, 0., 1., 1, 0., 1.));
    if (effFile_->IsZombie())
      throw cms::Exception("InvalidFile")
          << "eff file " << effFileN_
          << " does not exist!" << std::endl;
  }

  //if (domatch_ && sfFileN_ != notSF){
  //produces<VJet>("normaljet");}
  //else{
  //produces<VJet>();
  //}
  if (domatch_ && sfFileN_ != notSF){
    produces<float>("jetPUSFmulfac");
  }
}

void PATJetPUSFEmbedder::produce(edm::Event &iEvent,
                               const edm::EventSetup &iSetup)
{
  // evtcount++;
  // printf("====================RECO vs Gen jet Information=========================================\n");
  // printf("evt#   pt     eta    phi    pt     eta    phi    PUid0 PUidnew jet#\n");
  // jetcount = 0;
  float weight = 1.; // mult factor for jet PU id SF correction
  std::string notSF = "sfFileNone";
  edm::Handle<JetView> in;
  iEvent.getByToken(srcToken, in);
  edm::Handle<MatchMap> match;
  if (domatch_)
  {
    iEvent.getByToken(matchToken_, match);
  }

  std::unique_ptr<VJet> out(new VJet());

  for (size_t i = 0; i < in->size(); ++i)
  {
    out->push_back(in->at(i)); // copies, transfers ownership

    Jet &jet = out->back();

    if (domatch_)
    {
      // jetcount++;
      edm::Ref<JetView> jetRef(in, i);
      const auto genMatched = (*match)[jetRef];
      // PUid = jetRef->userInt("pileupJetIdUpdated:fullId");
      PUid = jet.userInt("pileupJetIdUpdated:fullId"); // can use jetRef, but just in case
      if (genMatched.isNonnull())
      {
        // printf("%3d %7.2f %6.2f %6.2f %7.2f %6.2f %6.2f %5d %5d %7d\n",
        // evtcount, jetRef->pt(), jetRef->eta(), jetRef->phi(), genMatched->pt(), genMatched->eta(), genMatched->phi(),jetRef->userInt("pileupJetId:fullId"), PUid, jetcount);
        //jet.addUserFloat("genjetMatched", 1.);

        if (jet.pt() < 50 && sfFileN_ != notSF)
        {
          auto jetbin = jetPUSF_->FindBin(jet.pt(), jet.eta());
          float jetPUSF = (float)jetPUSF_->GetBinContent(jetbin);
          auto jeffbin = jetPUeff_->FindBin(jet.pt(), jet.eta());
          float jeffPU = (float)jetPUeff_->GetBinContent(jeffbin);
          float mulfac = 1.;

          if (PUid < 7)
          { // doesn't pass PU id
            mulfac = (1. - jetPUSF * jeffPU) / (1. - jeffPU);
          }
          else
          {
            mulfac = jetPUSF;
          }
          weight *= mulfac;
        }
      }
      else
      {
        // printf("%3d %7.2f %6.2f %6.2f %7.2f %6.2f %6.2f %5d %5d %7d\n",
        // evtcount, jetRef->pt(), jetRef->eta(), jetRef->phi(), -1.,-1.,-1.,jetRef->userInt("pileupJetId:fullId"), PUid, jetcount);
        //jet.addUserFloat("genjetMatched", 0.);
      }
    }
  }

  //if (domatch_ && sfFileN_ != notSF){
  //iEvent.put(std::move(out), "normaljet");}
  //else{
  //iEvent.put(std::move(out));
  //}

  std::unique_ptr<float> putweight(new float(weight)); 
  if (domatch_ && sfFileN_ != notSF){
    iEvent.put(std::move(putweight),"jetPUSFmulfac");
  }
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(PATJetPUSFEmbedder);
