from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms


class DressedGenLeptonBase(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'flag'):
            self.flag = kwargs.pop('leptonStatusFlag', 'fromHardProcessFinalState')
        super(DressedGenLeptonBase, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(DressedGenLeptonBase, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'selection':
            promptPhotonsMod = cms.EDFilter("GenParticleSelector",
                src = step.getObjTag('a'),
                cut = cms.string("pdgId = 22 && statusFlags().isPrompt() && status() == 1")
            )
            step.addModule('promptPhotonsMod', promptPhotonsMod, 'a')

            genEMod = cms.EDFilter(
                "GenParticleSelector",
                src = step.getObjTag('e'),
                cut = cms.string("abs(pdgId) == 11 && {}".format(self.flag)),
                )
            step.addModule('genSelectionE', genEMod, 'e')

            dressedGenEMod = cms.EDProducer("DressedGenParticlesProducer",
                baseCollection = step.getObjTag('e'),
                associates = step.getObjTag('a'),
                dRmax = cms.untracked.double(0.1)
            )
            step.addModule('dressedElectrons', dressedGenEMod, 'e')

            genMuMod = cms.EDFilter(
                "GenParticleSelector",
                src = step.getObjTag('m'),
                cut = cms.string("abs(pdgId) == 13 && {}".format(self.flag)),
                )
            step.addModule('genSelectionMu', genMuMod, 'm')

            dressedGenMuMod = cms.EDProducer("DressedGenParticlesProducer",
                baseCollection = step.getObjTag('m'),
                associates = step.getObjTag('a'),
                dRmax = cms.untracked.double(0.1)
            )
            step.addModule('dressedMuons', dressedGenMuMod, 'm')

        return step
