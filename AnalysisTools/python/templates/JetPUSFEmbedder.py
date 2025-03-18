from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import mapObjects, parseChannels

import FWCore.ParameterSet.Config as cms

from os import path

class ZZJetPUSFEmbedder(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        super(ZZJetPUSFEmbedder, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZJetPUSFEmbedder, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "initialStateEmbedding":
            #TODO: Wait for Run3 Jet PUSFs to be added to JME POG under jmar.json
            for chan in parseChannels('zz'):
                try:
                    mod = cms.EDProducer(
                        "TempCleanedJetCollectionEmbedder",
                        src = step.getObjTag(chan),
                        jetSrc = step.getObjTag('j'),
                        jesUpJetSrc = step.getObjTag('j_jesUp'),
                        jesDownJetSrc = step.getObjTag('j_jesDown'),
                        jerUpJetSrc = step.getObjTag('j_jerUp'),
                        jerDownJetSrc = step.getObjTag('j_jerDown'),
                    )
                except KeyError:
                    mod = cms.EDProducer(
                        "TempCleanedJetCollectionEmbedder",
                        src = step.getObjTag(chan),
                        jetSrc = step.getObjTag('j'),
                    )
                step.addModule(chan+'CleanedJetsEmbed', mod, chan)

        return step
