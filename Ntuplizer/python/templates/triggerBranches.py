import FWCore.ParameterSet.Config as cms

# https://twiki.cern.ch/twiki/bin/viewauth/CMS/HiggsZZ4lRunIII#Trigger_Paths
# Same triggers as 2018 + HLT_Ele30_WPTight_Gsf_v*
triggerBranches_2022 = cms.PSet(
    trigNames = cms.vstring(
        "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL",#2018diEle
        "HLT_DoubleEle25_CaloIdL_MW",#2018diEle
        "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",#2018diMu
        "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL",#2018MuEle
        'HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ',#2018MuEle
        "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ",#2018MuEle
        "HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ",#2018MuEle
        "HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ",#2018MuEle
        "HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ",#2018MuEle
        "HLT_TripleMu_10_5_5_DZ",#2018triMu
        "HLT_TripleMu_12_10_5",#2018triMu
        "HLT_Ele30_WPTight_Gsf",#2022SingleEle
        "HLT_Ele32_WPTight_Gsf",#2018SingleEle
        "HLT_IsoMu24",#2018SingleMu
    ),
    HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVLPaths = cms.vstring('HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),#2018diEle
    HLT_DoubleEle25_CaloIdL_MWPaths = cms.vstring('HLT_DoubleEle25_CaloIdL_MW_v[0-9]+'),#2018diEle
    HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8Paths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v[0-9]+'),#2018diMu
    HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVLPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),#2018MuEle
    HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018MuEle
    HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018MuEle
    HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018MuEle
    HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZPaths = cms.vstring('HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_v[0-9]+'),#2018MuEle
    HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZPaths = cms.vstring('HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_v[0-9]+'),#2018MuEle
    HLT_TripleMu_10_5_5_DZPaths = cms.vstring('HLT_TripleMu_10_5_5_DZ_v[0-9]+'),#2018triMu
    HLT_TripleMu_12_10_5Paths = cms.vstring('HLT_TripleMu_12_10_5_v[0-9]+'),#2018triMu
    HLT_Ele30_WPTight_GsfPaths = cms.vstring('HLT_Ele30_WPTight_Gsf_v[0-9]+'),#2022SingleEle
    HLT_Ele32_WPTight_GsfPaths = cms.vstring('HLT_Ele32_WPTight_Gsf_v[0-9]+'),#2018SingleEle
    HLT_IsoMu24Paths = cms.vstring('HLT_IsoMu24_v[0-9]+'),#2018SingleMu
    
    trigResultsSrc = cms.InputTag("TriggerResults", "", "HLT"),
    trigPrescaleSrc = cms.InputTag("patTrigger"),

    checkPrescale = cms.bool(False),
    ignoreMissing = cms.untracked.bool(True),
)
