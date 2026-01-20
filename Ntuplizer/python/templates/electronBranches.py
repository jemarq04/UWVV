import FWCore.ParameterSet.Config as cms


electronBranches = cms.PSet(
    floats=cms.PSet(
        Loose=cms.string('? hasUserFloat("Loose") ? userFloat("Loose") : 1.'),
        Medium=cms.string('? hasUserFloat("Medium") ? userFloat("Medium") : 1.'),
        Tight=cms.string('? hasUserFloat("Tight") ? userFloat("Tight") : 1.'),
        Veto=cms.string('? hasUserFloat("Veto") ? userFloat("Veto") : 1.'),
        RelPFIsoRho=cms.string(
            "(pfIsolationVariables.sumChargedHadronPt"
            "+max(0.0,pfIsolationVariables.sumNeutralHadronEt"
            "+pfIsolationVariables.sumPhotonEt"
            '-userFloat("rho_fastjet")*userFloat("EffectiveArea")))'
            "/pt"
        ),
        Rho=cms.string('userFloat("rho_fastjet")'),
        EffectiveArea=cms.string('userFloat("EffectiveArea")'),
        PFChargedIso=cms.string("pfIsolationVariables.sumChargedHadronPt"),
        PFPhotonIso=cms.string("pfIsolationVariables.sumPhotonEt"),
        PFNeutralIso=cms.string("pfIsolationVariables.sumNeutralHadronEt"),
        PFPUIso=cms.string("pfIsolationVariables.sumPUPt"),
        SCEta=cms.string("superCluster.eta"),
        SCPhi=cms.string("superCluster.phi"),
        SCEnergy=cms.string("superCluster.energy"),
        SCRawEnergy=cms.string("superCluster.rawEnergy"),
        UnCorrPt=cms.string('? hasUserFloat("uncorrected_pt") ? userFloat("uncorrected_pt") : 1.'),
        EffScaleFactor=cms.string('? hasUserFloat("ptScaleFactor") ? userFloat("ptScaleFactor") : 1.'),
        ScaleUpPt=cms.string('? hasUserFloat("scaleUp_pt") ? userFloat("scaleUp_pt") : pt'),
        ScaleDnPt=cms.string('? hasUserFloat("scaleDn_pt") ? userFloat("scaleDn_pt") : pt'),
        SmearUpPt=cms.string('? hasUserFloat("smearUp_pt") ? userFloat("smearUp_pt") : pt'),
        SmearDnPt=cms.string('? hasUserFloat("smearDn_pt") ? userFloat("smearDn_pt") : pt'),
    ),
    uints=cms.PSet(
        MissingHits=cms.string("MissingHits"),
    ),
    bools=cms.PSet(
        IsGap=cms.string("isGap"),
        IsEB=cms.string("isEB"),
    ),
)
