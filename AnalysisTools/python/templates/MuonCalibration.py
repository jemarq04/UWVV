from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import UWVV_BASE_PATH

import FWCore.ParameterSet.Config as cms

class MuonCalibration(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC', True)
        if not hasattr(self, 'isSync'):
            self.isSync = self.isMC and kwargs.pop('isSync', False)
        if not hasattr(self, 'year'):
            self.year = kwargs.pop('year', '2022')
        if not hasattr(self, 'muonClosureShift'):
            self.muonClosureShift = kwargs.pop('muonClosureShift', 0) if self.isMC else 0
        if not hasattr(self, 'calibEEera22'):
            self.calibEEera22 = kwargs.pop('calibEEera22', 'preEE')
        super(MuonCalibration, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(MuonCalibration, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'preliminary':
            yearstring = ""
            if self.year == "2022":
                yearstring = "2022_Summer22%s" % ("" if self.calibEEera22 == "preEE" else "EE")
            scaleFile = "%s/data/MuonCorrections/%s.json" % (UWVV_BASE_PATH, yearstring)

            if yearstring:
                muCalibrator = cms.EDProducer(
                    "PATMuonCorrector",
                    src = step.getObjTag('m'),
                    isMC = cms.bool(self.isMC),
                    scaleFile = cms.string(scaleFile),
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








