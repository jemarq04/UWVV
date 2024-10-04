import FWCore.ParameterSet.Config as cms

from UWVV.Ntuplizer.templates.eventBranches import eventBranches, genNtupleEventBranches
from UWVV.Ntuplizer.templates.zBranches import zBranches
from UWVV.Ntuplizer.templates.objectBranches import objectBranches
from UWVV.Ntuplizer.templates.leptonBranches import leptonBranches
from UWVV.Ntuplizer.templates.electronBranches import electronBranches
from UWVV.Ntuplizer.templates.muonBranches import muonBranches
from UWVV.Ntuplizer.templates.crossDaughterBranches import makeCrossDaughterBranches

from UWVV.Utilities.helpers import pset2Dict, dict2PSet, \
    recursiveMerge, combinePSets, mapObjects




def makeLepBranchSet(lep, extraBranches=[]):
    branches = [objectBranches, leptonBranches]

    if lep[0] == 'e':
        branches.append(electronBranches)
    if lep[0] == 'm':
        branches.append(muonBranches)

    if hasattr(extraBranches, '__iter__'):
        for b in extraBranches:
            branches.append(b)
    elif isinstance(extraBranches, cms.PSet):
        branches.append(extraBranches)

    return combinePSets(*branches)

def makeZBranchSet(lep, n, extraBranches=[], extraLepBranches=[], addName=False):
    '''
    lep: 'e' or 'm'
    n: 1 or 2 (so we know whether to make, e.g., e1 and e2 or e3 and e4)
    addNameToZBranches: workaround for 2l final states: put 'e1_e2'
    (or whatever) at the start of the branch names so these branches can have
    the necessary names while the object stays nameless for event variables.
    extraBranches: see makeBranchSet extraIntermediateBranches
    extraLepBranches: see makeBranchSet extraFinalObjectBranches
    '''
    branches = [objectBranches, zBranches]
    if hasattr(extraBranches, '__iter__'):
        for b in extraBranches:
            branches.append(b)
    elif isinstance(extraBranches, cms.PSet):
        branches.append(extraBranches)

    branchSet = combinePSets(*branches)

    daughterNames = [lep[0]+str(i) for i in [n*2-1,n*2]]

    if addName:
        renamedBranches = {}
        for bName, b in branchSet.parameters_().items():
            renamedBranches['_'.join(daughterNames+[bName])] = b
        branchSet = cms.PSet(**renamedBranches)

    daughterBranchSet = makeLepBranchSet(lep, extraLepBranches)
    branchSet.daughterParams = cms.VPSet(
        daughterBranchSet,
        daughterBranchSet.clone(),
        )
    branchSet.daughterNames = cms.vstring(*daughterNames)

    return branchSet

def makeBranchSet(channel, extraInitialStateBranches=[],
                  extraIntermediateStateBranches=[],
                  **extraFinalObjectBranches):
    '''
    extraInitialStateBranches (PSet or list of PSets): branches for the whole
        event or the whole system
    extraIntermediateStateBranches (PSet or list of PSets): branches for the Zs
        or other intermediate composite objects
    extraFinalObjectBranches (PSet or list of PSets keyed to object type,
        e.g. 'e'): branches for individual leptons, jets, etc.
    '''
    branches = [eventBranches]

    if hasattr(extraInitialStateBranches, '__iter__'):
        for b in extraInitialStateBranches:
            branches.append(b)
    elif isinstance(extraInitialStateBranches, cms.PSet):
        branches.append(extraInitialStateBranches)

    finalObjects = mapObjects(channel)

    if len(channel) == 1: # single object
        branches.append(makeLepBranchSet(channel,
                                         extraInitialStateBranches+extraFinalObjectBranches[channel]))
    elif len(channel) == 2: # single Z
        assert channel[0] == channel[1], '{} does not make a valid Z'.format(channel)
        branches.append(makeZBranchSet(channel[0], 1, extraIntermediateStateBranches,
                                       extraFinalObjectBranches.get(channel[0], [])))
    else:
        branches.append(objectBranches)

    if len(channel) == 3: # Z+l
        if channel[0] != channel[1]: # emm -> mme
            assert channel[1] == channel[2], "Invalid channel {}".format(channel)
            channel = channel[1:] + channel[0]
            finalObjects = finalObjects[1:] + finalObjects[:1]

        assert channel[0] == channel[1], "Invalid channel {}".format(channel)

        branches.append(makeCrossDaughterBranches(channel, True))

        daughterSets = [
            makeZBranchSet(channel[0], 1, extraIntermediateStateBranches,
                           extraFinalObjectBranches.get(channel[0], [])),
            makeLepBranchSet(channel[2],
                             extraFinalObjectBranches.get(channel[2], []))
            ]
        daughterNames = [
            '_'.join(finalObjects[:2]+['']),
            finalObjects[2],
            ]

    elif len(channel) == 4:
        assert channel[0] == channel[1] and channel[2] == channel[3], \
            'Invalid channel {}'.format(channel)

        branches.append(makeCrossDaughterBranches(channel, True))

        daughterSets = [
            makeZBranchSet(channel[0], 1, extraIntermediateStateBranches,
                           extraFinalObjectBranches.get(channel[0], [])),
            makeZBranchSet(channel[2], int(finalObjects[3][1])//2,
                           extraIntermediateStateBranches,
                           extraFinalObjectBranches.get(channel[2], [])),
            ]
        daughterNames = [
            '_'.join(finalObjects[:2]+['']),
            '_'.join(finalObjects[2:]+['']),
            ]

    else:
        daughterSets = []
        daughterNames = []

    branchSet = combinePSets(*branches)

    if daughterSets:
        branchSet.daughterParams = cms.VPSet(*daughterSets)
    if daughterNames:
        branchSet.daughterNames = cms.vstring(*daughterNames)

    return branchSet


def makeGenBranchSet(channel, extraInitialStateBranches=[],
                     extraIntermediateStateBranches=[],
                     **extraFinalObjectBranches):
    if len(channel) != 4:
        raise RuntimeError("makeGenBranchSet is only implemented for 4l final "
                           "states. Please add it for {}".format(channel))

    branches = [genNtupleEventBranches, objectBranches] + extraInitialStateBranches
    branches.append(makeCrossDaughterBranches(channel))
    branchSet = combinePSets(*branches)

    finalObjects = mapObjects(channel)

    daughterNames = [
        '_'.join(finalObjects[:2]+['']),
        '_'.join(finalObjects[2:]+['']),
        ]
    branchSet.daughterNames = cms.vstring(*daughterNames)

    finalObjBranches = {
        'e' : combinePSets(objectBranches.clone(), extraFinalObjectBranches.get('e',cms.PSet())),
        'm' : combinePSets(objectBranches.clone(), extraFinalObjectBranches.get('m',cms.PSet())),
        }

    z1BranchSet = objectBranches.clone(
        daughterNames = cms.vstring(*finalObjects[:2]),
        daughterParams = cms.VPSet(finalObjBranches[channel[0]],
                                   finalObjBranches[channel[0]]),
        )
    z1BranchSet = combinePSets(z1BranchSet, *extraIntermediateStateBranches)

    z2BranchSet = objectBranches.clone(
        daughterNames = cms.vstring(*finalObjects[2:]),
        daughterParams = cms.VPSet(finalObjBranches[channel[2]],
                                   finalObjBranches[channel[2]]),
        )

    z2BranchSet = combinePSets(z2BranchSet, *extraIntermediateStateBranches)

    branchSet.daughterParams = cms.VPSet(z1BranchSet,z2BranchSet)

    return branchSet
