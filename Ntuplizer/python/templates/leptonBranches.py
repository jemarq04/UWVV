import FWCore.ParameterSet.Config as cms


leptonBranches = cms.PSet(
    floats=cms.PSet(
        ZZIso=cms.string('? hasUserFloat("ZZIsoVal") ? userFloat("ZZIsoVal") : 999.'),
        SIP3D=cms.string("SIP3D"),
        IP3D=cms.string("IP3D"),
        IP3DUncertainty=cms.string("IP3DUncertainty"),
        SIP2D=cms.string("SIP2D"),
        IP2D=cms.string("IP2D"),
        IP2DUncertainty=cms.string("IP2DUncertainty"),
        PVDXY=cms.string("PVDXY"),
        PVDZ=cms.string("PVDZ"),
    ),
    ints=cms.PSet(
        PdgId=cms.string("pdgId"),
    ),
    bools=cms.PSet(
        ZZLooseID=cms.string('? hasUserFloat("ZZIDPass") ? userFloat("ZZIDPass") : 0.'),
        ZZTightID=cms.string('? hasUserFloat("ZZIDPassTight") ? userFloat("ZZIDPassTight") : 0.'),
        # ZZTightIDHZZWP = cms.string('? hasUserFloat("ZZIDPassTightHZZWP") ? userFloat("ZZIDPassTightHZZWP") : 0.'),
        ZZLooseIDNoVtx=cms.string('? hasUserFloat("ZZIDPassNoVtx") ? userFloat("ZZIDPassNoVtx") : 0.'),
        ZZTightIDNoVtx=cms.string('? hasUserFloat("ZZIDPassTightNoVtx") ? userFloat("ZZIDPassTightNoVtx") : 0.'),
        # ZZTightIDNoVtxHZZWP = cms.string('? hasUserFloat("ZZIDPassTightNoVtxHZZWP") ? userFloat("ZZIDPassTightNoVtxHZZWP") : 0.'),
        ZZIsoPass=cms.string('? hasUserFloat("ZZIsoPass") ? userFloat("ZZIsoPass") : 0.'),
    ),
)

matchedGenLeptonBranches = cms.PSet(
    floats=cms.PSet(
        GenPt=cms.string("? genParticleRef.isNull ? -999. : genParticleRef.pt"),
        GenEta=cms.string("? genParticleRef.isNull ? -999. : genParticleRef.eta"),
        GenPhi=cms.string("? genParticleRef.isNull ? -999. : genParticleRef.phi"),
    ),
    ints=cms.PSet(
        GenPdgId=cms.string("? genParticleRef.isNull ? -999 : genParticleRef.pdgId"),
        GenCharge=cms.string("? genParticleRef.isNull ? -999 : genParticleRef.charge"),
    ),
)

dressedGenLeptonBranches = cms.PSet(
    floats=cms.PSet(
        UndressedPt=cms.string("undressedPt"),
        UndressedEta=cms.string("undressedEta"),
        UndressedPhi=cms.string("undressedPhi"),
    ),
    ints=cms.PSet(
        nAssociatedPhotons=cms.string("numAssociated"),
    ),
)
