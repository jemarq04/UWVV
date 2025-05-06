from UWVV.AnalysisTools.templates.WZLeptonCounters import WZLeptonCounters
from UWVV.AnalysisTools.templates.WZID import WZID

class WZFlow(WZLeptonCounters, WZID):
    def __init__(self, *args, **kwargs):
        super(WZFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        return super(WZFlow, self).makeAnalysisStep(stepName, **inputs)
