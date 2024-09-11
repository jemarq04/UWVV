


import FWCore.ParameterSet.Config as cms

from UWVV.AnalysisTools.AnalysisStep import AnalysisStep

from collections import OrderedDict
import pdb


class AnalysisFlowBase(object):
    def __init__(self, name, process=None, suffix='', *args, **initialInputs):
        '''
        Keyword arguments are interpreted as changes from the default
        initial object input tags.
        '''
        self.name = name
        self.suffix = suffix

        self.inputs = self.getInitialInputs(**initialInputs)
        self.outputs = []

        if process is None:
            self.process = cms.Process(name)
            self.process.schedule = cms.Schedule()
        else:
            self.process = process
            if process.schedule is None:
                self.process.schedule = cms.Schedule()

        self.steps = OrderedDict()
        for iStep, step in enumerate(self.listSteps()):
            if iStep:
                nextInputs = self.outputs[-1]
            else:
                nextInputs = self.inputs

            self.steps[step] = self.makeAnalysisStep(step, **nextInputs)

            self.outputs.append(self.steps[step].outputs.copy())
        #pdb.set_trace()
        self.path = self.setupPath()


    def getInitialInputs(self, **tags):
        '''
        Initial list of input tags.
        There are some defaults, which maybe be overridden by keyword arguments.
        '''
        self.inheritGuard('getInitialInputs')
        
        out = {
            'e' : 'slimmedElectrons',
            'm' : 'slimmedMuons',
            'a' : 'slimmedPhotons',
            'j' : 'slimmedJets',
            'v' : 'offlineSlimmedPrimaryVertices',
            'pfCands' : 'packedPFCandidates',
            }

        out.update(tags)

        return out
    

    def listSteps(self):
        '''
        Make a list of what steps need to occur. Analysis flows can add or
        modify.
        '''
        self.inheritGuard('listSteps')

        return ['preliminary', 'preselection', 'embedding', 'selection',
                'intermediateStateCreation', 'intermediateStateEmbedding',
                'intermediateStateSelection',
                'initialStateCreation', 'initialStateEmbedding', 
                'initialStateSelection']


    def makeAnalysisStep(self, step, **inputs):
        '''
        Make an empty AnalysisStep, which flows can fill with modules.
        '''
        self.inheritGuard('makeAnalysisStep')

        return AnalysisStep(self.name + step, self.suffix, **inputs)

    
    def setupPath(self):
        '''
        Set up a cms.Path with all analysis steps, add it to the Process, and
        return it
        '''
        p = cms.Path()
        for stepName, step in self.steps.items():
            p *= step.makeSequence(self.process)


        self.process.schedule.append(p)
        setattr(self.process, self.name+'FlowPath', p)

        return p


    def getProcess(self):
        return self.process


    def getPath(self):
        return self.path
    

    def inheritGuard(self, fName):
        '''
        Make sure our inheritance tree makes sense, i.e. that this base
        class really is the base. Do that by making sure that 
        super(AnalysisFlowBase, self).fName doesn't exist.
        '''
        assert not hasattr(super(AnalysisFlowBase, self), fName), \
            ("Analysis flow class {} does not derive from AnalysisFlowBase. "
             "Something is wrong.").format(super(AnalysisFlowBase, self).__class__.__name__)

    def finalObjTag(self, obj):
        return cms.InputTag(self.outputs[-1][obj])

    def finalObjTagString(self, obj):
        return self.outputs[-1][obj]
    
    def finalTags(self):
        return self.outputs[-1]
