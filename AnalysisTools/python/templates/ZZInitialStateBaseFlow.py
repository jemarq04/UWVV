from UWVV.AnalysisTools.templates.ZPlusXBaseFlow import ZPlusXBaseFlow
from UWVV.Utilities.helpers import mapObjects, parseChannels

import FWCore.ParameterSet.Config as cms

from UWVV.Utilities.helpers import UWVV_BASE_PATH

class ZZInitialStateBaseFlow(ZPlusXBaseFlow):
    def __init__(self, *args, **kwargs):
        super(ZZInitialStateBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZInitialStateBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'initialStateCreation':
            self.addZZCreation(step)

        if stepName == 'initialStateEmbedding':
            self.addAlternatePairInfo(step)

        return step


    def addZZCreation(self, step):
        '''
        Add modules to combine Zs into 4l candidates
        '''
        for chan in parseChannels('zz'):
            z1Name = 'z{}1'.format(chan[0])
            z2Name = 'z{}{}'.format(chan[2], 2 if chan[0] == chan[2] else 1)
            mod = cms.EDProducer(
                'PATCandViewShallowCloneCombiner',
                decay = cms.string('{0} {1}'.format(step.getObjTagString(chan[:2]),
                                                    step.getObjTagString(chan[2:]))),
                roles = cms.vstring(z1Name, z2Name),
                cut = cms.string(('daughter("{}").masterClone.mass < 150. && '
                                  'daughter("{}").masterClone.mass < 150.').format(z1Name, z2Name)),
                checkCharge = cms.bool(False),
                setPdgId = cms.int32(25),
                )
            
            step.addModule(chan+'Producer', mod, chan)


    def addAlternatePairInfo(self, step):
        '''
        Add modules to embed alternate lepton pair (e.g. e1+m1) info.
        '''
        for chan in parseChannels('zz'):
            mod = cms.EDProducer(
                'AlternateDaughterInfoEmbedder',
                src = step.getObjTag(chan),
                names = cms.vstring(*mapObjects(chan)),
                fsrLabel = cms.string("fsr"),
                )
            step.addModule(chan+'AlternatePairs', mod, chan)
