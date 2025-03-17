from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

class ElectronBaseFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'debug'):
            self.debug = kwargs.pop('debug', False)
        super(ElectronBaseFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ElectronBaseFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'preselection':
            if self.debug:
                step.addBasicCounter('e', 
                    nElectrons="",
                    nPreselElectrons="pt > 5 && abs(eta) < 2.6",
                )
            step.addBasicSelector('e', 'pt > 5 && abs(eta) < 2.6', 'preselection')

        if stepName == 'embedding':
            self.addElectronEAEmbedding(step)
            self.addElectronRhoEmbedding(step)

        return step


    def addElectronEAEmbedding(self, step):
        mod = cms.EDProducer(
            'PATElectronEAEmbedder',
            src = step.getObjTag('e'),
            label = cms.string('EffectiveArea'),
            configFile = cms.FileInPath('RecoEgamma/ElectronIdentification/data/Spring15/effAreaElectrons_cone03_pfNeuHadronsAndPhotons_25ns.txt'),
            )

        step.addModule('electronEAEmbedding', mod, 'e')

    def addElectronRhoEmbedding(self, step):
        mod = cms.EDProducer(
            "PATElectronValueEmbedder",
            src = step.getObjTag('e'),
            doubleLabels = cms.vstring('rho_fastjet'),
            doubleSrc = cms.VInputTag(cms.InputTag("fixedGridRhoFastjetAll")),
            )

        step.addModule('electronRhoEmbedding', mod, 'e')

