from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

class EFTBaseFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(EFTBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(EFTBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'initialStateEmbedding':
            EFTfit = cms.EDProducer(
                "EFTfitCoefficientsProducer",
                lheInfo = cms.InputTag("")
            )
            step.addModule("EFTfit", EFTfit)

        return step
