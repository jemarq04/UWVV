import FWCore.ParameterSet.Config as cms

# https://cms-jme-jmar.docs.cern.ch/recommendations/met/noise_filters/#run-3
metFilters = cms.PSet(
    trigNames=cms.vstring(
        "Flag_goodVertices",
        "Flag_globalSuperTightHalo2016Filter",
        "Flag_EcalDeadCellTriggerPrimitiveFilter",
        "Flag_BadPFMuonFilter",
        "Flag_BadPFMuonDzFilter",
        "Flag_hfNoisyHitsFilter",
        "Flag_eeBadScFilter",
        "Flag_ecalBadCalibFilter",
    ),
    Flag_goodVerticesPaths=cms.vstring("Flag_goodVertices"),
    Flag_globalSuperTightHalo2016FilterPaths=cms.vstring("Flag_globalSuperTightHalo2016Filter"),
    Flag_EcalDeadCellTriggerPrimitiveFilterPaths=cms.vstring("Flag_EcalDeadCellTriggerPrimitiveFilter"),
    Flag_BadPFMuonFilterPaths=cms.vstring("Flag_BadPFMuonFilter"),
    Flag_BadPFMuonDzFilterPaths=cms.vstring("Flag_BadPFMuonDzFilter"),
    Flag_hfNoisyHitsFilterPaths=cms.vstring("Flag_hfNoisyHitsFilter"),
    Flag_eeBadScFilterPaths=cms.vstring("Flag_eeBadScFilter"),
    Flag_ecalBadCalibFilterPaths=cms.vstring("Flag_ecalBadCalibFilter"),
    trigResultsSrc=cms.InputTag("TriggerResults", "", "RECO"),
    trigPrescaleSrc=cms.InputTag("patTrigger"),
    checkPrescale=cms.bool(False),
)

metFiltersSIM = cms.PSet(
    trigNames=cms.vstring(
        "Flag_goodVertices",
        "Flag_globalSuperTightHalo2016Filter",
        "Flag_EcalDeadCellTriggerPrimitiveFilter",
        "Flag_BadPFMuonFilter",
        "Flag_BadPFMuonDzFilter",
        "Flag_hfNoisyHitsFilter",
        "Flag_eeBadScFilter",
        "Flag_ecalBadCalibFilter",
    ),
    Flag_goodVerticesPaths=cms.vstring("Flag_goodVertices"),
    Flag_globalSuperTightHalo2016FilterPaths=cms.vstring("Flag_globalSuperTightHalo2016Filter"),
    Flag_EcalDeadCellTriggerPrimitiveFilterPaths=cms.vstring("Flag_EcalDeadCellTriggerPrimitiveFilter"),
    Flag_BadPFMuonFilterPaths=cms.vstring("Flag_BadPFMuonFilter"),
    Flag_BadPFMuonDzFilterPaths=cms.vstring("Flag_BadPFMuonDzFilter"),
    Flag_hfNoisyHitsFilterPaths=cms.vstring("Flag_hfNoisyHitsFilter"),
    Flag_eeBadScFilterPaths=cms.vstring("Flag_eeBadScFilter"),
    Flag_ecalBadCalibFilterPaths=cms.vstring("Flag_ecalBadCalibFilter"),
    trigResultsSrc=cms.InputTag("TriggerResults", "", "PAT"),
    trigPrescaleSrc=cms.InputTag("patTrigger"),
    checkPrescale=cms.bool(False),
)
