from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import UWVV_BASE_PATH
import os

import FWCore.ParameterSet.Config as cms

class MuonCalibration(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC', True)
        if not hasattr(self, 'year'):
            self.year = kwargs.pop('year', '2022')
        if not hasattr(self, 'calibEra22'):
            self.calibEra22 = kwargs.pop('calibEra22', 'preEE')
        if not hasattr(self, 'calibEra23'):
            self.calibEra23 = kwargs.pop('calibEra23', 'preBPix')
        super(MuonCalibration, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(MuonCalibration, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'preliminary':
            # Setup/configuration
            yearstring = ""
            if self.year == "2022":
                yearstring = "2022_Summer22%s" % ("" if self.calibEra22 == "preEE" else "EE")
            elif self.year == "2023":
                yearstring = "2023_Summer23%s" % ("" if self.calibEra23 == "preBPix" else "BPix")
            elif self.year == "2024":
                yearstring = "2024_Summer24"
            scaleFile = os.path.join(UWVV_BASE_PATH, "data", "MuonCorrections", "%s.json" % yearstring)

            # Muon corrections
            muCalibrator = cms.EDProducer(
                "PATMuonCorrector",
                src = step.getObjTag('m'),
                isMC = cms.bool(self.isMC),
                scaleFile = cms.string(scaleFile),
                minPt = cms.double(3.), # essentially disabling minimum pt threshold
            )
            step.addModule('calibratedPatMuons', muCalibrator, 'm')

            # need to re-sort now that we're calibrated
            mSort = cms.EDProducer(
                "PATMuonCollectionSorter",
                src = step.getObjTag('m'),
                function = cms.string('pt'),
                )
            step.addModule('muonSorting', mSort, 'm')

        return step
