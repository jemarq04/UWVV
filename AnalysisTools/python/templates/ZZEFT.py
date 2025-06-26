from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

class ZZEFT(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(ZZEFT, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZEFT, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'initialStateEmbedding':
            for chan in parseChannels('zz'):
                eftEmbed = cms.EDProducer(
                    "PATCompositeCandidateVectorEmbedder",
                    src = step.getObjTag(chan),
                    doubleLabels = cms.vstring('EFTfitCoefficients'),
                    doubleSrc = cms.VInputTag(cms.InputTag("EFTfit")),
                )
                step.addModule(chan + "EFTEmbedding", eftEmbed, chan)

        return step
