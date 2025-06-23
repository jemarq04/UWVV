from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import UWVV_BASE_PATH

from os import path

import FWCore.ParameterSet.Config as cms


class GGHZZKFactors(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(GGHZZKFactors, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(GGHZZKFactors, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'initialStateEmbedding':
            sfFile = path.join(UWVV_BASE_PATH, 'data', 'kFactors',
                               'Kfactor_Collected_ggHZZ_2l2l_NNLO_NNPDF_NarrowWidth_13TeV.root')

            ggKEmbedding4e = cms.EDProducer(
                "GGHZZKFactorEmbedderEEEE",
                src = step.getObjTag('eeee'),
                fileName = cms.string(sfFile),
                )
            step.addModule('ggKEmbedding4e', ggKEmbedding4e, 'eeee')

            ggKEmbedding2e2m = cms.EDProducer(
                "GGHZZKFactorEmbedderEEMuMu",
                src = step.getObjTag('eemm'),
                fileName = cms.string(sfFile),
                )
            step.addModule('ggKEmbedding2e2m', ggKEmbedding2e2m, 'eemm')

            ggKEmbedding4m = cms.EDProducer(
                "GGHZZKFactorEmbedderMuMuMuMu",
                src = step.getObjTag('mmmm'),
                fileName = cms.string(sfFile),
                )
            step.addModule('ggKEmbedding4m', ggKEmbedding4m, 'mmmm')

        return step
