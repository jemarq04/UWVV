from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class GenLeptonBase(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'flag'):
            self.flag = kwargs.pop('leptonStatusFlag', 'fromHardProcessFinalState')
        super(GenLeptonBase, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(GenLeptonBase, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'selection':
            genEMod = cms.EDFilter(
                "GenParticleSelector",
                src = step.getObjTag('e'),
                cut = cms.string("abs(pdgId) == 11 && {}".format(self.flag)),
                )
            step.addModule('genSelectionE', genEMod, 'e')

            genMuMod = cms.EDFilter(
                "GenParticleSelector",
                src = step.getObjTag('m'),
                cut = cms.string("abs(pdgId) == 13 && {}".format(self.flag)),
                )
            step.addModule('genSelectionMu', genMuMod, 'm')

        return step
