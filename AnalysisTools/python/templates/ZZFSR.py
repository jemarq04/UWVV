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
                "PATObjectFSREmbedder",
                muSrc = step.getObjTag('m'),
                eSrc = step.getObjTag('e'),
                candSrc = step.getObjTag('pfCands'),
                phoSelection = cms.string("pt > 2 && abs(eta) < 2.4"),
                nIsoSelection = cms.string("pt > 0.5"),
                chIsoSelection = cms.string("pt > 0.2"),
                eSelection = cms.string('userFloat("%s") > 0.5'%self.getZZIDLabel()),
                muSelection = cms.string('userFloat("%s") > 0.5'%self.getZZIDLabel()),
                fsrLabel = cms.string(self.getFSRLabel()),
                etPower = cms.double(2.),
                maxDR = cms.double(0.5),
                isoDR = cms.double(0.3),
                nIsoVetoDR = cms.double(0.01),
                chIsoVetoDR = cms.double(0.0001),
                relIsoCut = cms.double(1.8),
                eMuCrossCleaningDR = cms.double(0.05),
                )
            step.addModule('fsrEmbedder', leptonFSREmbedder, 'e', 'm')

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


            jsfFileP = path.join(UWVV_BASE_PATH, 'data', 'jetPUSF',
                               'scalefactorsPUID_81Xtraining.root')

            jeffFileP = path.join(UWVV_BASE_PATH, 'data', 'jetPUSF',
                               'effcyPUID_81Xtraining.root')

            jsfhist = "h2_eff_sf%s_T"%(int(self.year))
            jeffhist = "h2_eff_mc%s_T"%(int(self.year))

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

                jetPUSFEmbedding = cms.EDProducer(
                    "PATJetPUSFEmbedder",
                    src = step.getObjTag('j'),
                    setup = cms.int32(int(self.year)),
                    domatch = cms.bool(self.isMC),
                    jsfFile = cms.string(jsfFileP),
                    jeffFile = cms.string(jeffFileP),
                    SFhistName = cms.string(jsfhist),
                    effhistName = cms.string(jeffhist),
                    )
                #step.addModule('jetPUSFEmbedding', jetPUSFEmbedding)

            if self.isMC: #apply PU id here after calculating PU id SF multiplication factor
                selectionString2 = ('pt > 30. && abs(eta) < 4.7 && '
                               'userFloat("idTight") > 0.5 && (userInt("{}") >= 7||pt>50.)').format(step.getObjTagString('puID'))
                #step.addBasicSelector('j', selectionString2)
            
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
