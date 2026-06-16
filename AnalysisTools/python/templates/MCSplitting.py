from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class MCSplitting(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, "year"):
            self.year = kwargs.pop("year", "2024")
        super(MCSplitting, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(MCSplitting, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "preselection":
            years = ["2024", "2025"]

            evtSplitter = cms.EDFilter(
                "NEventFilter",
                numGroups=cms.int32(len(years)),
                group=cms.int32(years.index(self.year)),
            )
            step.addModule("mcEventSplitter", evtSplitter)

        return step
