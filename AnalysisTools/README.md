# Building an analysis flow

UWVV analysis flows are intended to be as modular as possible, to allow multiple analyses to live together in harmony, and to make modifications and additions to analyses as painless as possible. For every discrete task you want to do, like calculating electron IDs and embedding them in the `pat::Electron` objects, you make a Python class that sets up the desired action. These classes are combined into one Flow class that by the magic of multiple inheritance puts all desired EDM modules into several AnalysisStep objects which handle the collection input tags and similar sundries.

## Table of Contents

- [Building and executing a flow](#building-and-executing-a-flow)
- [Analysis steps](#analysis-steps)
   * [List of steps](#list-of-steps)
   * [Using the steps](#using-the-steps)
      + [Get the input tag for a collection](#get-the-input-tag-for-a-collection)
      + [Add a module](#add-a-module)
      + [Auto-generated selectors](#auto-generated-selectors)
   * [Order of modules within a step](#order-of-modules-within-a-step)
- [Flow base classes](#flow-base-classes)
   * [Adding modules](#adding-modules)
   * [Extra arguments and initial input tags](#extra-arguments-and-initial-input-tags)
   * [Example](#example)
   * [Getting the final collection tags](#getting-the-final-collection-tags)
- [Composite states](#composite-states)
   * [Building composite candidates](#building-composite-candidates)
   * [Accessing the daughters](#accessing-the-daughters)


## Building and executing a flow

A Flow class for an analysis is built from multiple base classes with the `createFlow` method. The constructor for this class takes a name and the `cms.Process` it should add modules to as positional arguments. Extra arguments needed by the base classes, and input tags for any nonstandard collections, are passed as constructor keyword arguments.

The following snippet makes a Flow that makes a few simple modifications to the electrons in the event, and schedules the resulting `cms.Path`.

```python
import FWCore.ParameterSet.Config as cms

# CMSSW requires full paths for import
from UWVV.Ntuplizer.templates.ElectronBaseFlow import ElectronBaseFlow
from UWVV.Ntuplizer.templates.ElectronCalibration import ElectronCalibration
from UWVV.Ntuplizer.templates.ElectronScaleFactors import ElectronScaleFactors

from UWVV.Ntuplizer.analysisFlowMaker import createFlow

process = cms.Process("Analysis")
process.schedule = cms.Schedule()

# Make new class combining all desired elements
Flow = createFlow(ElectronBaseFlow, ElectronCalibration, ElectronScaleFactors)

# ElectronCalibration base class needs extra argument isMC
# Electrons are a standard collection, so no extra input tags are needed
flow = Flow('flow', process, isMC=True)

# Path contains all actions from all base classes
p = flow.getPath()

# Could add something else here, like an ntuplizer module or an output module
```


## Analysis steps

The order of analysis actions frequently matters. If you want to cut electrons based on a certain ID, you need to embed that ID first. You need to finish modifying your leptons before building Z candidates out of them. So analysis actions are grouped into several Steps. Each Step keeps track of some information like collection input tags.

### List of steps

A Flow class may define any desired step by inserting it into the list of step names returned by the `listSteps()` method. The default list is

* Object manipulation steps
  * `preliminary` for anything that has to happen before preselection
  * `preselection` for loose cuts so we can run on smaller collections
  * `embedding` for embedding extra variables and making any other modifications to basic objects
  * `selection` for selecting the objects that will be built into composite objects
* Intermediate states
  * `intermediateStateCreation` for making Z candidates and other composite objects
  * `intermediateStateEmbedding` for variable embedding and manipulation of the intermediate composite objects
  * `intermediateStateSelection` for selecting the intermediate composites that will be built into final composite objects
* Initial states
  * `initialStateCreation` for building full composite objects out of intermediate state objects, e.g. building Higgs boson candidates out of Z candidates
  * `initialStateEmbedding` for variable embedding and manipulation of the full composite objects
  * `initialStateSelection` for cuts on the full composite objects

Steps happen in this order. It's fine to do nothing in a step.

### Using the steps

#### Get the input tag for a collection

As a collection in the event gets replaced or updated by its modules, the Step keeps track of the new input tags. It is recommended to always get object tags from the Step, not by hard-coding the collection name.

```python
tag = step.getObjTag(collection)
tagAsString = step.getObjTagString(collection)
```

The collections stored are set in the constructor, and more may be added by producer modules. The collections `AnalysisFlowBase` passes to its first step by default are
* `'e'`: electrons
* `'m'`: muons
* `'j'`: jets
* `'v'`: vertices
* `'pfCands'`: packed particle flow candidates

#### Add a module

Add a module to the step with

```python
Step.addModule(nameForModule, theModule, *objectsOutput, **tagSuffixes)
```
The `objectsOutput` argument tells the Step which collections, if any, are replaced by the output of this module. The appropriate tags are updated with the name of the module. If the module is actually a sequence of modules, the name of the last module is used. In the event that a module puts a suffix on the input tag (usually because it creates multiple collections and needs to differentiate between them), you must tell it what suffix to give to each collection with the `tagSuffixes` arguments. A module which creates an electron collection and a muon collection, and labels them as such, would be added like this

```python
step.addModule('twoOutputModule', twoOutputModule, 'e', 'm', e='electrons', m='muons')
```

If the module creates a new collection that doesn't replace any existing one, the step will start tracking the new collection.

#### Auto-generated selectors

Add a module to do simple cuts from a string on a collection with
```python
step.addBasicSelector(collection, selectionString)
```
So, to apply basic pT and eta cuts to your electrons, do
```python
step.addBasicSelector('e', 'pt>7 && abs(eta) < 2.5')
```

Cross cleaning, i.e. removing objects within a specified delta R of other objects, can also be done automatically
```python
step.addCrossSelector(objectTocut, selectionString, **otherObjects)
```
where each `otherObjects` argument is a dictionary specifying the distance for cross cleaning and a selection for the objects to clean against. To remove all jets that are below 30 GeV or within delta R < 0.4 of a 10 GeV muon
```python
step.addCrossSelector('j', 'pt>30.', m={'deltaR':0.4,'selection':'pt>10.'})
```

If `addBasicSelector()` and `addCrossSelector` receive a collection with a longer name, if an underscore is the second character, it will assume this is a secondary collection of a type given by the first character. For example, `j_jesUp` will be interpreted as a second jet collection used for energy scale systematics estimates. Any other multi-character collection will be assumed to be `CompositeCandidates`.

### Order of modules within a step

The order of modules within a step is determined by the order in which the Flow base classes that embed them are passed into `createFlow`. The first Flow class's modules will be first, the last Flow class's modules will be last. In general, this should not matter, but it comes up on occasion, e.g. when an `edm::ValueMap` is keyed to a specific particle collection and not a copy of the collection.


## Flow base classes

Each action in the analysis flow is represented by a class which derives from `AnalysisFlowBase`.

### Adding modules

The class sets up its modules in a method called `makeAnalysisStep` which should have the following properties:
* Arguments are `stepName`, a string giving the name of the step, and keyword arguments `**inputs` which should be passed along but not otherwise used.
  * This method is called once for every step, so use `stepName` to ensure the action is only done at the appropriate step. See above for the list of standard step names.
  * The `inputs` keyword arguments represent the initial collection input tags for the step. Unless your module happens to be the first one in its step, they will be wrong, so don't use them. The correct input tags for the modules should be retrieved from the AnalysisStep (see above).
* The AnalysisStep to modify is obtained from `super(ThisClass, self).makeAnalysisStep(stepName, **inputs)`.
* Necessary EDM modules are created and added to the step.
  * They do not need to be added to the `Process` here.
* Return the step.

### Extra arguments and initial input tags

Any extra parameters can be passed into the final `Flow` class constructor and retrieved in the base class constructor. The `__init__()` method must call `super().__init__()`. Extra arguments should be removed from the constructor keyword arguments before passing them on to super, as any arguments not removed will be interpreted as input tags for collections in the event stream.

### Example

The following example class embeds some basic IDs into the jet collection.

```python
from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

class JetID(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        '''
        Example of how to get an extra argument
        '''
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC') # remove from kwargs

        super(JetID, self).__init__(*args, **kwargs)


    def makeAnalysisStep(self, stepName, **inputs):
        '''
        Add jet ID embedding to the preliminary step
        '''
        # Get the step to modify
        step = super(JetID, self).makeAnalysisStep(stepName, **inputs)

        # IDs are used in preselection, so they should be embedded in the preliminary step
        if stepName == 'preliminary':

            # Make the module
            jetIDEmbedding = cms.EDProducer(
                "PATJetIDEmbedder",
                src = step.getObjTag('j'), # step handles collection input tags
                )

            # Add the module to the step, and tell it that the output replaces the jet collection
            step.addModule('jetIDEmbedding', jetIDEmbedding, 'j')

        return step
```

### Getting the final collection tags

To use objects after the Flow is completed, you can get the final tags from the Flow

```python
eTag = flow.finalObjTag('e') # final cms.InputTag for the electron collection
mTagString = flow.finalObjTagString('m') # string for final muon collection tag
tagStringDict = flow.finalTags() # all final tags, as strings
```


## Composite states

Intermediate and inital states are represented by `pat::CompositeCandidate` objects made of shallow clones of the daughters.

### Building composite candidates

Composite states are built in modules like this (which makes Z->ee candidates)

```python
zEEMod = cms.EDProducer(
    'PATCandViewShallowCloneCombiner',
    decay = cms.string('{0}@+ {0}@-'.format(step.getObjTagString('e'))),
    roles = cms.vstring('e1', 'e2'),
    cut = cms.string('daughter("e1").pt > 7 && '
                     'daughter("e2").pt > 7 && '
                     'abs(daughter("e1").eta) < 2.5 && '
                     'abs(daughter("e2").eta) < 2.5'),
    checkCharge = cms.bool(True),
    setPdgId = cms.int32(23),
    )

step.addModule('zEECreation', zEEMod, 'ee') # Step now tracks this collection as 'ee'
```

ZZ->4e candidates would then be built like this

```python
for chan in parseChannels('zz'):
    eeeeMod = cms.EDProducer(
        'PATCandViewShallowCloneCombiner',
        decay = cms.string('{0} {0}'.format(step.getObjTagString('ee'))),
        roles = cms.vstring('ze1', 'ze2'),
        cut = cms.string('daughter("ze1").masterClone.mass < 150. && '
                         'daughter("ze2").masterClone.mass < 150.')
        checkCharge = cms.bool(False),
        setPdgId = cms.int32(25),
        )

step.addModule('eeeeProducer', eeeeMod, 'eeee') # Step now tracks this collection as 'eeee'
```

The combiner module automatically checks for daughter overlaps. This is done recursively, so you won't get the same final state object twice in one initial state. If for some reason you don't want this checked, the combiner will accept the option `checkOverlap = cms.bool(False)`. UWVV has a utility to do remove overlaps for 3- and 4-object final states, if you for some reason want to do it yourself.

### Accessing the daughters

As an example, we'll access information from a `pat::Electron` which is the 0th daughter of a Z candidate stored as an `edm::Ptr<pat::CompositeCandidate>`.

If you need only basic information, just use the bare daughter shallow clone
```c++
float pt = cand->daughter(0)->pt();
```

If you need a pointer to the original object
```c++
// if you don't care about type, this works
const reco::Candidate* e1Base = cand->daughter(0)->masterClone().get();
// to get the typed pointer, do this
const pat::Electron* e1 = static_cast<const pat::Electron*>(e1Base); // or dynamic_cast where appropriate
```
The preferred idiom in UWVV is to use templating to guarantee the type rather than the cast-until-it-works method used in some CMS analysis code.

If you need the full EDM reference
```c++
edm::Ptr<pat::Electron> ePtr = cand->daughter(0)->masterClone().castTo<edm::Ptr<pat::Electron> >();
```
