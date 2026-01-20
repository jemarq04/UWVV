import FWCore.ParameterSet.Config as cms


objectBranches = cms.PSet(
    floats=cms.PSet(
        P=cms.string("p"),
        Pt=cms.string("pt"),
        Eta=cms.string("eta"),
        Phi=cms.string("phi"),
        Mass=cms.string("mass"),
        Energy=cms.string("energy"),
        MtToMET=cms.string("mtToMET"),
    ),
    ints=cms.PSet(
        Charge=cms.string("charge"),
        PdgId=cms.string("pdgId"),
    ),
)
