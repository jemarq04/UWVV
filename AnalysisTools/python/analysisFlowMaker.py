from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase


def createFlow(*bases):
    '''
    Given a list of classes inheriting from AnalysisFlowBase, construct a class
    that inherits from all of them, and does all their things.
    Arguments should be given in the order you want their modules to happen
    within each step (but of course the steps are there to make that unlikely
    to matter...)
    '''
    if not bases:
        return AnalysisFlowBase
    
    # fail with a verbose message if any argument isn't an AnalysisFlow
    # (or a not-so-verbose error message if one isn't even a class)
    try:
        basesOk = all(issubclass(b, AnalysisFlowBase) for b in bases)
    except TypeError:
        print("Error: Non-class object passed as Analysis Flow component")
        raise

    if not basesOk:
        for b in bases:
            if not issubclass(b, AnalysisFlowBase):
                BadClass = b
                break
        raise TypeError("{} does not inherit from AnalysisFlowBase".format(BadClass.__name__))

    if len(bases) == 1:
        return bases[0]

    newClassName = '_'.join(b.__name__ for b in bases)

    # reverse argument list for proper inheritance tree traversal
    C = type(newClassName, tuple(reversed(bases)), {})

    return C
