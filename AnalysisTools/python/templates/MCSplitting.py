from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class MCSplitting(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, "year"):
            self.year = kwargs.pop("year", "2024")
        if not hasattr(self, "isMC"):
            self.isMC = kwargs.pop("isMC", True)
        if not hasattr(self, "yearsWithSameMC"):
            self.yearsWithSameMC = kwargs.pop("yearsWithSameMC", ["2024", "2025"])
        super(MCSplitting, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(MCSplitting, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "preselection" and self.isMC:
            evtSplitter = cms.EDFilter(
                "NEventFilter",
                numGroups=cms.int32(len(self.yearsWithSameMC)),
                group=cms.int32(self.yearsWithSameMC.index(self.year)),
            )
            step.addModule("mcEventSplitter", evtSplitter)

        return step
