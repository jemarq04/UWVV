from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import parseChannels

import FWCore.ParameterSet.Config as cms


class GenZZXsecFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isDressed'):
            self.isDressed = kwargs.pop('isDressed', False)
        super(GenZZXsecFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(GenZZXsecFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'initialStateSelection':
            for chan in parseChannels('zz'):
                xsecMod = cms.EDAnalyzer(
                    "GenZZXsecAnalyzer",
                    electrons = step.getObjTag('e'),
                    muons = step.getObjTag('m'),
                    ZZ = step.getObjTag(chan),
                    dressed = cms.bool(self.isDressed)
                )
                step.addModule(f'{chan}Xsec', xsecMod, chan)

        return step
