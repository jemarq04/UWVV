#ifndef UWVV_Ntuplizer_BranchHolder_h
#define UWVV_Ntuplizer_BranchHolder_h

// STL
#include <string>
#include <functional>
#include <vector>
#include <memory>

// ROOT
#include "TTree.h"
#include "TMath.h"

// CMSSW
#include "DataFormats/Common/interface/Ptr.h"

// UWVV
#include "UWVV/Ntuplizer/interface/EventInfo.h"

namespace uwvv {

  // Container to fill an ntuple branch (of type B) with values computed by a
  // function specified either by a
  // std::function<B(const edm::Ptr<T>&, EventInfo&)>
  template <typename B, class T>
  class BranchHolder {
  public:
    // Declare FType to be the type of the function pointer used here
    typedef B(FType)(const edm::Ptr<T>&, EventInfo&);

    // Default constuctor (null)
    BranchHolder() { ; }
    // Construct with function passed as argument
    BranchHolder(const std::string& name, TTree* const tree, const std::function<FType> func);
    BranchHolder(const std::string& name, TTree* const tree, FType func);
    virtual ~BranchHolder() { ; }

    // Compute value for this and set so that the next tree->Fill() will take it
    void fill(const edm::Ptr<T>& obj, EventInfo& evt);

    const std::string& getName() const { return name; }

    const B& getValue() const { return value; }

  private:
    const std::string name;
    const std::function<FType> f;

    B value;
  };

  template <typename B, class T>
  BranchHolder<B, T>::BranchHolder(const std::string& name, TTree* const tree, BranchHolder<B, T>::FType func)
      : name(name), f(func) {
    tree->Branch(name.c_str(), &value);
  }

  template <typename B, class T>
  BranchHolder<B, T>::BranchHolder(const std::string& name,
                                   TTree* const tree,
                                   const std::function<BranchHolder<B, T>::FType> func)
      : name(name), f(func) {
    tree->Branch(name.c_str(), &value);
  }

  template <typename B, class T>
  void BranchHolder<B, T>::fill(const edm::Ptr<T>& obj, EventInfo& evt) {
    value = f(obj, evt);
  }

}  // namespace uwvv

#endif  // header guard
