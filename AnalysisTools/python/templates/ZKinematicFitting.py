from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class ZKinematicFitting(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, "isMC"):
            self.isMC = kwargs.pop("isMC", True)
        super(ZKinematicFitting, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZKinematicFitting, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "initialStateEmbedding":
            kinFit4e = cms.EDProducer(
                "ZKinematicFitEmbedderEEEE",
                src=step.getObjTag("eeee"),
                isMC=cms.bool(self.isMC),
                fsrLabel=cms.string(self.getFSRLabel()),
                leptonSelection=cms.string(
                    'userFloat("%sTight") > 0.5 && userFloat("%s") > 0.5' % (self.getZZIDLabel(), self.getZZIsoLabel())
                ),
            )
            step.addModule("kinFit4e", kinFit4e, "eeee")

            kinFit2e2m = cms.EDProducer(
                "ZKinematicFitEmbedderEEMM",
                src=step.getObjTag("eemm"),
                isMC=cms.bool(self.isMC),
                fsrLabel=cms.string(self.getFSRLabel()),
                leptonSelection=cms.string(
                    'userFloat("%sTight") > 0.5 && userFloat("%s") > 0.5' % (self.getZZIDLabel(), self.getZZIsoLabel())
                ),
            )
            step.addModule("kinFit2e2m", kinFit2e2m, "eemm")

            kinFit4m = cms.EDProducer(
                "ZKinematicFitEmbedderMMMM",
                src=step.getObjTag("mmmm"),
                isMC=cms.bool(self.isMC),
                fsrLabel=cms.string(self.getFSRLabel()),
                leptonSelection=cms.string(
                    'userFloat("%sTight") > 0.5 && userFloat("%s") > 0.5' % (self.getZZIDLabel(), self.getZZIsoLabel())
                ),
            )
            step.addModule("kinFit4m", kinFit4m, "mmmm")

        return step
