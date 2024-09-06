from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class ZZIso(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(ZZIso, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZIso, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'embedding':
            leptonIsoEmbedding = cms.EDProducer(
                "PATLeptonZZIsoEmbedder",
                srcE = step.getObjTag('e'),
                srcMu = step.getObjTag('m'),
                isoDecisionLabel = cms.string(self.getZZIsoLabel()),
                isoValueLabel = cms.string(self.getZZIsoLabel().replace("Pass", "Val")),
                fsrLabel = cms.string(self.getFSRLabel()),
                #fsrElecSelection = cms.string('userFloat("%s") > 0.5'%self.getZZIDLabel()),
                fsrMuonSelection = cms.string('userFloat("%s") > 0.5'%self.getZZIDLabel()),
                eaLabel = cms.string('EffectiveArea'),
                isoConeDRMaxE = cms.double(0.3),
                isoConeDRMaxMu = cms.double(0.3),
                #In 2017 we move to electron BDT that includes isolation. For now this is implemented in the framework by hacking isoCutE
                #value to a large number because the UserFloat ZZIsoPass is being used in other modules such as ZZCategoryEmbedder in "leptonSelector".
                #Need to figure out a smarter way other than having a separate leptonSelector for electrons and muons
                isoCutE = cms.double(99999),
                #Isolation and SIP cuts are no longer needed since these variables are included in the new Muon BDT
                #But I'm keeping this cut for now in case its needed for HighPtMuons(pt>200) with HighPtTrkID though its unlikely
                isoCutMu = cms.double(0.35),
                )
            step.addModule('leptonIsoEmbedding', leptonIsoEmbedding,
                           'e', 'm', e='electrons', m='muons')
            #The isolation cut is applied after recovered FSR photons are subtracted 
            #from the isolation cone (see below) only for muons since electron isolation variables are now included in the MVA!.
            #step.addModule('leptonIsoEmbedding', leptonIsoEmbedding,
            #               'm', m='muons')

        return step

    def getZZIsoLabel(self):
        return 'ZZIsoPass'
