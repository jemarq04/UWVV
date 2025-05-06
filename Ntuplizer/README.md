# UWVV ntuples

Each row in a UWVV ntuple represents one initial state candidate (not one event). The columns are information about the candidate, its daughters (and their daughters, recursively), and the event they live in. The format is essentially the same as in [FSA ntuples](https://github.com/uwcms/InitialStateAnalysis/tree/miniAOD_dev_80X/NtupleTools/test).

## Table of Contents

- [Tree generation](#tree-generation)
   * [Defining a single branch](#defining-a-single-branch)
      + [Scalar branches](#scalar-branches)
      + [Vector branches](#vector-branches)
   * [Specifying all branches](#specifying-all-branches)
   * [Function library](#function-library)
   * [Trigger branches](#trigger-branches)
   * [Branch naming convention](#branch-naming-convention)
   * [Gen ntuples](#gen-ntuples)
- [Metadata](#metadata)
- [Event info](#event-info)

## Tree generation

Trees are channel-specific. By default, a tree is in the output file as `[channel]/ntuple`, e.g. `eemm/ntuple` for a 2 electron, 2 muon final state. Trees are made by an EDAnalyzer called something like `TreeGeneratorEEMuMu` (or similar, depending on the channel).

This snippet makes a 2e2mu tree
```python
import FWCore.ParameterSet.Config as cms

from UWVV.Ntuplizer.eventParams import makeEventParams

###
### Set up flow (see AnalysisTools), ntuple parameters (see below), process etc.
###

process.eemmTree = cms.EDAnalyzer(
    'TreeGeneratorEEMuMu',
    src = flow.finalObjTag('eemm'),
    branches = eemmBranchPSet, # see below
    eventParams = makeEventParams(flow.finalTags()), # for setting up whole-event information
    triggers = triggerPSet, # see below
    )
```


### Defining a single branch

#### Scalar branches

Each branch is associated with an object (which may be a composite candidate or a final state object candidate) and defined by a cms.string. Quantities which can be calculated directly from the candidate object use CMS StringObjectFunctions constructed from the cms.string. For example, one can make a branch called `[object]GenPt` containing the pt of the gen particle associated to a PAT lepton (or -999 if there isn't one) like this
```python
GenPt = cms.string('? genParticleRef.isNull ? -999. : genParticleRef.pt')
```
CMS StringObjectFunctions always return doubles, which are automatically converted to the correct type for the branch.

Quantities that are more complicated to calculate, or which require information from other objects in the event, are defined in the function library (see below). If the branch string is the name of a function in the library, that function is used instead of a StringObjectFunction. Functions in the library take an extra string argument specified in the branch definition delimited from the function name by `::`. This most often used to specify an alternate collection to be used for a branch holding event info. For example, a branch of the dijet invariant mass with the jet energy scale shifted up would be give by `cms.string('mjj::jesUp')`.

#### Vector branches

Vector branches are defined the same way except with a cms.vstring instead of a cms.string giving the definition. If there is only one item in the cms.vstring, and there is a corresponding function in the library which returns a vector of the correct type, this function is used just like a scalar library function. Otherwise, each string in the cms.vstring is is interpreted as the definition of a scalar for one element in the vector (order is preserved).

### Specifying all branches

The definitions of the branches are passed to the TreeGenerator as a cms.PSet called `branches`. It should contain the following
* One PSet for each type of branch related to the event or the initial state. Each of these PSets should contain cms.strings/cms.vstrings defining the branch content (see above). Currently supported branch types are
  * `floats`
  * `ints`
  * `uints` (unsigned ints)
  * `bools`
  * `ulls` (unsigned long long ints)
  * `vFloats` (`std::vector<float>`)
  * `vInts` (`std::vector<int>`)
  * `vUInts` (`std::vector<unsigned int>`)
* `daughterNames`, a cms.vstring with the names of the first daughters, if any
* `daughterParams`, a cms.VPSet containing exactly one cms.PSet for each daughter, containing the same items as in this list, to define the branches related to the daughters

Not all branch type PSets are required; if no branches of a type are needed simply omit it. Vectors of other types can be added relatively trivially if needed.

As an example, consider a very simple tree, containing just the pt of a Z and its daughter electrons as floats, and the pt of the first jet without and with a systematic energy shift held in a length-2 vector of floats. The pt can be obtained from the Z and the electrons with StringObjectFunctions, while the jet pts must be obtained with a library function called `jet1Pt`. If the shifted jets are in a secondary collection called `jesUp`, the branches would be defined like this:
```python
branches = cms.PSet(
    floats = cms.PSet(
        Pt = cms.string('pt'),
        ),
    vFloats = cms.PSet(
        jet1Pt = cms.vstring('jet1Pt', 'jet1Pt::jesUp'),
        ),
    daughterNames = cms.vstring('e1', 'e2'),
    daughterParams = cms.VPSet(
        cms.PSet(
            floats = cms.PSet(
                Pt = cms.string('pt'),
                ),
            ),
        cms.PSet(
            floats = cms.PSet(
                Pt = cms.string('pt'),
                ),
            ),
        ),
    )
```

The branches in the tree will be `Pt`, `e1Pt`, and `e2Pt` (floats) and `jet1Pt` (`std::vector<float>`).

Some useful functions for copying and combining cms.PSets can be found in `Utilities/python/helpers.py`.


### Function library

For quantities that require a more involved calculation or other information about the event, functions are defined in `Ntuplizer/interface/FunctionLibrary.h`. These functions are stored as `std::function`s of the right signature, in maps specific to the object type and branch type. These functions take as arguments an `edm::Ptr` to the object, a reference to a `uwvv::EventInfo` object, which has access to a number of useful collections and quantities in the event, and an optional string defined in the branch string. I'd try to give more details about how to write the functions, but if you need to do anything with them, it's probably easier to just look at the code.



### Trigger branches

Trigger branches are defined separately, by the `triggers` parameter, which should be a cms.PSet containing
* `trigNames` (cms.vstring), a list of names for all the sets of triggers desired. Trigger results will appear in the ntuple as `[name]Pass`
* One cms.vstring for each entry in `trigNames`, called `[name]Paths`, giving the names of one or more paths to check
  * The decision stored in the ntuple is a logical OR of all paths in the cms.vstring
  * Regular expressions are allowed, which is most useful for the version number for each path. The string `'HLT_Mu50_v\\[0-9]+'` will match any version of the 50 GeV single muon path.
* `trigResultSrc` (cms.InputTag), the input tag for the trigger decisions
* `trigPrescaleSrc` (cms.InputTag), the input tag for the trigger prescales
* `checkPrescale` (cms.bool, optional) indicates whether the prescales should be stored in the ntuple (in branches called `[name]Prescale`).
  * If `checkPrescale` is True (the default), all paths for a given branch must have the same prescale or the TreeGenerator will throw an exception

The following will put dimuon and dielectron trigger decisions into the ntuple with no prescales
```python
triggerBranches = cms.PSet(
    trigNames = cms.vstring('doubleMu', 'doubleE',),
    doubleMuPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v\\[0-9]+',
                                'HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v\\[0-9]+'),
    doubleEPaths = cms.vstring('HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v\\[0-9]+',
                               'HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v\\[0-9]+',
                               'HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_v\\[0-9]+'),

    trigResultsSrc = cms.InputTag("TriggerResults", "", "HLT"),
    trigPrescaleSrc = cms.InputTag("patTrigger"),

    checkPrescale = cms.bool(False),
    )
```

### Branch naming convention

Branches with information about the initial state or the event are simply named after the quantity they hold, e.g. `Mass` for the reconstructed invariant mass of the initial state or `nvtx` for the number of reconstructed vertices in the event.

Branches with information about intermediate state particles or final state daughters are called `[object][Quantity]`, e.g. `e1Pt` for the pt of the first electron. Naming intermediate states `[daughter1]_[daughter2]_`, so that their branches are named things like `e1_e2_Mass` (for the mass of an intermediate Z->ee candidate), is recommended but not required.


### Gen ntuples

Composite candidates may be built from `reco::GenParticle`s the same as PAT particles, and generator level ntuples can be made from these with the `GenTreeGeneratorZZ` (4l final state) and `GenTreeGeneratorWZ` (3l final state) modules. In `ntuplize_cfg.py`, the option `genInfo=1` will make a second set of ntuples called `[channel]Gen` alongside the regular ntuples.


## Metadata

Useful meta info about the ntuples in the file, like which runs and lumi sections were processed and the sum of the generator weights of the sample, can be produced alongside the analysis ntuples.

```python
process.metaInfo = cms.EDAnalyzer(
    'MetaTreeGenerator',
    eventParams = makeEventParams(flow.finalTags()),
    )
```

The tree will be stored in the file as `metaInfo/metaInfo`. Each row is one luminosity block.


## Event info

The event info for library functions is held in a `uwvv::EventInfo` object. For each type of object, there is one primary collection and any number of secondary collections keyed to strings. The event info is set up from a `PSet` passed to the `TreeGenerator` as `eventParams`. For each primary collection, `eventParams` contains an `InputTag` called `[object]Src`. The secondary collections come from a `PSet` of `InputTag`s called `[object]Extra`, the keys of which are the strings used to get the collection from the `EventInfo` object. The collections are obtained with functions that return their `edm::Handle`s, usually to an `edm::View` or `std::vector` depending on the type. With no argument or `""`, they return the primary collection, with a `string` argument they return the collection of the appropriate key. The collections available are as follows:

| Collection       |  Type                  |  Object prefix                |  Retrieval function   |
|------------------|------------------------|-------------------------------|-----------------------|
| Vertices         | `reco::Vertex`         | `vtx`                         | `vertices()` <br> `pv()` (first)   |
| Electrons        | `pat::Electron`        | `e`                           | `electrons()`         |
| Muons            | `pat::Muon`            | `m`                           | `muons()`             |
| Taus             | `pat::Tau`             | `t`                           | `taus()`              |
| Photons          | `pat::Photon`          | `g`                           | `photons()`           |
| Jets             | `pat::Jet`             | `j`                           | `jets()`              |
| Packed particle flow candidates  | `pat::PackedCandidate` | `pfCand`      | `pfCands()`           |
| Missing energy   | `pat::METCollection` <br> `pat::MET` | `met`           | `mets()` (collection) <br> `met()` (first) |
| Pileup           | `PileupSummaryInfo`    | `pu`                          | `puInfo()`            |
| Generator information  | `GenEventInfoProduct`  | `genEventInfo`          | `genEventInfo()`      |
| LHE information  | `LHEEventProduct`      |                               | `lheEventInfo()`      |
| Generator-level jets  | `reco::GenJet`         | `genJet`                 | `genJets()`           |
| Pruned gen particles  | `reco::GenParticle`    | `genParticle`            | `genParticles()`      |
| Initial states   | `pat::CompositeCandidate`   | `initialState`           | `initialStates()`     |
| Generated initial states  | `pat::CompositeCandidate`  | `genInitialState` | `genInitialStates()` |

So the parameter definitions would look something like
```python
eventParams = cms.PSet(
    eSrc = cms.InputTag('mainElectronCollection'),
    eExtra = cms.PSet(
        collection2 = cms.InputTag('electronCollection2'),
        collection3 = cms.InputTag('electronCollection3'),
        ),
    # ... etc
    )
```
and in a library function the first electron in each collection would be obtained like this:
```c++
const pat::Electron& e1 = evt.electrons()->at(0);
const pat::Electron& e1_2 = evt.electrons("collection2")->at(0);
const pat::Electron& e1_3 = evt.electrons("collection3")->at(0);
```
