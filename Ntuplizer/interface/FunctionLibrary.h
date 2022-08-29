#ifndef UWVV_Ntuplizer_FunctionLibrary_h
#define UWVV_Ntuplizer_FunctionLibrary_h


#include <functional>
#include <unordered_map>
#include <string>
#include <vector>

#include "UWVV/Ntuplizer/interface/EventInfo.h"
#include "UWVV/Ntuplizer/interface/StringFunctionMaker.h"
#include "UWVV/Utilities/interface/helpers.h"
#include "UWVV/DataFormats/interface/DressedGenParticle.h"

#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Common/interface/Ptr.h"
#include "DataFormats/Math/interface/deltaPhi.h"
#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/JetReco/interface/GenJet.h"


namespace
{
  //// Separate templates to allow easier partial specialization

  template<typename B>
    struct GeneralFunctionList
    {
      // Null version for types we don't specify anything
      template<class T> static void
      addFunctions(std::unordered_map<std::string, std::function<B(const edm::Ptr<T>&,uwvv::EventInfo&,const std::string&)> >& addTo) {;}
    };

  template<>
    struct GeneralFunctionList<std::vector<float> >
    {
      template<class T> static void
      addFunctions(std::unordered_map<std::string, std::function<std::vector<float>(const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&)> >& addTo)
      {
        typedef std::vector<float> (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

        addTo["genJetPt"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     if(!uwvv::helpers::overlapWithAnyDaughter(evt.genJets(option)->at(i), *obj, 0.4))
                                       out.push_back(evt.genJets(option)->at(i).pt());
                                   }

                                 return out;
                               });

        addTo["genJetEta"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     if(!uwvv::helpers::overlapWithAnyDaughter(evt.genJets(option)->at(i), *obj, 0.4))
                                       out.push_back(evt.genJets(option)->at(i).eta());
                                   }

                                 return out;
                               });

        addTo["genJetPhi"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     if(!uwvv::helpers::overlapWithAnyDaughter(evt.genJets(option)->at(i), *obj, 0.4))
                                       out.push_back(evt.genJets(option)->at(i).phi());
                                   }

                                 return out;
                               });

        addTo["genJetRapidity"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     if(!uwvv::helpers::overlapWithAnyDaughter(evt.genJets(option)->at(i), *obj, 0.4))
                                       out.push_back(evt.genJets(option)->at(i).rapidity());
                                   }

                                 return out;
                               });

        addTo["lheWeights"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                  std::vector<float> out;

                                  if (!evt.lheEventInfo().isValid())
                                    throw cms::Exception("ProductNotFound")
                                        << "Unable to open LHE event information";

                                  unsigned long first_weight = 0;
                                  // Arbitrary choice, but 1000 weights would be pretty excessive
                                  unsigned long last_weight = 1000;
                                  if (option != "")
                                    {
                                      size_t pos = option.find(",");
                                      // If only 1 weight is specified, take it as the last weight (start at 0)
                                      if (pos == std::string::npos)
                                        try
                                          {
                                            last_weight = std::stoul(option);
                                          }
                                        catch (const std::exception& e)
                                          {
                                            std::string message = "Unable to parse option " + option +
                                                " for LHE weights. Error from ";
                                            throw std::runtime_error(message + e.what());
                                          }
                                      else
                                        {
                                          std::string begin = option.substr(0, pos);
                                          std::string end = option.substr(pos+1);
                                          try
                                            {
                                              first_weight = std::stoul(begin);
                                              last_weight = std::stoul(end);
                                            }
                                          catch (const std::exception& e)
                                            {
                                              std::string message = "Unable to parse option " + option +
                                                  " for LHE weights. Error from ";
                                              throw std::runtime_error(message + e.what());
                                            }
                                        }
                                    }
                                  auto weights = evt.lheEventInfo()->weights();
                                  for (unsigned long i = first_weight; i <  weights.size(); i++)
                                    {
                                      if (i == last_weight)
                                        break;
                                      out.push_back(weights[i].wgt);
                                    }
                                  return out;
                                });
      }
    };

  template<>
    struct GeneralFunctionList<float>
    {
      template<class T> static void
      addFunctions(std::unordered_map<std::string, std::function<float(const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&)> >& addTo)
      {
        typedef float (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

        addTo["pvZ"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return (evt.pv().isNonnull() ? evt.pv()->z() : -999.);
                               });

        addTo["pvndof"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return (evt.pv().isNonnull() ? evt.pv()->ndof() : -999.);
                               });

        addTo["pvRho"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return (evt.pv().isNonnull() ? evt.pv()->position().Rho() : -999.);
                               });

        addTo["nTruePU"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {return (evt.puInfo().isValid() && evt.puInfo()->size() > 0 ?
                                        evt.puInfo()->at(1).getTrueNumInteractions() :
                                        -1.);});
        addTo["L1prefiringWeight"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {float L1PrefWeight = 1.0;
                               if (*(evt.prefweight()) >= 0.) {L1PrefWeight = *(evt.prefweight());}
                               return (L1PrefWeight);});

        addTo["L1prefiringWeightUp"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {float L1PrefWeightUp = 1.0;
                               if (*(evt.prefweightup()) >= 0.) {L1PrefWeightUp = *(evt.prefweightup());}
                               return (L1PrefWeightUp);});
        addTo["L1prefiringWeightDn"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {float L1PrefWeightDn = 1.0;
                               if (*(evt.prefweightdown()) >= 0.) {L1PrefWeightDn = *(evt.prefweightdown());}
                               return (L1PrefWeightDn);});
        addTo["type1_pfMETEt"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {return evt.met().pt();});

        addTo["type1_pfMETPhi"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {return evt.met().phi();});

        addTo["genWeight"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return (evt.genEventInfo().isValid() ? evt.genEventInfo()->weight() : 0.);
                               });

        addTo["originalXWGTUP"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return (evt.lheEventInfo()->originalXWGTUP());
                               });

        addTo["mtToMET"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 float totalEt = obj->et() + evt.met().et();
                                 float totalPt = (obj->p4() + evt.met().p4()).pt();
                                 float mtSqr = totalEt * totalEt - totalPt * totalPt;

                                 return std::sqrt(mtSqr);
                               });

        addTo["mjjGen"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.genJets(option)->size() < 2)
                                   return -999.;

                                 const reco::GenJet* j1 = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     const reco::GenJet& j = evt.genJets(option)->at(i);
                                     if(!uwvv::helpers::overlapWithAnyDaughter(j, *obj, 0.4))
                                       {
                                         if(j1)
                                           return (j1->p4()+j.p4()).mass();
                                         else
                                           j1 = &j;
                                       }
                                   }

                                 return -999.;
                               });

        addTo["ptjjGen"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.genJets(option)->size() < 2)
                                   return -999.;

                                 const reco::GenJet* j1 = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     const reco::GenJet& j = evt.genJets(option)->at(i);
                                     if(!uwvv::helpers::overlapWithAnyDaughter(j, *obj, 0.4))
                                       {
                                         if(j1)
                                           return (j1->p4()+j.p4()).pt();
                                         else
                                           j1 = &j;
                                       }
                                   }

                                 return -999.;
                               });

        addTo["etajjGen"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.genJets(option)->size() < 2)
                                   return -999.;

                                 const reco::GenJet* j1 = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     const reco::GenJet& j = evt.genJets(option)->at(i);
                                     if(!uwvv::helpers::overlapWithAnyDaughter(j, *obj, 0.4))
                                       {
                                         if(j1)
                                           return (j1->p4()+j.p4()).eta();
                                         else
                                           j1 = &j;
                                       }
                                   }

                                 return -999.;
                               });

        addTo["phijjGen"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.genJets(option)->size() < 2)
                                   return -999.;

                                 const reco::GenJet* j1 = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     const reco::GenJet& j = evt.genJets(option)->at(i);
                                     if(!uwvv::helpers::overlapWithAnyDaughter(j, *obj, 0.4))
                                       {
                                         if(j1)
                                           return (j1->p4()+j.p4()).phi();
                                         else
                                           j1 = &j;
                                       }
                                   }

                                 return -999.;
                               });

        addTo["deltaEtajjGen"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.genJets(option)->size() < 2)
                                   return -999.;

                                 const reco::GenJet* j1 = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     const reco::GenJet& j = evt.genJets(option)->at(i);
                                     if(!uwvv::helpers::overlapWithAnyDaughter(j, *obj, 0.4))
                                       {
                                         if(j1)
                                           return std::abs(j1->eta() - j.eta());
                                         else
                                           j1 = &j;
                                       }
                                   }

                                 return -999.;
                               });

        addTo["zeppenfeldGen"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.genJets(option)->size() < 2)
                                   return -999.;

                                 const reco::GenJet* j1 = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     const reco::GenJet& j = evt.genJets(option)->at(i);
                                     if(!uwvv::helpers::overlapWithAnyDaughter(j, *obj, 0.4))
                                       {
                                         if(j1)
                                           return std::abs(obj->rapidity() -
                                                           (j1->rapidity() +
                                                            j.rapidity()) / 2.
                                                           );
                                         else
                                           j1 = &j;
                                       }
                                   }

                                 return -999.;
                               });

        addTo["zeppenfeldj3Gen"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.genJets(option)->size() < 3)
                                   return -999.;

                                 const reco::GenJet* j1 = 0;
                                 const reco::GenJet* j2 = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     const reco::GenJet& j = evt.genJets(option)->at(i);
                                     if(!uwvv::helpers::overlapWithAnyDaughter(j, *obj, 0.4))
                                       {
                                         if(j2)
                                           return std::abs(j.rapidity() -
                                                           (j1->rapidity() +
                                                            j2->rapidity()) / 2.
                                                           );
                                         else if(j1)
                                           j2 = &j;
                                         else
                                           j1 = &j;
                                       }
                                   }

                                 return -999.;
                               });

        addTo["deltaPhiTojjGen"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.genJets(option)->size() < 2)
                                   return -999.;

                                 const reco::GenJet* j1 = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     const reco::GenJet& j = evt.genJets(option)->at(i);
                                     if(!uwvv::helpers::overlapWithAnyDaughter(j, *obj, 0.4))
                                       {
                                         if(j1)
                                           {
                                             float phiJJ = (j1->p4() + j.p4()).phi();
                                             return std::abs(deltaPhi(obj->phi(), phiJJ));
                                           }
                                         else
                                           j1 = &j;
                                       }
                                   }

                                 return -999.;
                               });

        addTo["minLHEWeight"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                  if (!evt.lheEventInfo().isValid())
                                    throw cms::Exception("ProductNotFound")
                                        << "Unable to open LHE event information";

                                  unsigned long first_weight = 0;
                                  // Arbitrary choice, but 1000 weights would be pretty excessive
                                  unsigned long last_weight = 1000;
                                  if (option != "")
                                    {
                                      size_t pos = option.find(",");
                                      // If only 1 weight is specified, take it as the last weight (start at 0)
                                      if (pos == std::string::npos)
                                        try
                                          {
                                            last_weight = std::stoul(option);
                                          }
                                        catch (const std::exception& e)
                                          {
                                            std::string message = "Unable to parse option " + option +
                                                " for LHE weights. Error from ";
                                            throw std::runtime_error(message + e.what());
                                          }
                                      else
                                        {
                                          std::string begin = option.substr(0, pos);
                                          std::string end = option.substr(pos+1);
                                          try
                                            {
                                              first_weight = std::stoul(begin);
                                              last_weight = std::stoul(end);
                                            }
                                          catch (const std::exception& e)
                                            {
                                              std::string message = "Unable to parse option " + option +
                                                  " for LHE weights. Error from ";
                                              throw std::runtime_error(message + e.what());
                                            }
                                        }
                                    }

                                  float minWeight = 999.;

                                  auto weights = evt.lheEventInfo()->weights();
                                  for (unsigned long i = first_weight; i <  weights.size(); i++)
                                    {
                                      if (i == last_weight)
                                        break;
                                      if(i == 5 || i == 7) // some scale weights don't count, apparently
                                        continue;
                                      if(weights[i].wgt < minWeight)
                                        minWeight = weights[i].wgt;
                                    }

                                  return minWeight;
                                });

        addTo["maxLHEWeight"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                  if (!evt.lheEventInfo().isValid())
                                    throw cms::Exception("ProductNotFound")
                                        << "Unable to open LHE event information";

                                  unsigned long first_weight = 0;
                                  // Arbitrary choice, but 1000 weights would be pretty excessive
                                  unsigned long last_weight = 1000;
                                  if (option != "")
                                    {
                                      size_t pos = option.find(",");
                                      // If only 1 weight is specified, take it as the last weight (start at 0)
                                      if (pos == std::string::npos)
                                        try
                                          {
                                            last_weight = std::stoul(option);
                                          }
                                        catch (const std::exception& e)
                                          {
                                            std::string message = "Unable to parse option " + option +
                                                " for LHE weights. Error from ";
                                            throw std::runtime_error(message + e.what());
                                          }
                                      else
                                        {
                                          std::string begin = option.substr(0, pos);
                                          std::string end = option.substr(pos+1);
                                          try
                                            {
                                              first_weight = std::stoul(begin);
                                              last_weight = std::stoul(end);
                                            }
                                          catch (const std::exception& e)
                                            {
                                              std::string message = "Unable to parse option " + option +
                                                  " for LHE weights. Error from ";
                                              throw std::runtime_error(message + e.what());
                                            }
                                        }
                                    }

                                  float maxWeight = -999.;

                                  auto weights = evt.lheEventInfo()->weights();
                                  for (unsigned long i = first_weight; i <  weights.size(); i++)
                                    {
                                      if (i == last_weight)
                                        break;
                                      if(i == 5 || i == 7) // some scale weights don't count, apparently
                                        continue;
                                      if(weights[i].wgt > maxWeight)
                                        maxWeight = weights[i].wgt;
                                    }

                                  return maxWeight;
                                });

        addTo["genInitialStateMass"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.initialStates()->size())
                                   return evt.initialStates()->at(0).mass();
                                 return -999.;
                               });

        addTo["genInitialStatePt"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.initialStates()->size())
                                   return evt.initialStates()->at(0).pt();
                                 return -999.;
                               });

        addTo["genInitialStateEta"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.initialStates()->size())
                                   return evt.initialStates()->at(0).eta();
                                 return -999.;
                               });

        addTo["genInitialStatePhi"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if(evt.initialStates()->size())
                                   return evt.initialStates()->at(0).phi();
                                 return -999.;
                               });
      }
    };

  template<>
    struct GeneralFunctionList<bool>
    {
      template<class T> static void
      addFunctions(std::unordered_map<std::string, std::function<bool(const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&)> >& addTo)
      {
        typedef bool (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

        addTo["pvIsValid"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return evt.pv().isNonnull() && evt.pv()->isValid();
                               });

        addTo["pvIsFake"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return evt.pv().isNull() || evt.pv()->isFake();
                               });
      }
    };

  template<>
    struct GeneralFunctionList<int>
    {
      template<class T> static void
      addFunctions(std::unordered_map<std::string, std::function<int(const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&)> >& addTo)
      {
        typedef int (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

        addTo["Charge"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option) {return obj->charge();});

        addTo["PdgId"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option) {return obj->pdgId();});
      }
    };

  template<>
    struct GeneralFunctionList<unsigned>
    {
      template<class T> static void
      addFunctions(std::unordered_map<std::string, std::function<unsigned(const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&)> >& addTo)
      {
        typedef unsigned (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

        addTo["lumi"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {return evt.id().luminosityBlock();});

        addTo["run"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {return evt.id().run();});

        addTo["nvtx"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {return evt.nVertices();});

        addTo["nGenJets"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 unsigned out = 0;
                                 for(size_t i = 0; i < evt.genJets(option)->size(); ++i)
                                   {
                                     if(!uwvv::helpers::overlapWithAnyDaughter(evt.genJets(option)->at(i), *obj, 0.4))
                                       out++;
                                   }

                                 return out;
                               });
      }
    };

  template<>
    struct GeneralFunctionList<unsigned long long>
    {
      template<class T> static void
      addFunctions(std::unordered_map<std::string, std::function<unsigned long long(const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&)> >& addTo)
      {
        typedef unsigned long long (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

        addTo["evt"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {return evt.id().event();});
      }
    };



  //// Functions specific to a particular object type

  // null if it's some type combination we don't have functions for
  template<typename B, class T>
    struct ObjectFunctionList
    {
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);
      static void addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo) {;}
    };

  template<>
    struct ObjectFunctionList<unsigned, pat::Electron>
    {
      // cheating with typedefs for standardization
      typedef pat::Electron T;
      typedef unsigned B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {
        addTo["MissingHits"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->gsfTrack()->hitPattern().numberOfAllHits(reco::HitPattern::MISSING_INNER_HITS);
                               });
      }
    };

  template<>
    struct ObjectFunctionList<float, pat::Electron>
    {
      // cheating with typedefs for standardization
      typedef pat::Electron T;
      typedef float B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {
        addTo["SIP3D"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return fabs(obj->dB(T::PV3D)) / obj->edB(T::PV3D);
                               });

        addTo["IP3D"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return fabs(obj->dB(T::PV3D));
                               });

        addTo["IP3DUncertainty"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->edB(T::PV3D);
                               });

        addTo["SIP2D"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return fabs(obj->dB(T::PV2D)) / obj->edB(T::PV2D);
                               });

        addTo["IP2D"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return fabs(obj->dB(T::PV2D));
                               });

        addTo["IP2DUncertainty"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->edB(T::PV2D);
                               });

        addTo["PVDZ"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->gsfTrack()->dz(evt.pv()->position());
                               });

        addTo["PVDXY"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->gsfTrack()->dxy(evt.pv()->position());
                               });
      }
    };

  template<>
    struct ObjectFunctionList<float, pat::Muon>
    {
      // cheating with typedefs for standardization
      typedef pat::Muon T;
      typedef float B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {
        addTo["SIP3D"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return fabs(obj->dB(T::PV3D)) / obj->edB(T::PV3D);
                               });

        addTo["IP3D"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return fabs(obj->dB(T::PV3D));
                               });

        addTo["IP3DUncertainty"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->edB(T::PV3D);
                               });

        addTo["SIP2D"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return fabs(obj->dB(T::PV2D)) / obj->edB(T::PV2D);
                               });

        addTo["IP2D"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return fabs(obj->dB(T::PV2D));
                               });

        addTo["IP2DUncertainty"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->edB(T::PV2D);
                               });

        addTo["PVDZ"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->muonBestTrack()->dz(evt.pv()->position());
                               });

        addTo["PVDXY"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->muonBestTrack()->dxy(evt.pv()->position());
                               });
      }
    };

  template<>
    struct ObjectFunctionList<unsigned, pat::Muon>
    {
      // cheating with typedefs for standardization
      typedef pat::Muon T;
      typedef unsigned B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {
        addTo["BestTrackType"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option){return obj->muonBestTrackType();});

        addTo["MatchedStations"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option){return obj->numberOfMatchedStations();});

        addTo["NoOfMatches"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option){return obj->numberOfMatches();});
      }
    };

  math::XYZTLorentzVector getUndressedP4(const edm::Ptr<pat::CompositeCandidate>& cand)
    {
      math::XYZTLorentzVector out;
      for(size_t i = 0; i < cand->numberOfDaughters(); ++i)
        {
          const reco::Candidate* d = cand->daughter(i);
          if(d->numberOfDaughters())
            {
              edm::Ptr<pat::CompositeCandidate> lep = d->masterClone().castTo<edm::Ptr<pat::CompositeCandidate> >();
              out += ::getUndressedP4(lep);
            }
          else
            {
              const DressedGenParticle* lep = dynamic_cast<const DressedGenParticle*>(d->masterClone().get());
              if(lep)
                out += lep->undressedP4();
              else // assume anything other than a DressedGenParticle is ok
                out += d->p4();
            }
        }

      return out;
    }

  template<>
    struct ObjectFunctionList<unsigned int, pat::CompositeCandidate>
    {
      // cheating with typedefs for standardization
      typedef pat::CompositeCandidate T;
      typedef unsigned int B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {
        addTo["nJets"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                   return uwvv::helpers::getCleanedJetCollection(*obj, option)->size();
                               });
      }
    };

  template<>
    struct ObjectFunctionList<float, pat::CompositeCandidate>
    {
      // cheating with typedefs for standardization
      typedef pat::CompositeCandidate T;
      typedef float B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {
        addTo["mjj"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 const edm::PtrVector<pat::Jet>* cleanedJets = uwvv::helpers::getCleanedJetCollection(*obj, option);
                                 if(cleanedJets->size() < 2)
                                   return -999.;
                                    
                                 return ((*cleanedJets)[0]->p4() + (*cleanedJets)[1]->p4()).mass();
                               });
        addTo["jetPUSFmulfac"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 if ( ! obj->hasUserData("jetPUSFmulfac") ){
                                    throw cms::Exception("ProductNotFound")
                                    << "jet PUSF mulfac not found"
                                    << "for variation " << option;}
                                    
                                 return *(obj->userData<float>("jetPUSFmulfac"));
                               });
        addTo["ptjj"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 const edm::PtrVector<pat::Jet>* cleanedJets = uwvv::helpers::getCleanedJetCollection(*obj, option);
                                 if(cleanedJets->size() < 2)
                                   return -999.;
                                    
                                 return ((*cleanedJets)[0]->p4() + (*cleanedJets)[1]->p4()).pt();
                               });

        addTo["etajj"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 const edm::PtrVector<pat::Jet>* cleanedJets = uwvv::helpers::getCleanedJetCollection(*obj, option);
                                 if(cleanedJets->size() < 2)
                                   return -999.;
                                    
                                 return ((*cleanedJets)[0]->p4() + (*cleanedJets)[1]->p4()).eta();
                               });

        addTo["phijj"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 const edm::PtrVector<pat::Jet>* cleanedJets = uwvv::helpers::getCleanedJetCollection(*obj, option);
                                 if(cleanedJets->size() < 2)
                                   return -999.;
                                    
                                 return ((*cleanedJets)[0]->p4() + (*cleanedJets)[1]->p4()).phi();
                               });

        addTo["deltaEtajj"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 const edm::PtrVector<pat::Jet>* cleanedJets = uwvv::helpers::getCleanedJetCollection(*obj, option);
                                 if(cleanedJets->size() < 2)
                                   return -999.;
                                    
                                 return std::abs((*cleanedJets)[0]->eta() - (*cleanedJets)[1]->eta());
                               });

        addTo["zeppenfeld"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 const edm::PtrVector<pat::Jet>* cleanedJets = uwvv::helpers::getCleanedJetCollection(*obj, option);
                                 if(cleanedJets->size() < 2)
                                   return -999.;
                                    
                                 return std::abs(obj->rapidity() -
                                                           ((*cleanedJets)[0]->rapidity() +
                                                            (*cleanedJets)[1]->rapidity()) / 2.
                                                           );
                               });

        addTo["zeppenfeldj3"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 const edm::PtrVector<pat::Jet>* cleanedJets = uwvv::helpers::getCleanedJetCollection(*obj, option);
                                 if(cleanedJets->size() < 3)
                                   return -999.;
                                    
                                 return std::abs((*cleanedJets)[2]->rapidity() -
                                                           ((*cleanedJets)[0]->rapidity() +
                                                            (*cleanedJets)[1]->rapidity()) / 2.
                                                           );
                               });

        addTo["deltaPhiTojj"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 const edm::PtrVector<pat::Jet>* cleanedJets = uwvv::helpers::getCleanedJetCollection(*obj, option);
                                 if(cleanedJets->size() < 2)
                                   return -999.;
                                    
                                 float phiJJ = ((*cleanedJets)[0]->p4() + (*cleanedJets)[1]->p4()).phi();
                                 return std::abs(deltaPhi(obj->phi(), phiJJ));
                               });


        addTo["DR"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return reco::deltaR(obj->daughter(0)->p4(),
                                                     obj->daughter(1)->p4());
                               });

        addTo["massNoFSR"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return uwvv::helpers::p4WithoutFSR(obj).mass();
                               });

        addTo["ptNoFSR"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return uwvv::helpers::p4WithoutFSR(obj).pt();
                               });

        addTo["etaNoFSR"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return uwvv::helpers::p4WithoutFSR(obj).eta();
                               });

        addTo["phiNoFSR"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return uwvv::helpers::p4WithoutFSR(obj).phi();
                               });

        addTo["energyNoFSR"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return uwvv::helpers::p4WithoutFSR(obj).energy();
                               });

        addTo["undressedMass"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return ::getUndressedP4(obj).mass();
                               });

        addTo["undressedPt"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return ::getUndressedP4(obj).pt();
                               });

        addTo["undressedEta"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return ::getUndressedP4(obj).eta();
                               });

        addTo["undressedPhi"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return ::getUndressedP4(obj).phi();
                               });

      }
    };

  template<>
    struct ObjectFunctionList<std::vector<int>, pat::CompositeCandidate>
    {
      // cheating with typedefs for standardization
      typedef pat::CompositeCandidate T;
      typedef std::vector<int> B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {

        addTo["jetHadronFlavor"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<int> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     out.push_back(jet->hadronFlavour());
                                   }

                                 return out;
                               });
        addTo["isGenJetMatched"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<int> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     int isMatch = -999;
                                     if(jet->hasUserFloat("genjetMatched")){
                                       isMatch = (int) jet->userFloat("genjetMatched");}

                                     out.push_back(isMatch);
                                   }

                                 return out;
                               });
        addTo["jetPUID"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<int> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     int puID = -999;
                                     if(jet->hasUserInt("pileupJetIdUpdated:fullId"))
                                       puID = jet->userInt("pileupJetIdUpdated:fullId");

                                     out.push_back(puID);
                                   }

                                 return out;
                               });
      }
    };

  template<>
    struct ObjectFunctionList<std::vector<float>, pat::CompositeCandidate>
    {
      // cheating with typedefs for standardization
      typedef pat::CompositeCandidate T;
      typedef std::vector<float> B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {
        addTo["jetPt"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     out.push_back(jet->pt());
                                   }

                                 return out;
                               });
        addTo["jetEta"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     out.push_back(jet->eta());
                                   }

                                 return out;
                               });
        addTo["jetPhi"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     out.push_back(jet->phi());
                                   }

                                 return out;
                               });

        addTo["jetRapidity"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     out.push_back(jet->rapidity());
                                   }

                                 return out;
                               });

        addTo["jetQGLikelihood"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     if(jet->hasUserFloat("qgLikelihood"))
                                       out.push_back(jet->userFloat("qgLikelihood"));
                                   }

                                 return out;
                               });

        addTo["jetCSVv2"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     out.push_back(jet->bDiscriminator("pfCombinedInclusiveSecondaryVertexV2BJetTags"));
                                   }

                                 return out;
                               });
        //https://twiki.cern.ch/twiki/bin/viewauth/CMS/BtagRecommendation94X
        addTo["jetDeepCSV"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     out.push_back(jet->bDiscriminator("pfDeepCSVJetTags:probb") + jet->bDiscriminator("pfDeepCSVJetTags:probbb"));
                                   }

                                 return out;
                               });

        addTo["jetCMVAv2"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 std::vector<float> out;

                                 for(auto& jet : *uwvv::helpers::getCleanedJetCollection(*obj, option))
                                   {
                                     out.push_back(jet->bDiscriminator("pfCombinedMVAV2BJetTags"));
                                   }

                                 return out;
                               });
      }
    };

  template<>
    struct ObjectFunctionList<bool, pat::CompositeCandidate>
    {
      // cheating with typedefs for standardization
      typedef pat::CompositeCandidate T;
      typedef bool B;
      typedef B (FType) (const edm::Ptr<T>&, uwvv::EventInfo&, const std::string&);

      static void
        addFunctions(std::unordered_map<std::string, std::function<FType> >& addTo)
      {
        addTo["SS"] =
          std::function<FType>([](const edm::Ptr<T>& obj, uwvv::EventInfo& evt, const std::string& option)
                               {
                                 return obj->daughter(0)->charge() == obj->daughter(1)->charge();
                               });
      }
    };

} // anonymous namespace


namespace uwvv
{

  template<typename B, class T>
  class BasicFunctionLibrary
  {
   public:

    // Declare signature of these functions as FType
    typedef B (FType) (const edm::Ptr<T>&, EventInfo&, const std::string&);
    // Outward-facing signature doesn't include option argument
    typedef B (FSig) (const edm::Ptr<T>&, EventInfo&);

    BasicFunctionLibrary()
      {
        ::GeneralFunctionList<B>::addFunctions(functions);
        ::ObjectFunctionList<B,T>::addFunctions(functions);
      }
    ~BasicFunctionLibrary() {;}

    std::function<FSig>
    getFunction(const std::string& f) const
      {
        // option indicated by '::', i.e. f="functionName::option"
        size_t sepStart = f.find("::");
        std::string fname = f.substr(0, sepStart);

        // if there's an option but the function is not in the library,
        // something is probably wrong, but we'll just let the
        // StringObjectFunction fail to compile
        if(functions.find(fname) == functions.end())
          return StringFunctionMaker::makeStringFunction<B, T, uwvv::EventInfo&>(f);

        std::string option = "";
        if(sepStart != std::string::npos && sepStart+2 < f.size())
          option = f.substr(sepStart+2);

        return std::bind(functions.at(fname), std::placeholders::_1,
                         std::placeholders::_2, option);
      }

    // for testing purposes
    // const std::unordered_map<std::string, std::function<FType> >&
    //   getAllFunctions() const {return functions;}

   protected:
    std::unordered_map<std::string,
      std::function<FType> > functions;
  };


  // Most function libraries need only what's in BasicFunctionLibrary
  template<typename B, class T>
  class FunctionLibrary : public BasicFunctionLibrary<B,T> {};


  // Function libraries returning vectors need some specialization
  template<typename B, class T>
  class FunctionLibrary<std::vector<B>,T> : public BasicFunctionLibrary<std::vector<B>,T>
  {
   public:
    typedef typename BasicFunctionLibrary<std::vector<B>,T>::FSig FSig;

    using BasicFunctionLibrary<std::vector<B>,T>::getFunction;

    std::function<FSig>
    getFunction(const std::vector<std::string>& fs) const
      {
        if(fs.size() == 1)
          {
            size_t sepStart = fs.at(0).find("::");
            std::string fname = fs.at(0).substr(0,sepStart);

            if(this->functions.find(fname) != this->functions.end())
              return getFunction(fs.at(0));
          }

        // Otherwise, make a new function that returns a vector
        std::vector<std::function<typename FunctionLibrary<B,T>::FSig> > needed;
        for(const auto& f : fs)
          needed.push_back(baseLib.getFunction(f));

        auto out = std::function<FSig>([needed](const edm::Ptr<T>& obj,
                                                uwvv::EventInfo& evt)
                                       {
                                         std::vector<B> out;
                                         for(const auto& fun : needed)
                                           out.push_back(fun(obj, evt));

                                         return out;
                                       });

        return out;
      }

   private:
    const FunctionLibrary<B,T> baseLib;
  };

} // namespace uwvv


#endif // header guard

