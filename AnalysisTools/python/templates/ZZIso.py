from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class ZZIso(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(ZZIso, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZIso, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "embedding":
            leptonIsoEmbedding = cms.EDProducer(
                "PATLeptonIsoEmbedder",
                electrons=step.getObjTag("e"),
                muons=step.getObjTag("m"),
                isoDecisionLabel=cms.string(self.getZZIsoLabel()),
                isoValueLabel=cms.string(self.getZZIsoLabel().replace("Pass", "Val")),
                fsrLabel=cms.string(self.getFSRLabel()),
                rhoLabel=cms.string("rho_fastjet"),
                eaLabel=cms.string("EffectiveArea"),
                # In 2017 we moved to electron BDT that includes isolation. This is implemented in the framework by setting eIsoCut to a large number
                # so that all electrons pass isolation
                eIsoCut=cms.double(9999),
                muIsoCut=cms.double(0.35),
            )
            step.addModule("leptonIsoEmbedding", leptonIsoEmbedding, "e", "m", e="electrons", m="muons")

        return step

    def getZZIsoLabel(self):
        return "ZZIsoPass"
