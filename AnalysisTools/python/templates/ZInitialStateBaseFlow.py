from UWVV.Utilities.helpers import mapObjects, parseChannels

import FWCore.ParameterSet.Config as cms

from UWVV.Utilities.helpers import UWVV_BASE_PATH

class ZInitialStateBaseFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC', True)
        super(ZInitialStateBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZInitialStateBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'initialStateEmbedding':
            #Add modules to embed alternate lepton pair (e.g. e1+m1) info.
            for chan in parseChannels('z'):
                mod = cms.EDProducer(
                    'AlternateDaughterInfoEmbedder',
                    src = step.getObjTag(chan),
                    names = cms.vstring(*mapObjects(chan)),
                    fsrLabel = cms.string(""),#"fsr"),
                    )
                step.addModule(chan+'AlternatePairs', mod, chan)

                #Add modules to embed jet collection in the initial state object
                if self.isMC:
                    #TODO: Wait for Run3 Jet PUSFs to be added to JME POG under jmar.json
                    mod = cms.EDProducer(
                        'CleanedJetCollectionEmbedder',
                        src = step.getObjTag(chan),
                        jetSrc = step.getObjTag('j'),
                        jesUpJetSrc = step.getObjTag('j_jesUp'),
                        jesDownJetSrc = step.getObjTag('j_jesDown'),
                        jerUpJetSrc = step.getObjTag('j_jerUp'),
                        jerDownJetSrc = step.getObjTag('j_jerDown'),
                        domatch = cms.bool(True),
                        scaleFile = cms.string("sfFileNone"),
                    )
                else:
                    mod = cms.EDProducer(
                        'CleanedJetCollectionEmbedder',
                        src = step.getObjTag(chan),
                        jetSrc = step.getObjTag('j'),
                    )
                step.addModule(chan+'CleanedJetsEmbed', mod, chan)

        return step
