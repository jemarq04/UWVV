from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase


class NoCombinedStates(AnalysisFlowBase):
    """
    A class that prevents composite intermediate/initial states.
    """

    def __init__(self, *args, **kwargs):
        super(NoCombinedStates, self).__init__(*args, **kwargs)

    def listSteps(self):
        """
        Remove any steps that have "intermediateState" or "initialState" in them
        (case is ignored).
        """
        steps = super(NoCombinedStates, self).listSteps()

        newSteps = []
        for s in steps:
            if "intermediatestate" not in s.lower() and "initialstate" not in s.lower():
                newSteps.append(s)

        return newSteps
