from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

from os import path

class ElectronCalibration(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC', True)
        if not hasattr(self, 'isSync'):
            self.isSync = self.isMC and kwargs.pop('isSync', False)

        if not hasattr(self, 'year'):
            self.year = kwargs.pop('year', '2022')

        if not hasattr(self, 'calibEra22'):
            self.calibEra22 = kwargs.pop('calibEra22', 'preEE')

        eesShift = kwargs.pop('electronScaleShift', 0) if self.isMC else 0
        eerRhoShift = kwargs.pop('electronRhoResShift', 0) if self.isMC else 0
        eerPhiShift = kwargs.pop('electronPhiResShift', 0) if self.isMC else 0
        if not hasattr(self, 'electronScaleShift'):
            self.electronScaleShift = eesShift
        if not hasattr(self, 'electronRhoResShift'):
            self.electronRhoResShift = eerRhoShift
        if not hasattr(self, 'electronPhiResShift'):
            self.electronPhiResShift = eerPhiShift

        super(ElectronCalibration, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ElectronCalibration, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'preliminary':
            if not hasattr(self.process, 'RandomNumberGeneratorService'):
                self.process.RandomNumberGeneratorService = cms.Service(
                    'RandomNumberGeneratorService',
                    )
            LeptonSetup = cms.string(self.year)

            #For Run3: https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun3
            from RecoEgamma.EgammaTools.EgammaPostRecoTools import setupEgammaPostRecoSeq, _defaultEleIDModules

            if LeptonSetup == "2022":
                setupEgammaPostRecoSeq(self.process,
                    runEnergyCorrections=False,
                    runVID=True,
                    era="2022-Prompt",
                    eleIDModules=_defaultEleIDModules + ["RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Winter22_HZZ_V1_cff"]
                )
            step.addModule('egammaPostRecoSeq',self.process.egammaPostRecoSeq)

            seedGainEle = cms.EDProducer(
                "ElectronSeedGainProducer",
                src = step.getObjTag('e')
            )
            step.addModule("seedGainEle", seedGainEle)

            embedSeedGain = cms.EDProducer(
                "PATElectronValueMapEmbedder",
                src = step.getObjTag('e'),
                intLabels = cms.untracked.vstring("seedGain"),
                intVals = cms.untracked.VInputTag("seedGainEle")
            )
            step.addModule("seedGainEmbedding", embedSeedGain, 'e')

            yearstring = ""
            if LeptonSetup == "2022":
                yearstring = "2022_Summer22%s" % ("" if self.calibEra22 == "preEE" else "EE")
            scaleFileP = path.join("/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM",
                                    yearstring, "electronSS.json.gz")
            eCorr = cms.EDProducer(
                "PATElectronCorrector",
                src = step.getObjTag('e'),
                scaleFile = cms.string(scaleFileP),
                isMC = cms.bool(self.isMC)
            )
            step.addModule("calibratedPatElectrons", eCorr, 'e')

            # need to re-sort now that we're calibrated
            eSort = cms.EDProducer(
                "PATElectronCollectionSorter",
                src = step.getObjTag('e'),
                function = cms.string('pt'),
            )
            step.addModule('electronSorting', eSort, 'e')

        return step
