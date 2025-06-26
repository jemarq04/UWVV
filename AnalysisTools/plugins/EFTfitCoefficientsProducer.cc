//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//   EFTfitCoefficientsProducer.cc                                          //
//                                                                          //
//   Author: Justin Marquez, U. Wisconsin                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// system includes
#include <memory>
#include <vector>
#include <iostream>
#include <utility>
#include <string>
#include <sstream>

// CMS includes
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "SimDataFormats/GeneratorProducts/interface/LHEEventProduct.h"

// ROOT includes
#include "TMatrixD.h"
#include "TVectorD.h"
#include "TDecompSVD.h"

// TODO: didn't end up using names... remove them here? store them as well?
//  -> need to store it *somewhere*

class EFTfitCoefficientsEmbedder : public edm::stream::EDProducer<>
{
  public:
    explicit EFTfitCoefficientsEmbedder(const edm::ParameterSet &iConfig);
    virtual ~EFTfitCoefficientsEmbedder(){};

  private:
    virtual void produce(edm::Event &iEvent, const edm::EventSetup &iSetup);

    typedef std::pair<std::string, float> WC;
    typedef std::vector<WC> WCVec;

    void makeIndexPairs(size_t N);

    const edm::EDGetTokenT<LHEEventProduct> lheToken_;

    std::vector<std::pair<size_t, size_t> > indexPairs_;
};

EFTfitCoefficientsEmbedder::EFTfitCoefficientsEmbedder(const edm::ParameterSet &iConfig) :
  lheToken_(consumes<LHEEventProduct>(iConfig.getParameter<edm::InputTag>("lheInfo")))
{
  produces<std::vector<double> >();
}

void EFTfitCoefficientsEmbedder::produce(edm::Event &iEvent, const edm::EventSetup &iSetup)
{
  // Setup inputs and outputs
  edm::Handle<LHEEventProduct> lheInfo;
  iEvent.getByToken(lheToken_, lheInfo);

  std::unique_ptr<std::vector<double> > EFTfitCoefficients(new std::vector<double>());

  // Determine reweights
  size_t numReweights = 0;
  size_t numTerms = 0;
  std::vector<std::pair<WCVec, float> > reweightInfo;
  for (size_t i=0; i<lheInfo->weights().size(); i++)
  {
    auto weight = lheInfo->weights()[i];
    // Confirm we are looking at EFT reweights
    if (weight.id.find("EFTrwgt") != 0)
      continue;
    numReweights++;

    // Initialize vector
    WCVec coeffs;

    // Tokenize reweight ID
    std::vector<std::string> words;
    std::stringstream ss_name(weight.id);
    for (std::string word; std::getline(ss_name, word, '_');)
      words.push_back(word);

    // Store WCs from tokens
    for (size_t i=1; i<words.size(); i+=2)
      coeffs.push_back(std::make_pair(words[i], std::stod(words[i+1])));

    // Store number of WC*WC terms
    //  ((N+1)^2 - (N+1))/2 + N+1
    if (numTerms == 0)
    {
      int N = coeffs.size();
      numTerms = ((N+1)*(N+1) - (N+1))/2 + N+1;

      makeIndexPairs(N);
    }

    // Store WCs with reweight
    reweightInfo.push_back(std::make_pair(coeffs, weight.wgt));
  }

  // Confirm sufficient number of reweights to make fit
  if (numReweights < numTerms)
    throw cms::Exception("InsufficientEFTInputs")
      << "Not enough reweights were provided for the number of WCs used" << std::endl
      << "Needed " << numTerms << " and received " << numReweights;

  // Fill matrix A and vector w
  //  w = As
  //    w: vector of reweights
  //    A: matrix of N-quadratic terms
  //    s: structure constants of N-quadratic
  TMatrixD A(numReweights, numTerms);
  TVectorD w(numReweights);

  double x1,x2;
  for (size_t row=0; row<numReweights; row++)
  {
    for (size_t col=0; col<numTerms; col++)
    {
      x1 = indexPairs_[col].first > 0 ?
        reweightInfo[row].first[indexPairs_[col].first].second : 1.0;
      x2 = indexPairs_[col].second > 0 ?
        reweightInfo[row].first[indexPairs_[col].second].second : 1.0;
      A(row,col) = x1 * x2;
    }
    w(row) = reweightInfo[row].second;
  }

  // Solve for structure constants
  bool ok;
  TDecompSVD svd(A);
  TVectorD s = svd.Solve(w, ok);
  if (!ok)
    throw cms::Exception("EFTFitError")
      << "Unable to generate valid fit for given inputs";

  // Store fits in plain vector
  std::vector<float> EFTfit;
  for (size_t i=0; i<numTerms; i++)
    EFTfitCoefficients->push_back(s(i));

  // Write out coefficients
  iEvent.put(std::move(EFTfitCoefficients));
}

void EFTfitCoefficientsEmbedder::makeIndexPairs(size_t N){
  //Quadratic Form Convention:
  //  Dim=0: (0,0)
  //  Dim=1: (0,0) (1,0) (1,1)
  //  Dim=2: (0,0) (1,0) (1,1) (2,0) (2,1) (2,2)
  //  Dim=3: (0,0) (1,0) (1,1) (2,0) (2,1) (2,2) (3,0) (3,1) (3,2) (3,3)
  //  etc.
  //  Note: For ALL pairs --> p.first >= p.second

  indexPairs_.clear();
  for (size_t i=0; i<N; i++)
    for (size_t j=0; j<=i; j++)
      indexPairs_.push_back(std::make_pair(i, j));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(EFTfitCoefficientsEmbedder);
