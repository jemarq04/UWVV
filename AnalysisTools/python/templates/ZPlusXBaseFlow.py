from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

class ZPlusXBaseFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'debug'):
            self.debug = kwargs.pop('debug', False)
        super(ZPlusXBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZPlusXBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'intermediateStateCreation':
            zEEMod = cms.EDProducer(
                'PATCandViewShallowCloneCombiner',
                decay = cms.string('{0}@+ {0}@-'.format(step.getObjTagString('e'))),
                roles = cms.vstring('e1', 'e2'),
                cut = cms.string(self.__class__.getZEECuts()),
                checkCharge = cms.bool(True),
                setPdgId = cms.int32(23),
                )

            zMuMuMod = cms.EDProducer(
                'PATCandViewShallowCloneCombiner',
                decay = cms.string('{0}@+ {0}@-'.format(step.getObjTagString('m'))),
                roles = cms.vstring('m1', 'm2'),
                cut = cms.string(self.__class__.getZMMCuts()),
                checkCharge = cms.bool(True),
                setPdgId = cms.int32(23),
                )

            step.addModule("zEECreation", zEEMod, 'ee')
            step.addModule("zMuMuCreation", zMuMuMod, 'mm')
            if self.debug:
                step.addBasicCounter('ee', nEleZs="")
                step.addBasicCounter('mm', nMuZs="")

        return step

    @classmethod
    def getZEECuts(cls):
        return ('daughter("e1").pt > 7 && '
                'daughter("e2").pt > 7 && '
                'abs(daughter("e1").eta) < 2.5 && '
                'abs(daughter("e2").eta) < 2.5')


    @classmethod
    def getZMMCuts(cls):
        return ('daughter("m1").pt > 5 && '
                'daughter("m2").pt > 5 && '
                'abs(daughter("m1").eta) < 2.4 && '
                'abs(daughter("m2").eta) < 2.4')


class ZPlusXBaseFlowGen(ZPlusXBaseFlow):
    @classmethod
    def getZEECuts(cls):
        return ('daughter("e1").pt > 5 && '
                'daughter("e2").pt > 5 && '
                'abs(daughter("e1").eta) < 2.5 && '
                'abs(daughter("e2").eta) < 2.5')


    @classmethod
    def getZMMCuts(cls):
        return ('daughter("m1").pt > 5 && '
                'daughter("m2").pt > 5 && '
                'abs(daughter("m1").eta) < 2.5 && '
                'abs(daughter("m2").eta) < 2.5')
