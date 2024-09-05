//////////////////////////////////////////////////////////////////////////////
///                                                                        ///
///    PATObjectFSREmbedder.cc                                             ///
///                                                                        ///
///    From a collection of photons, a collection of muons, and a          ///
///        collection of electrons: make sure each photon is not in an     ///
///        electron supercluster, and pair it to its closest lepton.       ///
///        For each lepton, embed the photon with the smallest deltaR/eT   ///
///        as a usercand. Cut strings may be supplied for all three types  ///
///        of objects, and parameters for calculating isolation from       ///
///        packed PF candidates (which can also have a cut string) can be  ///
///        specified.                                                      ///
///                                                                        ///
///    Author: Nate Woods, U. Wisconsin                                    ///
///                                                                        ///
//////////////////////////////////////////////////////////////////////////////


// system include files
#include <memory>
#include <iostream>
#include <cmath> // pow, abs
#include <list>
#include <functional>
#include <utility> // pair

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Candidate/interface/Candidate.h"
#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/Common/interface/RefToPtr.h"


typedef reco::Candidate Cand;
typedef edm::Ptr<Cand> CandPtr;
typedef pat::PackedCandidate PCand;
typedef edm::Ptr<PCand> PCandPtr;
typedef edm::Ref<pat::PackedCandidateCollection> PCandRef;
typedef edm::View<PCand> PCandView;
typedef pat::Electron Elec;
typedef edm::Ptr<pat::Electron> ElecPtr;
typedef edm::View<pat::Electron> ElecView;
typedef pat::Muon Muon;
typedef edm::Ptr<pat::Muon> MuonPtr;
typedef edm::View<pat::Muon> MuonView;

class PATObjectFSREmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit PATObjectFSREmbedder(const edm::ParameterSet&);
    ~PATObjectFSREmbedder();

  private:
    virtual void produce(edm::Event&, const edm::EventSetup&);

    // check if pho is in PF supercluster of any passing electron
    bool candInSuperCluster(const PCandRef& pho,
        const edm::Handle<edm::View<Elec> >& elecs) const;

    // Compute relative isolation for pho from the cands in nIsoCands and chIsoCands
    // If those vectors are empty, they are filled from allCands
    bool passIso(const PCandRef& pho,
        std::vector<PCandRef>& nIsoCands,
        std::vector<PCandRef>& chIsoCands,
        const edm::Handle<edm::View<PCand> >& allCands) const;

    edm::EDGetTokenT<PCandView> cands_;
    edm::EDGetTokenT<ElecView> electrons_;
    edm::EDGetTokenT<MuonView> muons_;


    StringCutObjectSelector<PCand> phoESelection_;
    StringCutObjectSelector<PCand> phoMSelection_;
    StringCutObjectSelector<PCand> nIsoSelection_;
    StringCutObjectSelector<PCand> chIsoSelection_;
    StringCutObjectSelector<Elec> eSelection_;
    StringCutObjectSelector<Muon> mSelection_;

    std::string fsrLabel_;

    const float cut_; // the actual cut on deltaR/eT^n

    const float etPower_;
    const float maxDR_;

    const float isoDR_;
    const float nIsoVetoDR_;
    const float chIsoVetoDR_;
    const float relIsoCut_;

    const float eMuCrossCleaningDR_;
};


PATObjectFSREmbedder::PATObjectFSREmbedder(const edm::ParameterSet& iConfig):
  cands_(consumes<PCandView>(iConfig.getParameter<edm::InputTag>("candSrc"))),
  electrons_(consumes<ElecView>(iConfig.getParameter<edm::InputTag>("eSrc"))),
  muons_(consumes<MuonView>(iConfig.getParameter<edm::InputTag>("muSrc"))),
  phoESelection_("pdgId == 22 " +
      ((iConfig.exists("phoESelection") &&
        !iConfig.getParameter<std::string>("phoESelection").empty()) ?
       " && " + iConfig.getParameter<std::string>("phoESelection") :
       "")),
  phoMSelection_("pdgId == 22 " +
      ((iConfig.exists("phoMSelection") &&
        !iConfig.getParameter<std::string>("phoMSelection").empty()) ?
       " && " + iConfig.getParameter<std::string>("phoMSelection") :
       "")),
  nIsoSelection_("(pdgId == 22 || pdgId == 130)" +
      ((iConfig.exists("nIsoSelection") &&
        !iConfig.getParameter<std::string>("nIsoSelection").empty()) ?
       " && " + iConfig.getParameter<std::string>("nIsoSelection") :
       "")),
  chIsoSelection_("abs(pdgId) == 211" +
      ((iConfig.exists("chIsoSelection") &&
        !iConfig.getParameter<std::string>("chIsoSelection").empty()) ?
       " && " + iConfig.getParameter<std::string>("chIsoSelection") :
       "")),
  eSelection_(iConfig.exists("eSelection") ?
      iConfig.getParameter<std::string>("eSelection") :
      ""),
  mSelection_(iConfig.exists("muSelection") ?
      iConfig.getParameter<std::string>("muSelection") :
      ""),
  fsrLabel_(iConfig.exists("fsrLabel") ?
      iConfig.getParameter<std::string>("fsrLabel") :
      "dREtFSRCand"),
  cut_(iConfig.exists("cut") ?
      float(iConfig.getParameter<double>("cut")) :
      0.012), // cut on dR/eT^2 as of 21 October 2015
  etPower_(iConfig.exists("etPower") ?
      float(iConfig.getParameter<double>("etPower")) :
      1.),
  maxDR_(iConfig.exists("maxDR") ?
      float(iConfig.getParameter<double>("maxDR")) :
      0.5),
  isoDR_(iConfig.exists("isoDR") ?
      float(iConfig.getParameter<double>("isoDR")) :
      0.3),
  nIsoVetoDR_(iConfig.exists("nIsoVetoDR") ?
      float(iConfig.getParameter<double>("nIsoVetoDR")) :
      0.01),
  chIsoVetoDR_(iConfig.exists("chIsoVetoDR") ?
      float(iConfig.getParameter<double>("chIsoVetoDR")) :
      0.0001),
  relIsoCut_(iConfig.exists("relIsoCut") ?
      float(iConfig.getParameter<double>("relIsoCut")) :
      1.8),
  eMuCrossCleaningDR_(iConfig.exists("eMuCrossCleaningDR") ?
      float(iConfig.getParameter<double>("eMuCrossCleaningDR")) :
      0.)
{
  produces<std::vector<Muon> >();
  produces<std::vector<Elec> >();
}


PATObjectFSREmbedder::~PATObjectFSREmbedder()
{
}


// Some cuts, especially isolation, are very slow, so do the steps in
// a sensible order
void PATObjectFSREmbedder::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::unique_ptr<std::vector<Muon> > mOut = std::make_unique<std::vector<Muon> > ();
  std::unique_ptr<std::vector<Elec> > eOut = std::make_unique<std::vector<Elec> > ();
  edm::Handle<PCandView> cands;
  iEvent.getByToken(cands_, cands);
  edm::Handle<edm::View<Elec> > elecs;
  iEvent.getByToken(electrons_, elecs);
  edm::Handle<edm::View<Muon> > mus;
  iEvent.getByToken(muons_, mus);


  // associate photons to their closest leptons
  std::vector<std::vector<PCandRef> > phosByEle = std::vector<std::vector<PCandRef> >(elecs->size());
  std::vector<std::vector<PCandRef> > phosByMu = std::vector<std::vector<PCandRef> >(mus->size());

  for( size_t iPho = 0; iPho != cands->size(); ++iPho )
  {
    PCandRef pho = cands->refAt(iPho).castTo<PCandRef>();

    // basic selection
    //if (!phoSelection_(*pho))
    //  continue;
    bool phoEdecision = phoESelection_(*pho);
    bool phoMdecision = phoMSelection_(*pho);
    if (!phoEdecision && !phoMdecision) continue;

    std::list<std::pair<size_t, float> > closeEles;
    std::list<std::pair<size_t, float> > closeMus;

    if (phoEdecision){
      for(size_t iE = 0; iE < elecs->size(); ++iE)
      {
        float deltaR = reco::deltaR(pho->p4(), elecs->at(iE).p4());

        if(deltaR > maxDR_ || !eSelection_(elecs->at(iE)))
          continue;

        if(closeEles.empty() || deltaR < closeEles.front().second)
        {
          closeEles.emplace_front(std::pair<size_t, float>(iE, deltaR));
        }
        else
        {
          // we almost never need the second one, so don't waste time
          // sorting the rest
          closeEles.emplace_back(std::pair<size_t, float>(iE, deltaR));
        }
      }
    }

    if (phoMdecision){
      for(size_t iM = 0; iM < mus->size(); ++iM)
      {
        float deltaR = reco::deltaR(pho->p4(), mus->at(iM).p4());

        if(deltaR > maxDR_ || !mSelection_(mus->at(iM)))
          continue;

        if(closeMus.empty() || deltaR < closeMus.front().second)
        {
          closeMus.emplace_front(std::pair<size_t, float>(iM, deltaR));
        }
        else
        {
          // we almost never need the second one, so don't waste time
          // sorting the rest
          closeMus.emplace_back(std::pair<size_t, float>(iM, deltaR));
        }
      }
    }


    if(closeEles.size() &&
        (closeMus.empty() ||
         closeEles.front().second < closeMus.front().second)
      )
    {
      // Make sure electron isn't removed by cross cleaning
      bool crossCleaned = false;
      for(auto& m : closeMus)
      {
        if(std::abs(closeEles.front().second - m.second) < eMuCrossCleaningDR_)
        {
          if(reco::deltaR(elecs->at(closeEles.front().first).p4(),
                mus->at(m.first)) < eMuCrossCleaningDR_)
          {
            crossCleaned = true;
            break;
          }
        }
      }

      if(!crossCleaned)
        phosByEle.at(closeEles.front().first).push_back(pho);
      else
      {
        // remove the bad electron
        closeEles.pop_front();

        //// find the new closest electron
        // function for sorting these things
        std::function<bool(const std::pair<size_t,float>&,
            const std::pair<size_t,float>&)>
          f([](const std::pair<size_t,float>& a,
                const std::pair<size_t,float>& b)
              {return a.second < b.second;});

        closeEles.sort(f);

        // if there are only muons left, use them
        if(closeEles.empty())
        {
          if(closeMus.size() && closeMus.front().second < maxDR_)
            phosByMu.at(closeMus.front().first).push_back(pho);
        }
        else
        {
          for(auto& e : closeEles)
          {
            // if the best muon is better, use that
            if(closeMus.size() && e.second > closeMus.front().second)
            {
              phosByMu.at(closeMus.front().first).push_back(pho);
              break;
            }

            // is this electron also cross cleaned?
            bool crossCleaned = false;
            for(auto& m : closeMus)
            {
              if(std::abs(e.second - m.second) < eMuCrossCleaningDR_)
              {
                if(reco::deltaR(elecs->at(e.first).p4(),
                      mus->at(m.first)) < eMuCrossCleaningDR_)
                {
                  crossCleaned = true;
                  break;
                }
              }
            }

            if(!crossCleaned)
            {
              phosByEle.at(e.first).push_back(pho);
              break;
            }
          }
        }
      }
    }
    else if(closeMus.size() && closeMus.front().second < maxDR_)
      phosByMu.at(closeMus.front().first).push_back(pho);
  }


  // Will be filled in isolation calculation function if needed
  std::vector<PCandRef> nIsoCands;
  std::vector<PCandRef> chIsoCands;

  for(size_t iE = 0; iE < elecs->size(); ++iE)
  {
    Elec e = elecs->at(iE);

    PCandRef bestPho;
    float dREtBestPho = 9999.;

    for(size_t iPho = 0; iPho < phosByEle[iE].size(); ++iPho)
    {
      PCandRef pho = phosByEle[iE][iPho];

      float drEt = reco::deltaR(e.p4(), pho->p4()) / pow(pho->et(), etPower_);

      if(drEt > cut_ || drEt > dREtBestPho) continue;

      if(candInSuperCluster(pho, elecs)) continue;

      if(!passIso(pho, nIsoCands, chIsoCands, cands)) continue;

      dREtBestPho = drEt;
      bestPho = pho;
    }

    if(bestPho.isNonnull())
    {
      e.addUserCand(fsrLabel_, edm::refToPtr(bestPho));
      e.addUserFloat(fsrLabel_+"DREt", dREtBestPho);
    }

    eOut->push_back(e);
  }

  for(size_t iM = 0; iM < mus->size(); ++iM)
  {
    Muon m = mus->at(iM);

    PCandRef bestPho;
    float dREtBestPho = 9999.;

    for(size_t iPho = 0; iPho < phosByMu[iM].size(); ++iPho)
    {
      PCandRef pho = phosByMu[iM][iPho];

      float drEt = reco::deltaR(m.p4(), pho->p4()) / pow(pho->et(), etPower_);

      if(drEt > cut_ || drEt > dREtBestPho) continue;

      if(candInSuperCluster(pho, elecs)) continue;

      if(!passIso(pho, nIsoCands, chIsoCands, cands)) continue;

      dREtBestPho = drEt;
      bestPho = pho;
    }

    if(bestPho.isNonnull())
    {
      m.addUserCand(fsrLabel_, edm::refToPtr(bestPho));
      m.addUserFloat(fsrLabel_+"DREt", dREtBestPho);
    }

    mOut->push_back(m);
  }

  iEvent.put(std::move(eOut));
  iEvent.put(std::move(mOut));
}


bool PATObjectFSREmbedder::candInSuperCluster(const PCandRef& pho,
    const edm::Handle<edm::View<Elec> >& elecs) const
{
  for(size_t iE = 0; iE < elecs->size(); ++iE)
  {
    ElecPtr elec = elecs->ptrAt(iE);
    if(eSelection_(*elec))
    {
      for(auto& cand : elec->associatedPackedPFCandidates())
      {
        if(pho == cand)
          return true;
      }
    }
  }

  return false;
}


bool PATObjectFSREmbedder::passIso(const PCandRef& pho,
    std::vector<PCandRef>& nIsoCands,
    std::vector<PCandRef>& chIsoCands,
    const edm::Handle<edm::View<PCand> >& allCands) const
{
  // fill iso cand lists if needed
  if(nIsoCands.size() == 0 && chIsoCands.size() == 0)
  {
    for(size_t i = 0; i < allCands->size(); ++i)
    {
      if(nIsoSelection_(allCands->at(i)))
        nIsoCands.push_back(allCands->refAt(i).castTo<PCandRef>());
      else if(chIsoSelection_(allCands->at(i)))
        chIsoCands.push_back(allCands->refAt(i).castTo<PCandRef>());
    }
  }

  double iso = 0.;

  for(auto& cand : nIsoCands)
  {
    double dR = reco::deltaR(pho->p4(), cand->p4());
    if(dR < isoDR_ && dR > nIsoVetoDR_)
      iso += cand->pt();
  }

  for(auto& cand : chIsoCands)
  {
    double dR = reco::deltaR(pho->p4(), cand->p4());
    if(dR < isoDR_ && dR > chIsoVetoDR_)
      iso += cand->pt();
  }

  return iso / pho->pt() < relIsoCut_;
}


//define this as a plug-in
DEFINE_FWK_MODULE(PATObjectFSREmbedder);

