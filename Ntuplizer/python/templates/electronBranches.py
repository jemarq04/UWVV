import FWCore.ParameterSet.Config as cms


electronBranches = cms.PSet(
    floats = cms.PSet(
        Loose = cms.string('? hasUserFloat("Loose")?' 'userFloat("Loose"): 1.'),
        Medium = cms.string('? hasUserFloat("Medium")?' 'userFloat("Medium"): 1.'),
        Tight = cms.string('? hasUserFloat("Tight")?' 'userFloat("Tight"): 1.'),
        Veto = cms.string('? hasUserFloat("Veto")?' 'userFloat("Veto"): 1.'),
        RelPFIsoRho = cms.string('(pfIsolationVariables.sumChargedHadronPt'
                                 '+max(0.0,pfIsolationVariables.sumNeutralHadronEt'
                                 '+pfIsolationVariables.sumPhotonEt'
                                 '-userFloat("rho_fastjet")*userFloat("EffectiveArea")))'
                                 '/pt'),
        Rho = cms.string('userFloat("rho_fastjet")'),
        EffectiveArea = cms.string('userFloat("EffectiveArea")'),
        PFChargedIso = cms.string('pfIsolationVariables.sumChargedHadronPt'),
        PFPhotonIso = cms.string('pfIsolationVariables.sumPhotonEt'),
        PFNeutralIso = cms.string('pfIsolationVariables.sumNeutralHadronEt'),
        PFPUIso = cms.string('pfIsolationVariables.sumPUPt'),
        SCEta = cms.string('superCluster.eta'),
        SCPhi = cms.string('superCluster.phi'),
        SCEnergy = cms.string('superCluster.energy'),
        SCRawEnergy = cms.string('superCluster.rawEnergy'),

        UnCorrPt = cms.string('? hasUserFloat("uncorrected_pt") ? '
                           'userFloat("uncorrected_pt") : '
                           '1.'),
        UnCorrMass = cms.string('? hasUserFloat("uncorrected_mass") ? '
                                'userFloat("uncorrected_mass") : 1.'),

        ScaleValue = cms.string('? hasUserFloat("energyScaleValue") ? '
                            'userFloat("energyScaleValue") : '
                            '1.'),
        ScaleUp = cms.string('? hasUserFloat("energyScaleUp") ? '
                            'userFloat("energyScaleUp") : '
                            '1.'),
        ScaleDn = cms.string('? hasUserFloat("energyScaleDn") ? '
                            'userFloat("energyScaleDn") : '
                            '1.'),
        SmearValue = cms.string('? hasUserFloat("energySmearValue") ? '
                            'userFloat("energySmearValue") : '
                            '0.'),
        SmearUp = cms.string('? hasUserFloat("energySmearUp") ? '
                            'userFloat("energySmearUp") : '
                            '0.'),
        SmearDn = cms.string('? hasUserFloat("energySmearDn") ? '
                            'userFloat("energySmearDn") : '
                            '0.'),

        ScaleTotUp = cms.string('? hasUserFloat("scale_total_up") ? '
                                    'userFloat("scale_total_up") : 0.'),
        ScaleSystUp = cms.string('? hasUserFloat("scale_syst_up") ? '
                                    'userFloat("scale_syst_up") : 0.'),
        ScaleStatUp = cms.string('? hasUserFloat("scale_stat_up") ? '
                                    'userFloat("scale_stat_up") : 0.'),
        ScaleGainUp = cms.string('? hasUserFloat("scale_gain_up") ? '
                                    'userFloat("scale_gain_up") : 0.'),
        ScaleTotDn = cms.string('? hasUserFloat("scale_total_dn") ? '
                                    'userFloat("scale_total_dn") : 0.'),
        ScaleSystDn = cms.string('? hasUserFloat("scale_syst_dn") ? '
                                    'userFloat("scale_syst_dn") : 0.'),
        ScaleStatDn = cms.string('? hasUserFloat("scale_stat_dn") ? '
                                    'userFloat("scale_stat_dn") : 0.'),
        ScaleGainDn = cms.string('? hasUserFloat("scale_gain_dn") ? '
                                    'userFloat("scale_gain_dn") : 0.'),

        SigmaTotUp = cms.string('? hasUserFloat("sigma_total_up") ? '
                                    'userFloat("sigma_total_up") : 0.'),
        SigmaRhoUp = cms.string('? hasUserFloat("sigma_rho_up") ? '
                                    'userFloat("sigma_rho_up") : 0.'),
        SigmaPhiUp = cms.string('? hasUserFloat("sigma_phi_up") ? '
                                    'userFloat("sigma_phi_up") : 0.'),
        SigmaTotDn = cms.string('? hasUserFloat("sigma_total_dn") ? '
                                    'userFloat("sigma_total_dn") : 0.'),
        SigmaRhoDn = cms.string('? hasUserFloat("sigma_rho_dn") ? '
                                    'userFloat("sigma_rho_dn") : 0.'),
        SigmaPhiDn = cms.string('? hasUserFloat("sigma_phi_dn") ? '
                                    'userFloat("sigma_phi_dn") : 0.'),
        ),

    uints = cms.PSet(
        MissingHits = cms.string('MissingHits'),
        ),
    bools = cms.PSet(
        IsGap = cms.string('isGap'),
        IsEB = cms.string('isEB'),
        )
    )
