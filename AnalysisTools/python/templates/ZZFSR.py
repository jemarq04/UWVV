from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.AnalysisTools.templates.ZPlusXBaseFlow import ZPlusXBaseFlow
from UWVV.AnalysisTools.templates.ZZInitialStateBaseFlow import ZZInitialStateBaseFlow

import FWCore.ParameterSet.Config as cms

from UWVV.Utilities.helpers import UWVV_BASE_PATH
import os
from os import path


class ZZFSR(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC', True)
        super(ZZFSR, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZFSR, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'embedding':
            leptonFSREmbedder = cms.EDProducer(
                "PATLeptonFSREmbedder",
                candidates = step.getObjTag('pfCands'),
                electrons = step.getObjTag('e'),
                muons = step.getObjTag('m'),
                electronsForVeto = cms.InputTag("slimmedElectrons"),
                fsrLabel = cms.string(self.getFSRLabel()),
                eCut = cms.string('userFloat("%s") > 0.5' % self.getZZIDLabel()),
                muCut= cms.string('userFloat("%s") > 0.5' % self.getZZIDLabel()),
            )
            step.addModule('fsrEmbedder', leptonFSREmbedder, 'e', 'm', e='electrons', m='muons')

        if stepName == 'selection':
            jetFSRCleaner = cms.EDProducer(
                'PATJetFSRCleaner',
                src = step.getObjTag('j'),
                srcE = step.getObjTag('e'),
                srcMu = step.getObjTag('m'),
                fsrLabel = cms.string(self.getFSRLabel()),
                fsrElecSelection = cms.string('userFloat("%sTight") > 0.5 && userFloat("%s") > 0.5'%(self.getZZIDLabel(), self.getZZIsoLabel())),
                fsrMuonSelection = cms.string('userFloat("%sTight") > 0.5 && userFloat("%s") > 0.5'%(self.getZZIDLabel(), self.getZZIsoLabel())),
                )
            step.addModule('jetFSRCleaner', jetFSRCleaner, 'j')


            if self.isMC:
                patJetGenJetMatch2 = cms.EDProducer("GenJetMatcher",  # cut on deltaR; pick best by deltaR
                src         = step.getObjTag('j'),                    # RECO jets (any View<Jet> is ok)
                matched     = cms.InputTag("slimmedGenJets"),        # GEN jets  (must be GenJetCollection)
                mcPdgId     = cms.vint32(),                      # n/a
                mcStatus    = cms.vint32(),                      # n/a
                checkCharge = cms.bool(False),                   # n/a
                maxDeltaR   = cms.double(0.4),                   # Minimum deltaR for the match
                #maxDPtRel   = cms.double(3.0),                  # Minimum deltaPt/Pt for the match (not used in GenJetMatcher)
                resolveAmbiguities    = cms.bool(True),          # Forbid two RECO objects to match to the same GEN object
                resolveByMatchQuality = cms.bool(False),         # False = just match input in order; True = pick lowest deltaR pair first
                )
                
                step.addModule("patJetGenJetMatch2",patJetGenJetMatch2) #store RECO/gen jet association in the event

            if self.isMC:
                jetFSRCleaner_jesUp = jetFSRCleaner.clone(src = step.getObjTag('j_jesUp'))
                step.addModule('jetFSRCleanerJESUp', jetFSRCleaner_jesUp, 'j_jesUp')
                jetFSRCleaner_jesDown = jetFSRCleaner.clone(src = step.getObjTag('j_jesDown'))
                step.addModule('jetFSRCleanerJESDown', jetFSRCleaner_jesDown, 'j_jesDown')
                jetFSRCleaner_jerUp = jetFSRCleaner.clone(src = step.getObjTag('j_jerUp'))
                step.addModule('jetFSRCleanerJERUp', jetFSRCleaner_jerUp, 'j_jerUp')
                jetFSRCleaner_jerDown = jetFSRCleaner.clone(src = step.getObjTag('j_jerDown'))
                step.addModule('jetFSRCleanerJERDown', jetFSRCleaner_jerDown, 'j_jerDown')

        if stepName == 'intermediateStateEmbedding':
            if isinstance(self, ZPlusXBaseFlow):
                zeFSREmbedder = cms.EDProducer(
                    'PATElectronCompositeUserCandPromoter',
                    src = step.getObjTag('ee'),
                    label = cms.string(self.getFSRLabel()),
                    )
                zmFSREmbedder = cms.EDProducer(
                    'PATMuonCompositeUserCandPromoter',
                    src = step.getObjTag('mm'),
                    label = cms.string(self.getFSRLabel()),
                    )

                step.addModule('zeFSREmbedder', zeFSREmbedder, 'ee')
                step.addModule('zmFSREmbedder', zmFSREmbedder, 'mm')

        return step

    def getFSRLabel(self):
        return "fsr"
