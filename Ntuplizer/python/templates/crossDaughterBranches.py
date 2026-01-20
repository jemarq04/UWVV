import FWCore.ParameterSet.Config as cms

from itertools import combinations

from UWVV.Utilities.helpers import dict2PSet, mapObjects


def makeCrossDaughterBranches(channel, includeFSR=False):
    """
    Make a PSet of branches for di-object variables outside of Zs,
    e.g. e1_m2_Mass.
    """
    objects = mapObjects(channel)

    params = {"floats": {}, "bools": {}}
    for pair in combinations(objects, 2):
        # don't include regular Zs
        if pair[0][0] == pair[1][0] and int(pair[0][1]) % 2 == 1 and int(pair[0][1]) + 1 == int(pair[1][1]):
            continue

        name = "_".join([pair[0], pair[1], ""])

        params["floats"][name + "Mass"] = cms.string(
            '? hasUserFloat("{0}") ? userFloat("{0}") : -999.'.format(name + "Mass")
        )
        if includeFSR:
            params["floats"][name + "MassNoFSR"] = cms.string(
                '? hasUserFloat("{0}") ? userFloat("{0}") : -999.'.format(name + "MassNoFSR")
            )
        params["bools"][name + "SS"] = cms.string('? hasUserFloat("{0}") ? userFloat("{0}") : 0'.format(name + "SS"))
        params["floats"][name + "DR"] = cms.string(
            '? hasUserFloat("{0}") ? userFloat("{0}") : -999.'.format(name + "DR")
        )

    return dict2PSet(params)
