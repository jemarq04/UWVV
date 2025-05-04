from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

from os import path,environ

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
        if not hasattr(self, 'calibEra23'):
            self.calibEra23 = kwargs.pop('calibEra23', 'preBPix')

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

            #For Run3: https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun3
            from RecoEgamma.EgammaTools.EgammaPostRecoTools import setupEgammaPostRecoSeq, _defaultEleIDModules

            # Embed MVAs and BDT scores
            if self.year in ["2022", "2023"]:
                # TODO: update 2023 when available
                eleIDModules = _defaultEleIDModules
                if int(environ["CMSSW_VERSION"].split("_")[1]) >= 14:
                    eleIDModules += ["RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Winter22_HZZ_V1_cff"]
                setupEgammaPostRecoSeq(self.process,
                    runEnergyCorrections=False,
                    runVID=True,
                    era="2022-Prompt",
                    eleIDModules=eleIDModules,
                )
            step.addModule('egammaPostRecoSeq',self.process.egammaPostRecoSeq)

            # Produce and embed seed gain into electrons
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

            # Setup/configuration
            yearstring = scaleConfig = smearConfig = ""
            if self.year == "2022":
                yearstring = "2022_Summer22%s" % ("" if self.calibEra22 == "preEE" else "EE")
                scaleConfig = "Scale"
                smearConfig = "Smearing"
            elif self.year == "2023":
                yearstring = "2023_Summer23%s" % ("" if self.calibEra23 == "preBPix" else "BPix")
                # TODO: update configs below when 2023D is available
                scaleConfig = "2023PromptC_ScaleJSON"
                smearConfig = "2023PromptC_SmearingJSON"
            scaleFileP = path.join("/cvmfs/cms.cern.ch/rsync/cms-nanoAOD/jsonpog-integration/POG/EGM",
                                    yearstring, "electronSS.json.gz")

            # Electron corrections
            eCorr = cms.EDProducer(
                "PATElectronCorrector",
                src = step.getObjTag('e'),
                scaleFile = cms.string(scaleFileP),
                isMC = cms.bool(self.isMC),
                scaleConfig = cms.string(scaleConfig),
                smearConfig = cms.string(smearConfig),
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
