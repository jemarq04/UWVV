import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.Mixins import _Parameterizable
from functools import reduce

from os import path
from math import sqrt, pi


def pset2Dict(ps):
    '''
    Turns a PSet into a dict of (copies of) its elements.
    Acts recursively, such that a PSet in a PSet becomes a dict in a dict.
    '''
    d = ps.parameters_()
    for p in d:
        if isinstance(d[p], _Parameterizable):
            d[p] = pset2Dict(d[p])

    return d

def dict2PSet(d):
    '''
    Opposite of above.
    '''
    for p in d:
        if isinstance(d[p], dict):
            d[p] = dict2PSet(d[p])

    return cms.PSet(**d)

def recursiveMerge(a, b=None, path=None):
    '''
    Merge dict b into dict a, recursively in the case of nested dicts.
    '''
    if b is None:
        b = {}
    if path is None:
        path = []

    for k in b:
        if k in a:
            if isinstance(a[k], dict) and isinstance(b[k], dict):
                recursiveMerge(a[k], b[k], path+[str(k)])
            elif a[k] == b[k]:
                pass
            else:
                raise Exception("Can't merge parameters: {}".format('.'.join(path+[str(k)])))
        else:
            a[k] = b[k]

    return a

def combinePSets(ps, *more):
    '''
    Get a PSet that is a deep copy of one or more PSets. If multiple PSets
    have an item with the same name, the last argument to this function wins.
    '''
    if not more:
        return ps.copy()

    allParams = [pset2Dict(p) for p in [ps]+list(more)]
    params = reduce(recursiveMerge, allParams)

    return dict2PSet(params)

def parseChannels(channels):
    '''
    Take a string or list of strings and return a list of channels.
    '''
    if type(channels) is str:
        channels = channels.lower()
        if channels in ['4l', 'zz']:
            return ['eeee', 'eemm', 'mmmm']
        elif channels in ['3l', 'zl', 'z+l', 'wz']:
            return ['eee', 'eem', 'emm', 'mmm']
        elif channels in ['z', '2l', 'll']:
            return ['ee', 'mm']
        elif channels in ['l', '1l']:
            return ['e', 'm']
        else:
            chanList = channels.split(',')
            assert all(all(letter in ['e','m','t','g','j'] for letter in ch) and len(ch) <= 4 for ch in chanList),\
                'Invalid channel ' + channels
            return chanList
    else:
        assert hasattr(channels, '__iter__'), 'Channels must be a list or a string'
        out = []
        for ch in channels:
            out += parseChannels(ch)
        return out

_expandedObjNames = {
    'e' : 'E',
    'm' : 'Mu',
    't' : 'Tau',
    'g' : 'Pho',
    'j' : 'Jet',
    }
def expandChannelName(channel):
    '''
    Turn 'eemt' into 'EEMuTau' or something like that.
    '''
    return ''.join([_expandedObjNames[ob] for ob in channel.lower()])


# Location of base directory regardless of anything else
_pathToHere = path.split(path.realpath(__file__))[0]
UWVV_BASE_PATH = path.normpath(path.join(_pathToHere, '..', '..'))

_objectList = {
    'e' : 'electron',
    'm' : 'muon',
    't' : 'tau',
    'g' : 'photon',
    'j' : 'jet',
}

def getObjTypes():
    return list(_objectList.keys())

def getObjName(obj, capitalize=False):
    '''
    Get the spelled out name of the object type ('electron' for 'e' etc.).
    '''
    name = _objectList[obj[0]]
    if capitalize:
        name = name[0].upper() + name[1:]
    return name

_zzhelpers_object_maps_ = {}
def mapObjects(channel):
    '''
    Return a list of objects of the form ['e1','e2','m1','m2'] or ['e1','e2','m']
    Objects are in alphabetical/numerical order order
    '''
    global _zzhelpers_object_maps_ #noqa: PLW0602

    try:
        return _zzhelpers_object_maps_[channel]
    except KeyError:
        nObjects = {}
        objects = []

        for obj in channel:
            if obj not in nObjects:
                nObjects[obj] = 1
            else:
                nObjects[obj] += 1

        for obj, num in nObjects.items():
            if num == 1:
                objects.append(obj)
            else:
                for i in range(num):
                    objects.append(obj+str(i+1))

        objects.sort()

        _zzhelpers_object_maps_[channel] = objects
        return objects

def deltaR(eta1, phi1, eta2, phi2):
    dPhi = abs(phi2 - phi1)
    while dPhi > pi:
        dPhi -= 2*pi
    return sqrt(dPhi**2 + (eta2 - eta1)**2)

def getCorrectionFile(pog, era, correction):
    era_map = {
        "2022": "Run3-22CDSep23-Summer22-NanoAODv12",
        "2022EE": "Run3-22EFGSep23-Summer22EE-NanoAODv12",
        "2023": "Run3-23CSep23-Summer23-NanoAODv12",
        "2023BPix": "Run3-23DSep23-Summer23BPix-NanoAODv12",
        "2024": "Run3-24CDEReprocessingFGHIPrompt-Summer24-NanoAODv15",
        "2025": "Run3-25Prompt-Winter25-NanoAODv15",
    }

    corr_path = path.join(UWVV_BASE_PATH, "data", "XPOG")
    if not path.isdir(corr_path):
        corr_path = "/cvmfs/cms-griddata.cern.ch/cat/metadata"
        print("WARNING: central correction files can change unexpectedly, use check_corrections.sh to create local copies")
    return path.join(corr_path, pog, era_map[era], "latest", correction)
