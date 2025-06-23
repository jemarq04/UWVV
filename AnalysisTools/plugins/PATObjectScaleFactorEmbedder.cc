///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    PATObjectScaleFactorEmbedder                                           //
//                                                                           //
//    Takes a TH2F and a collection of PAT objects and embeds the            //
//    content of the bin the object would fill as a userFloat with a         //
//    specified label. If the "useError" option is used, the bin error is    //
//    also stored, with the same label except with "Error" appended.         //
//                                                                           //
//    Nate Woods, U. Wisconsin                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


// system includes
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/View.h"
#include "CommonTools/Utils/interface/StringObjectFunction.h"

// ROOT includes
#include "TH2F.h"
#include "TFile.h"


template<typename T>
class PATObjectScaleFactorEmbedder : public edm::stream::EDProducer<>
{

public:
  explicit PATObjectScaleFactorEmbedder(const edm::ParameterSet& iConfig);
  virtual ~PATObjectScaleFactorEmbedder() {};

private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  const edm::EDGetTokenT<edm::View<T> > srcToken;
  std::unique_ptr<TFile> file;
  std::unique_ptr<TH2F> h;
  const std::string label;
  const bool useError;

  StringObjectFunction<T> xFunction;
  StringObjectFunction<T> yFunction;
};


template<typename T>
PATObjectScaleFactorEmbedder<T>::PATObjectScaleFactorEmbedder(const edm::ParameterSet& iConfig) :
  srcToken(consumes<edm::View<T> >(iConfig.getParameter<edm::InputTag>("src"))),
  label(iConfig.getParameter<std::string>("label")),
  useError(iConfig.exists("useError") &&
           iConfig.getParameter<bool>("useError")),
  xFunction(iConfig.exists("xValue") ?
            iConfig.getParameter<std::string>("xValue") :
            "eta"),
  yFunction(iConfig.exists("yValue") ?
            iConfig.getParameter<std::string>("yValue") :
            "pt")
{
  std::string baseName = iConfig.getParameter<std::string>("fileName");

  // For crab submission, the data directory will be copied over without
  // the UWVV base directory. In this case we also check in this path
  // if the original file path isn't found
  std::ifstream checkfile(baseName);
  if (!checkfile.good())
    baseName = baseName.substr(baseName.find("UWVV/")+5);
  file = std::unique_ptr<TFile>(new TFile(baseName.c_str()));
  h = std::unique_ptr<TH2F>((file->IsOpen() && !file->IsZombie()) ?
      (TH2F*)(file->Get(iConfig.getParameter<std::string>("histName").c_str())->Clone()) :
      new TH2F("h","h",1,0.,1.,1,0.,1.));
  if(file->IsZombie())
    throw cms::Exception("InvalidFile")
      << "Scale factor file "<< iConfig.getParameter<std::string>("fileName")
      << " does not exist!" << std::endl;

  produces<std::vector<T> >();
}


template<typename T>
void PATObjectScaleFactorEmbedder<T>::produce(edm::Event& iEvent,
                                              const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<T> > in;
  std::unique_ptr<std::vector<T> > out(new std::vector<T>);

  iEvent.getByToken(srcToken, in);

  for(size_t i = 0; i < in->size(); ++i)
    {
      const T& t = in->at(i);

      float x = xFunction(t);
      float y = yFunction(t);

      // don't just give 0 for under/overflow
      int bin = h->FindBin(x, y);
      if(h->IsBinOverflow(bin))
        {
          int binx, biny, binz;
          h->GetBinXYZ(bin, binx, biny, binz);
          if(binx > h->GetNbinsX())
            binx -= 1;
          if(biny > h->GetNbinsY())
            biny -= 1;

          bin = h->GetBin(binx, biny, binz);
        }
      if(h->IsBinUnderflow(bin))
        {
          int binx, biny, binz;
          h->GetBinXYZ(bin, binx, biny, binz);
          if(!binx)
            binx += 1;
          if(!biny)
            biny += 1;

          bin = h->GetBin(binx, biny, binz);
        }

      float value = h->GetBinContent(bin);
      float error = h->GetBinError(bin);

      out->push_back(t); // copies with ownership
      out->back().addUserFloat(label, value);
      if(useError)
        out->back().addUserFloat(label+"Error", error);
    }

  iEvent.put(std::move(out));
}


typedef PATObjectScaleFactorEmbedder<pat::Electron> PATElectronScaleFactorEmbedder;
typedef PATObjectScaleFactorEmbedder<pat::Muon> PATMuonScaleFactorEmbedder;
typedef PATObjectScaleFactorEmbedder<pat::Tau> PATTauScaleFactorEmbedder;
typedef PATObjectScaleFactorEmbedder<pat::Jet> PATJetScaleFactorEmbedder;
typedef PATObjectScaleFactorEmbedder<pat::CompositeCandidate> PATCompositeCandidateScaleFactorEmbedder;

DEFINE_FWK_MODULE(PATElectronScaleFactorEmbedder);
DEFINE_FWK_MODULE(PATMuonScaleFactorEmbedder);
DEFINE_FWK_MODULE(PATTauScaleFactorEmbedder);
DEFINE_FWK_MODULE(PATJetScaleFactorEmbedder);
DEFINE_FWK_MODULE(PATCompositeCandidateScaleFactorEmbedder);
