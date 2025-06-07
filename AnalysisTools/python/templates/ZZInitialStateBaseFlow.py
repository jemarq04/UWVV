from UWVV.AnalysisTools.templates.ZPlusXBaseFlow import ZPlusXBaseFlow
from UWVV.Utilities.helpers import mapObjects, parseChannels

import FWCore.ParameterSet.Config as cms

from UWVV.Utilities.helpers import UWVV_BASE_PATH

class ZZInitialStateBaseFlow(ZPlusXBaseFlow):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC', True)
        super(ZZInitialStateBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZInitialStateBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'initialStateCreation':
            #Add modules to combine Zs into 4l candidates
            for chan in parseChannels('zz'):
                z1Name = 'z{}1'.format(chan[0])
                z2Name = 'z{}{}'.format(chan[2], 2 if chan[0] == chan[2] else 1)
                mod = cms.EDProducer(
                    'PATCandViewShallowCloneCombiner',
                    decay = cms.string('{0} {1}'.format(step.getObjTagString(chan[:2]),
                                                        step.getObjTagString(chan[2:]))),
                    roles = cms.vstring(z1Name, z2Name),
                    cut = cms.string(('daughter("{}").masterClone.mass < 150. && '
                                      'daughter("{}").masterClone.mass < 150.').format(z1Name, z2Name)),
                    checkCharge = cms.bool(False),
                    setPdgId = cms.int32(25),
                )
                
                step.addModule(chan+'Producer', mod, chan)

        elif stepName == 'initialStateEmbedding':
            for chan in parseChannels('zz'):
                #Add modules to embed alternate lepton pair (e.g. e1+m1) info.
                mod = cms.EDProducer(
                    'AlternateDaughterInfoEmbedder',
                    src = step.getObjTag(chan),
                    names = cms.vstring(*mapObjects(chan)),
                    fsrLabel = cms.string("fsr"),
                )
                step.addModule(chan+'AlternatePairs', mod, chan)

                # embed alternate Z candidate masses (Za, Zb) for "smart cut" in 4e and 4mu
                if chan == chan[0]*len(chan):
                    mod = cms.EDProducer(
                        "AlternateZZPairMassEmbedder",
                        src = step.getObjTag(chan),
                        names = cms.vstring(*mapObjects(chan)),
                    )
                    step.addModule(chan+"AlternateZZPairMass", mod, chan)

                #Add modules to embed jet collection in the initial state object
                if self.isMC:
                    #TODO: Wait for Run3 Jet PUSFs to be added to JME POG under jmar.json
                    mod = cms.EDProducer(
                        "CleanedJetCollectionEmbedder",
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
                        "CleanedJetCollectionEmbedder",
                        src = step.getObjTag(chan),
                        jetSrc = step.getObjTag('j'),
                    )
                step.addModule(chan+'CleanedJetsEmbed', mod, chan)

        return step
