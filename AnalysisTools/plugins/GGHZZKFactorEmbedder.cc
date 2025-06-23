///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//    GGHZZKFactorEmbedder                                                   //
//                                                                           //
//    Given a collection of pat::CompositeCandidates made of two two-object  //
//    CompositeCandidates, embeds some information about the alternate       //
//    dilepton pairings.                                                     //
//                                                                           //
//    Nate Woods, U. Wisconsin                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


// system includes
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <map>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/Common/interface/View.h"
#include "DataFormats/Math/interface/deltaR.h"

// ROOT includes
#include "TSpline.h"
#include "TFile.h"

typedef pat::CompositeCandidate CCand;


template<class T12, class T34>
class GGHZZKFactorEmbedder : public edm::stream::EDProducer<>
{

 public:
  explicit GGHZZKFactorEmbedder(const edm::ParameterSet& iConfig);
  virtual ~GGHZZKFactorEmbedder() {};

 private:
  virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);

  const edm::EDGetTokenT<edm::View<CCand> > srcToken;

  const std::unique_ptr<TFile> file;

  const std::map<std::string, TSpline3*> splines;

  std::map<std::string, TSpline3*> getSplines() const;
  float getGenMass(const CCand& cand) const;
};


template<class T12, class T34>
GGHZZKFactorEmbedder<T12,T34>::GGHZZKFactorEmbedder(const edm::ParameterSet& iConfig) :
  srcToken(consumes<edm::View<CCand> >(iConfig.getParameter<edm::InputTag>("src"))),
  file(new TFile(iConfig.getParameter<std::string>("fileName").c_str())),
  splines(getSplines())
{
  if(file->IsZombie())
    throw cms::Exception("InvalidFile")
      << "Scale factor file "<< iConfig.getParameter<std::string>("fileName")
      << " does not exist!" << std::endl;

  produces<std::vector<CCand> >();
}


template<class T12, class T34>
void GGHZZKFactorEmbedder<T12,T34>::produce(edm::Event& iEvent,
                                            const edm::EventSetup& iSetup)
{
  edm::Handle<edm::View<CCand> > in;
  std::unique_ptr<std::vector<CCand> > out(new std::vector<CCand>);

  iEvent.getByToken(srcToken, in);

  for(size_t i = 0; i < in->size(); ++i)
    {
      out->push_back(in->at(i));

      CCand& cand = out->back(); // for convenience

      float mGen = getGenMass(cand);

      for(auto& iSpl : splines)
        {
          float kFactor = 1.;
          if(mGen > 0.)
            kFactor = iSpl.second->Eval(mGen);

          cand.addUserFloat(iSpl.first, kFactor);
        }
    }

  iEvent.put(std::move(out));
}


template<class T12, class T34>
std::map<std::string, TSpline3*> GGHZZKFactorEmbedder<T12,T34>::getSplines() const
{
  std::map<std::string, TSpline3*> out;

  if(file->IsZombie() || !file->IsOpen())
    return out;

  out["kFactor"] = (TSpline3*)file->Get("sp_kfactor_Nominal");
  out["kFactorPDFScaleUp"] = (TSpline3*)file->Get("sp_kfactor_PDFScaleUp");
  out["kFactorPDFScaleDn"] = (TSpline3*)file->Get("sp_kfactor_PDFScaleDn");
  out["kFactorQCDScaleUp"] = (TSpline3*)file->Get("sp_kfactor_QCDScaleUp");
  out["kFactorQCDScaleDn"] = (TSpline3*)file->Get("sp_kfactor_QCDScaleDn");
  out["kFactorAsUp"] = (TSpline3*)file->Get("sp_kfactor_AsUp");
  out["kFactorAsDn"] = (TSpline3*)file->Get("sp_kfactor_AsDn");
  out["kFactorPDFReplicaUp"] = (TSpline3*)file->Get("sp_kfactor_PDFReplicaUp");
  out["kFactorPDFReplicaDn"] = (TSpline3*)file->Get("sp_kfactor_PDFReplicaDn");

  return out;
}


template<class T12, class T34>
float GGHZZKFactorEmbedder<T12,T34>::getGenMass(const CCand& cand) const
{
  if(cand.numberOfDaughters() < 2 ||
     cand.daughter(0)->numberOfDaughters() < 2 ||
     cand.daughter(1)->numberOfDaughters() < 2)
    return -1.;

  math::XYZTLorentzVector genP4(0.,0.,0.,0.);
  for(size_t iZ = 0; iZ < cand.numberOfDaughters(); ++iZ)
    {
      for(size_t iLep = 0;
          iLep < cand.daughter(iZ)->numberOfDaughters();
          ++iLep)
        {
          reco::GenParticleRef gen;
          const reco::Candidate* uncasted = cand.daughter(iZ)->daughter(iLep);
          if(iZ == 0)
            {
              const T12* p = static_cast<const T12*>(uncasted->masterClone().get());
              gen = p->genParticleRef();
            }
          else
            {
              const T34* p = static_cast<const T34*>(uncasted->masterClone().get());
              gen = p->genParticleRef();
            }

          if(gen.isNull())
            return -1.;

          genP4 += gen->p4();
        }
    }

  return genP4.mass();
}


typedef GGHZZKFactorEmbedder<pat::Electron,pat::Electron> GGHZZKFactorEmbedderEEEE;
typedef GGHZZKFactorEmbedder<pat::Electron,pat::Muon> GGHZZKFactorEmbedderEEMuMu;
typedef GGHZZKFactorEmbedder<pat::Muon,pat::Muon> GGHZZKFactorEmbedderMuMuMuMu;

DEFINE_FWK_MODULE(GGHZZKFactorEmbedderEEEE);
DEFINE_FWK_MODULE(GGHZZKFactorEmbedderEEMuMu);
DEFINE_FWK_MODULE(GGHZZKFactorEmbedderMuMuMuMu);
