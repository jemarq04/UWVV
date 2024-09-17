from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class VertexCleaning(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(VertexCleaning, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(VertexCleaning, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'preselection':
            vtxCleaner = cms.EDFilter(
                "VertexSelector",
                src = step.getObjTag('v'),
                cut = cms.string('!isFake && ndof > 4 && abs(z) <= 24 && position.Rho <= 2'),
                filter = cms.bool(True),
                )

            step.addModule("vertexCleaning", vtxCleaner, 'v')

        return step

