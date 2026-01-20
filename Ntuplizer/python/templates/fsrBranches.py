import FWCore.ParameterSet.Config as _cms

compositeObjectFSRBranches = _cms.PSet(
    floats=_cms.PSet(
        MassNoFSR=_cms.string("massNoFSR"),
        PtNoFSR=_cms.string("ptNoFSR"),
        EtaNoFSR=_cms.string("etaNoFSR"),
        PhiNoFSR=_cms.string("phiNoFSR"),
        EnergyNoFSR=_cms.string("energyNoFSR"),
    ),
)

leptonFSRBranches = _cms.PSet(
    floats=_cms.PSet(
        FSREt=_cms.string('? hasUserCand("fsr") ? userCand("fsr").et() : -999.'),
        FSREta=_cms.string('? hasUserCand("fsr") ? userCand("fsr").eta() : -999.'),
        FSRPhi=_cms.string('? hasUserCand("fsr") ? userCand("fsr").phi() : -999.'),
        FSRDREt2=_cms.string('? hasUserFloat("fsrDREt2") ? userFloat("fsrDREt2") : 999.'),
    ),
)
