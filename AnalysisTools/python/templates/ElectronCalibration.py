from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

class ElectronCalibration(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC', True)
        if not hasattr(self, 'isSync'):
            self.isSync = self.isMC and kwargs.pop('isSync', False)

        if not hasattr(self, 'year'):
            self.year = kwargs.pop('year', '2022')

        if not hasattr(self, 'calibEEera22'):
            self.calibEEera22 = kwargs.pop('calibEEera22', 'preEE')

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
            #TODO: Determine Run3 Electron Calibrations
            # At the moment, EgammaPostRecoTools does not support above CMSSW 12
            # (even though they recommend using >= CMSSW_13_X ...)
            '''
            if not hasattr(self.process, 'RandomNumberGeneratorService'):
                self.process.RandomNumberGeneratorService = cms.Service(
                    'RandomNumberGeneratorService',
                    )
            LeptonSetup = cms.string(self.year)

            #For Run3: https://twiki.cern.ch/twiki/bin/view/CMS/MultivariateElectronIdentificationRun3
            from RecoEgamma.EgammaTools.EgammaPostRecoTools import setupEgammaPostRecoSeq, _defaultEleIDModules

            if LeptonSetup == "2022":
                setupEgammaPostRecoSeq(self.process,
                    runEnergyCorrections=True,
                    runVID=True,
                    era="2022-Prompt",
                    eleIDModules=_defaultEleIDModules \
                        + ['RecoEgamma.ElectronIdentification.Identification.mvaElectronID_RunIIIWinter22_iso_V1_cff',
                           'RecoEgamma.ElectronIdentification.Identification.mvaElectronID_RunIIIWinter22_noIso_V1_cff',
                           'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Winter22_122X_V1_cff']
                )
            step.addModule('egammaPostRecoSeq',self.process.egammaPostRecoSeq)
            '''

        if stepName == 'selection':
            # need to re-sort now that we're calibrated
            eSort = cms.EDProducer(
                "PATElectronCollectionSorter",
                src = step.getObjTag('e'),
                function = cms.string('pt'),
                )
            step.addModule('electronSorting', eSort, 'e')

        return step
