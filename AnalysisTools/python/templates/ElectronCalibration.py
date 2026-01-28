from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import getCorrectionFile

import FWCore.ParameterSet.Config as cms


class ElectronCalibration(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, "isMC"):
            self.isMC = kwargs.pop("isMC", True)
        if not hasattr(self, "year"):
            self.year = kwargs.pop("year", "2022")
        if not hasattr(self, "calibEra22"):
            self.calibEra22 = kwargs.pop("calibEra22", "preEE")
        if not hasattr(self, "calibEra23"):
            self.calibEra23 = kwargs.pop("calibEra23", "preBPix")
        super(ElectronCalibration, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ElectronCalibration, self).makeAnalysisStep(stepName, **inputs)

        if stepName == "preliminary":
            if not hasattr(self.process, "RandomNumberGeneratorService"):
                self.process.RandomNumberGeneratorService = cms.Service(
                    "RandomNumberGeneratorService",
                )

            # For Run3: https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun3
            from RecoEgamma.EgammaTools.EgammaPostRecoTools import setupEgammaPostRecoSeq, _defaultEleIDModules

            # Embed MVAs and BDT scores
            # TODO: update 2023-2024 when available
            eleIDModules = _defaultEleIDModules + [
                "RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Summer18UL_ID_ISO_cff",
                "RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Winter22_HZZ_V1_cff",
            ]
            setupEgammaPostRecoSeq(
                self.process,
                runEnergyCorrections=False,
                runVID=True,
                era="2022-Prompt",
                eleIDModules=eleIDModules,
            )
            step.addModule("egammaPostRecoSeq", self.process.egammaPostRecoSeq)

            if not self.isMC:
                # Produce and embed seed gain into electrons
                seedGainEle = cms.EDProducer("ElectronSeedGainProducer", src=step.getObjTag("e"))
                step.addModule("seedGainEle", seedGainEle)

                embedSeedGain = cms.EDProducer(
                    "PATElectronValueMapEmbedder",
                    src=step.getObjTag("e"),
                    intLabels=cms.untracked.vstring("seedGain"),
                    intVals=cms.untracked.VInputTag("seedGainEle"),
                )
                step.addModule("seedGainEmbedding", embedSeedGain, "e")

            # Setup/configuration
            yearstring = self.year
            if self.year == "2022" and self.calibEra22 == "postEE":
                yearstring += "EE"
            elif self.year == "2023" and self.calibEra23 == "postBPix":
                yearstring += "BPix"
            elif self.year == "2025":
                yearstring = "2025"
            scaleFile = getCorrectionFile("EGM", yearstring, "electronSS_EtDependent.json.gz")

            # Electron corrections
            eCorr = cms.EDProducer(
                "PATElectronCorrector",
                src=step.getObjTag("e"),
                scaleFile=cms.string(scaleFile),
                isMC=cms.bool(self.isMC),
                seedGainLabel=cms.string("seedGain"),
                minPt=cms.double(3.0),  # essentially disabling minimum pt threshold
            )
            step.addModule("calibratedPatElectrons", eCorr, "e")

            # need to re-sort now that we're calibrated
            eSort = cms.EDProducer(
                "PATElectronCollectionSorter",
                src=step.getObjTag("e"),
                function=cms.string("pt"),
            )
            step.addModule("electronSorting", eSort, "e")

        return step
