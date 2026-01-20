from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class MuonGhostCleaning(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(MuonGhostCleaning, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(MuonGhostCleaning, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "preliminary":
            self.addGhostCleaning(step)
        return step

    def addGhostCleaning(self, step):
        """
        Add modules to resolve track ambiguities.
        """
        mod = cms.EDProducer(
            "PATMuonCleanerBySegments",
            src=step.getObjTag("m"),
            preselection=cms.string("track.isNonnull"),
            passthrough=cms.string("isGlobalMuon && numberOfMatches >= 2"),
            fractionOfSharedSegments=cms.double(0.499),
        )

        step.addModule("muonGhostCleaning", mod, "m")
