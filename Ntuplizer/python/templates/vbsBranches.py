import FWCore.ParameterSet.Config as cms


vbsPrimitiveBranches = cms.PSet(
    floats=cms.PSet(
        rapidity=cms.string("rapidity"),
        mjj=cms.string("mjj"),
        phijj=cms.string("phijj"),
        etajj=cms.string("etajj"),
        ptjj=cms.string("ptjj"),
    ),
    vFloats=cms.PSet(
        jetRapidity=cms.vstring("jetRapidity"),
    ),
)

vbsDerivedBranches = cms.PSet(
    floats=cms.PSet(
        deltaPhiTojj=cms.string("deltaPhiTojj"),
        zeppenfeld=cms.string("zeppenfeld"),
        zeppenfeldj3=cms.string("zeppenfeldj3"),
        deltaEtajj=cms.string("deltaEtajj"),
    ),
)


vbsGenBranches = cms.PSet(
    floats=cms.PSet(
        rapidity=cms.string("rapidity"),
        mjj=cms.string("mjjGen"),
        deltaPhiTojj=cms.string("deltaPhiTojjGen"),
        zeppenfeld=cms.string("zeppenfeldGen"),
        zeppenfeldj3=cms.string("zeppenfeldj3Gen"),
        deltaEtajj=cms.string("deltaEtajjGen"),
    ),
    vFloats=cms.PSet(
        jetRapidity=cms.vstring("genJetRapidity"),
    ),
)

vbsPrimitiveSystematicBranches = cms.PSet(
    floats=cms.PSet(
        mjj_jesUp=cms.string("mjj::jesUp"),
        mjj_jesDown=cms.string("mjj::jesDown"),
        mjj_jerUp=cms.string("mjj::jerUp"),
        mjj_jerDown=cms.string("mjj::jerDown"),
        phijj_jesUp=cms.string("phijj::jesUp"),
        phijj_jesDown=cms.string("phijj::jesDown"),
        phijj_jerUp=cms.string("phijj::jerUp"),
        phijj_jerDown=cms.string("phijj::jerDown"),
        etajj_jesUp=cms.string("etajj::jesUp"),
        etajj_jesDown=cms.string("etajj::jesDown"),
        etajj_jerUp=cms.string("etajj::jerUp"),
        etajj_jerDown=cms.string("etajj::jerDown"),
        ptjj_jesUp=cms.string("ptjj::jesUp"),
        ptjj_jesDown=cms.string("ptjj::jesDown"),
        ptjj_jerUp=cms.string("ptjj::jerUp"),
        ptjj_jerDown=cms.string("ptjj::jerDown"),
    ),
    vFloats=cms.PSet(
        jetRapidity_jesUp=cms.vstring("jetRapidity::jesUp"),
        jetRapidity_jesDown=cms.vstring("jetRapidity::jesDown"),
        jetRapidity_jerUp=cms.vstring("jetRapidity::jerUp"),
        jetRapidity_jerDown=cms.vstring("jetRapidity::jerDown"),
    ),
)

vbsDerivedSystematicBranches = cms.PSet(
    floats=cms.PSet(
        deltaPhiTojj_jesUp=cms.string("deltaPhiTojj::jesUp"),
        deltaPhiTojj_jesDown=cms.string("deltaPhiTojj::jesDown"),
        deltaPhiTojj_jerUp=cms.string("deltaPhiTojj::jerUp"),
        deltaPhiTojj_jerDown=cms.string("deltaPhiTojj::jerDown"),
        zeppenfeld_jesUp=cms.string("zeppenfeld::jesUp"),
        zeppenfeld_jesDown=cms.string("zeppenfeld::jesDown"),
        zeppenfeld_jerUp=cms.string("zeppenfeld::jerUp"),
        zeppenfeld_jerDown=cms.string("zeppenfeld::jerDown"),
        zeppenfeldj3_jesUp=cms.string("zeppenfeldj3::jesUp"),
        zeppenfeldj3_jesDown=cms.string("zeppenfeldj3::jesDown"),
        zeppenfeldj3_jerUp=cms.string("zeppenfeldj3::jerUp"),
        zeppenfeldj3_jerDown=cms.string("zeppenfeldj3::jerDown"),
        deltaEtajj_jesUp=cms.string("deltaEtajj::jesUp"),
        deltaEtajj_jesDown=cms.string("deltaEtajj::jesDown"),
        deltaEtajj_jerUp=cms.string("deltaEtajj::jerUp"),
        deltaEtajj_jerDown=cms.string("deltaEtajj::jerDown"),
    ),
)
