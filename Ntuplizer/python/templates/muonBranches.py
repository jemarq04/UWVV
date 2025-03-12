import FWCore.ParameterSet.Config as cms

muonBranches = cms.PSet(
    floats = cms.PSet(
        RelPFIsoDBR03 = cms.string('(pfIsolationR03.sumChargedParticlePt'
                                '+max(0.0,pfIsolationR03.sumPhotonEt'
                                '+pfIsolationR03.sumNeutralHadronEt'
                                '-0.5*pfIsolationR03.sumPUPt))'
                                '/pt()'),
        RelPFIsoDBR04  = cms.string(
                                '(pfIsolationR04().sumChargedHadronPt'
                                '+ max(0., pfIsolationR04().sumNeutralHadronEt'
                                '+ pfIsolationR04().sumPhotonEt'
                                '- 0.5*pfIsolationR04().sumPUPt))'
                                '/pt()'
        ),  
        PFChargedIso = cms.string('pfIsolationR03.sumChargedParticlePt'),
        PFPhotonIso = cms.string('pfIsolationR03.sumPhotonEt'),
        PFNeutralIso = cms.string('pfIsolationR03.sumNeutralHadronEt'),
        PFPUIso = cms.string('pfIsolationR03.sumPUPt'),
        TrackIso = cms.string('trackIso()'), 
        
        PtErr = cms.string('? hasUserFloat("correctedPtError") ? '
                           'userFloat("correctedPtError") : '
                           'bestTrack.ptError'),
        #RochesterPATMuonCorrector.cc
        UnCorrPt = cms.string('? hasUserFloat("uncorrected_pt") ? '
                           'userFloat("uncorrected_pt") : '
                           '1.'),
        EffScaleFactor = cms.string('? hasUserFloat("ptScaleFactor") ? '
                                    'userFloat("ptScaleFactor") : 1.'),
        PtSyst = cms.string('? hasUserFloat("syst_pt") ? '
                            'userFloat("syst_pt") : 0.'),
        PtStat = cms.string('? hasUserFloat("stat_pt") ? '
                            'userFloat("stat_pt") : 0.'),
        # TrkRecoEffScaleFactor = cms.string('? hasUserFloat("trkRecoEffScaleFactor") ? '
        #                                    'userFloat("trkRecoEffScaleFactor") : 1.'),
        MtToMET = cms.string('mtToMET'),
        ),
    uints = cms.PSet(
        MatchedStations = cms.string('MatchedStations'),
        NoOfMatches = cms.string('NoOfMatches'),
        BestTrackType = cms.string('BestTrackType'),
        IsTight = cms.string('? hasUserInt("isTightMuon") ? '
                                'userInt("isTightMuon") : 0'),
        CutBasedLoose = cms.string('? hasUserInt("CutBasedIdLoose") ? '
                                'userInt("CutBasedIdLoose") : 0'),
        CutBasedMedium = cms.string('? hasUserInt("CutBasedIdMedium") ? '
                                'userInt("CutBasedIdMedium") : 0'),
        CutBasedTight = cms.string('? hasUserInt("CutBasedIdTight") ? '
                                'userInt("CutBasedIdTight") : 0'),
        PFIsoLoose = cms.string('? hasUserInt("PFIsoLoose") ? '
                                'userInt("PFIsoLoose") : 0'),
        PFIsoMedium = cms.string('? hasUserInt("PFIsoMedium") ? '
                                'userInt("PFIsoMedium") : 0'),
        PFIsoTight = cms.string('? hasUserInt("PFIsoTight") ? '
                                'userInt("PFIsoTight") : 0'),
        PFIsoVTight = cms.string('? hasUserInt("PFIsoVeryTight") ? '
                                'userInt("PFIsoVeryTight") : 0'),
        ),
    bools = cms.PSet(
        IsPFMuon = cms.string('isPFMuon'),
        IsGlobal = cms.string('isGlobalMuon'),
        IsTracker = cms.string('isTrackerMuon'),
        IsLoose = cms.string('isLooseMuon'),
        IsMedium = cms.string('isMediumMuon'),
        HighPtID = cms.string('? hasUserFloat("ZZIDPassHighPt") ? '
                              'userFloat("ZZIDPassHighPt") : 0.'),
        HighPtIDNoVtx = cms.string('? hasUserFloat("ZZIDPassHighPtNoVtx") ? '
                                   'userFloat("ZZIDPassHighPtNoVtx") : 0.'),
        PFID = cms.string('? hasUserFloat("ZZIDPassPF") ? '
                          'userFloat("ZZIDPassPF") : 0.'),
        PFIDNoVtx = cms.string('? hasUserFloat("ZZIDPassPFNoVtx") ? '
                               'userFloat("ZZIDPassPFNoVtx") : 0.'),
        ),
    )

muonCalibrationBranches = cms.PSet(
    floats = cms.PSet(
        PtUncorrected = cms.string('? hasUserCand("uncorrected") ? '
                                   'userCand("uncorrected").pt : '
                                   'pt'),
        PtErrUncorrected = cms.string('? hasUserCand("uncorrected") ? '
                                      'userCand("uncorrected").bestTrack.ptError : '
                                      'bestTrack.ptError'),
        ),
    )
