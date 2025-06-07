import FWCore.ParameterSet.Config as cms

altZZBranches = cms.PSet(
    floats = cms.PSet(
        ZaMass = cms.string('? hasUserFloat("ZaMass") ? userFloat("ZaMass") : -1.'),
        ZbMass = cms.string('? hasUserFloat("ZbMass") ? userFloat("ZbMass") : -1.'),
        ),
    )
