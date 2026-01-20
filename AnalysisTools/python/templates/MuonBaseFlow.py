from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class MuonBaseFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, "debug"):
            self.debug = kwargs.pop("debug", False)
        super(MuonBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(MuonBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "preselection":
            if self.debug:
                step.addBasicCounter(
                    "m",
                    nMuons="",
                    nPreselMuons="pt > 5 && (isGlobalMuon || isTrackerMuon)",
                )
            step.addBasicSelector("m", "pt > 5 && (isGlobalMuon || isTrackerMuon)", "preselection")

        elif stepName == "embedding":
            embedMuId = cms.EDProducer("PATMuonIDEmbedder", src=step.getObjTag("m"), vertexSrc=step.getObjTag("v"))
            step.addModule("muonIDembedding", embedMuId, "m")

        return step
