


import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.SequenceTypes import _ModuleSequenceType

from UWVV.Utilities.helpers import getObjTypes, getObjName

from collections import OrderedDict



class AnalysisStep(object):
    '''
    A class to make a Sequence to run all modules in one step of an analysis
    '''
    def __init__(self, name, suffix='', *args, **initialInputTags):
        self.name = name
        self.suffix = suffix
        self.inputs = initialInputTags
        self.outputs = initialInputTags.copy()

        self.modules = OrderedDict()


    def getObjTag(self, obj):
        '''
        Get the tag that the collection named obj would end with if this step
        were run as-is. Basically, gives you the input tag for the next module
        that will use this collection.
        '''
        return cms.InputTag(self.outputs[obj])


    def getObjTagString(self, obj):
        return self.outputs[obj]


    def addModule(self, name, module, *objectsOutput, **tagSuffixes):
        '''
        Add a module, give it this name, and update the tags of the collections
        listed in objectsOutput.
        If the module is a sequence, the name of the last module is assumed to
        be the desired tag.
        If the module puts a suffix on the collection names (e.g. a module that
        outputs both someMod:muons and someMod:electrons), the suffixes for the
        appropriate objects can be indicated with the keyword arguments,
        e.g. addModule(name, module, 'e', 'm', e='electrons', m='muons')
        '''
        assert name not in self.modules, "Module {} already exists.".format(name)
        self.modules[name] = module

        if objectsOutput:
            if isinstance(module, _ModuleSequenceType):
                try:
                    newTag = module._seq._collection[-1].__str__()
                except AttributeError:
                    if module._seq is None or module._seq._collection is None:
                        raise AttributeError(("I don't know how to extract an "
                                              "input tag from the sequence in "
                                              )+name)
                    raise
            else:
                newTag = name + self.suffix

        for obj in objectsOutput:
            self.outputs[obj] = newTag
        for obj, suffix in tagSuffixes.items():
            self.outputs[obj] = ':'.join([self.outputs[obj], suffix])


    def makeSequence(self, process):
        '''
        Add all modules, and a Sequence that calls them, to the process, and
        return the Sequence
        '''
        seq = cms.Sequence()
        setattr(process, self.name+"Sequence", seq)

        for name, mod in self.modules.items():
            if not hasattr(process, name+self.suffix):
                setattr(process, name+self.suffix, mod)
            if not isinstance(mod, cms.ESSource):
                seq *= mod

        return seq


    def addBasicSelector(self, obj, selection, name='', objectType='',
                         newCollection=''):
        '''
        Add a basic, no-frills string cut selector module for objects
        of collection obj ('e', 'm' etc.).
        objectType should be 'e' or "Electron" or whatever. If it is not
        specified, the name is inferred from the name up to the first '_', e.g.
        'j_jesUp' is interpreted as a jet collection. If this is more than one
        letter, the type is assumed to be pat::CompositeCandidate.
        If 'name' is empty, the module is called <obj>cleaning<stepName>.
        If newCollection is non-empty, a new collection called
        [obj]_[newCollection] is made instead of replacing the collection
        that's already there.
        '''
        inferTypeFrom = objectType
        if not inferTypeFrom:
            inferTypeFrom = obj.split('_')[0]
            if len(inferTypeFrom) > 1:
                inferTypeFrom = 'CompositeCandidate'
        if len(inferTypeFrom) == 1:
            typeName = getObjName(inferTypeFrom, True)
        else:
            typeName = inferTypeFrom
        modName = 'PAT{}RefSelector'.format(typeName)

        mod = cms.EDFilter(
            modName,
            src = self.getObjTag(obj),
            cut = cms.string(selection),
            filter = cms.bool(False),
            )

        collection = obj
        if newCollection:
            collection += '_'+newCollection

        self.addModule(''.join([obj, newCollection, name if name else 'cleaning',
                                self.name]).replace('_',''),
                       mod, collection)


    def addCrossSelector(self, obj, selection, name='', **otherObjects):
        '''
        Add a module to do a string cut and to do delta R cross-cleaning on
        objects of type obj, and any other object collections. The keyword
        arguments specify the other collections to clean with respect to, and
        should be of the form obj:{'selection':'someSelection','deltaR':distance}.
        So to clean objects that have pt<5 GeV or are within 0.4 of any 30+ GeV
        slimmed jet, one would call this funtion like:
            crossSelector('pt > 5.', j={'deltaR':0.4, 'selection':'pt>30','tag':'slimmedJets'})
        If 'name' is empty, the module is called <obj>crossCleaning<stepName>.
        '''
        overlapParams = cms.PSet()
        for obj2, params in otherObjects.items():
            objParams = cms.PSet(
                src=self.getObjTag(obj2),
                algorithm=cms.string('byDeltaR'),
                preselection=cms.string(params.get('selection','')),
                deltaR=cms.double(params.get('deltaR',0.3)),
                checkRecoComponents=cms.bool(False),
                pairCut=cms.string(''),
                requireNoOverlaps=cms.bool(True),
                )
            setattr(overlapParams, getObjName(obj2), objParams)

        mod = cms.EDProducer(
            "PAT{}Cleaner".format(getObjName(obj.split('_')[0], True)),
            src=self.getObjTag(obj),
            preselection=cms.string(selection),
            checkOverlaps = overlapParams,
            finalCut = cms.string(''),
            )

        self.addModule(''.join([obj, name if name else 'crossCleaning',
                                self.name]).replace('_',''), mod, obj)
