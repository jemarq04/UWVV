import FWCore.ParameterSet.Config as cms



zzCompositeTriggerBranches = cms.PSet(
    #trigNames = cms.vstring(), 'doubleMu', 'doubleMuDZ', 'doubleE',
    #                           'singleESingleMu', 'singleMuSingleE',
    #                           'tripleE', 'doubleESingleMu', 'doubleMuSingleE',
    #                           'tripleMu', 'singleE', 'singleIsoMu',
    #                           'singleIsoMu20', 'singleMu',),
    trigNames = cms.vstring('doubleMuDZ', 'doubleE','singleESingleMu', 
                            'tripleE', 'doubleESingleMu', 'doubleMuSingleE',
                            'tripleMu', 'singleE', 'singleIsoMu',),

    #doubleMuPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v[0-9]+',
    #                            'HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v[0-9]+'),
    doubleMuDZPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v[0-9]+',
                                  'HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8_v[0-9]+'),
    doubleEPaths = cms.vstring('HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
                               'HLT_DoubleEle33_CaloIdL_MW_v[0-9]+'),
    singleESingleMuPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
                                       'HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+',
                                       'HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+',
                                       'HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),
    #singleMuSingleEPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
    #                                   'HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),
    tripleEPaths = cms.vstring('HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v[0-9]+'),
    doubleESingleMuPaths = cms.vstring('HLT_Mu8_DiEle12_CaloIdL_TrackIdL_v[0-9]+',
                                        'HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_v[0-9]+'),
    doubleMuSingleEPaths = cms.vstring('HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_v[0-9]+'),
    tripleMuPaths = cms.vstring('HLT_TripleMu_12_10_5_v[0-9]+',
                                'HLT_TripleMu_10_5_5_DZ_v[0-9]+'),
    singleEPaths = cms.vstring('HLT_Ele35_WPTight_Gsf_v[0-9]+', 
                               'HLT_Ele38_WPTight_Gsf_v[0-9]+',
                               'HLT_Ele40_WPTight_Gsf_v[0-9]+',),
    #singleIsoMu20Paths = cms.vstring('HLT_IsoMu20_v[0-9]+',
    #                                 'HLT_IsoTkMu20_v[0-9]+',),
    singleIsoMuPaths = cms.vstring('HLT_IsoMu27_v[0-9]+'),
    #singleMuPaths = cms.vstring('HLT_Mu50_v[0-9]+',
    #                            'HLT_Mu45_eta2p1_v[0-9]+',),

    trigResultsSrc = cms.InputTag("TriggerResults", "", "HLT"),
    trigPrescaleSrc = cms.InputTag("patTrigger"),

    checkPrescale = cms.bool(False),
    )
verboseTriggerBranches = cms.PSet(
    trigNames = cms.vstring(
         "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8",#2018/2017
         "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8",
         "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL",#2018/2017
         "HLT_DoubleEle25_CaloIdL_MW",#2018
         "HLT_DoubleEle33_CaloIdL_MW",#2017
         "HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL",#2018/2017
         "HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ",#2018/2017
         "HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ",#2018/2017
         'HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ',#2018/2017
         "HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL",
         "HLT_Mu8_DiEle12_CaloIdL_TrackIdL",
         "HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ",
         "HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ",#2018/2017
         "HLT_TripleMu_12_10_5",#2018/2017
         "HLT_TripleMu_10_5_5_DZ",#2018/2017
         "HLT_Ele32_WPTight_Gsf",#2018
         "HLT_Ele35_WPTight_Gsf",#2017
         "HLT_Ele38_WPTight_Gsf",#2017
         "HLT_Ele40_WPTight_Gsf",#2017
         "HLT_IsoMu24",#2018
         "HLT_IsoMu27",#2017
    ),
    #HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVLPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v[0-9]+'),
    #HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVLPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v[0-9]+'),
    HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8Paths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8_v[0-9]+'),#2018/2017
    HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8Paths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8_v[0-9]+'),
    HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVLPaths = cms.vstring('HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),#2018/2017
    #HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),
    HLT_DoubleEle25_CaloIdL_MWPaths = cms.vstring('HLT_DoubleEle25_CaloIdL_MW_v[0-9]+'),#2018
    HLT_DoubleEle33_CaloIdL_MWPaths = cms.vstring('HLT_DoubleEle33_CaloIdL_MW_v[0-9]+'),#2017
    HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVLPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),#2018/2017
    HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018/2017
    HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu12_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018/2017
    HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),#2018/2017
    #HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZPaths =cms.vstring('HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),
    #HLT_Mu17_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVLPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),
    #HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVLPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+'),
    #HLT_Mu23_TrkIsoVVL_Ele8_CaloIdL_TrackIdL_IsoVL_DZPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele8_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),
    HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdLPaths = cms.vstring('HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v[0-9]+'),
    HLT_Mu8_DiEle12_CaloIdL_TrackIdLPaths = cms.vstring('HLT_Mu8_DiEle12_CaloIdL_TrackIdL_v[0-9]+'),
    HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZPaths = cms.vstring('HLT_Mu8_DiEle12_CaloIdL_TrackIdL_DZ_v[0-9]+'),
    HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZPaths = cms.vstring('HLT_DiMu9_Ele9_CaloIdL_TrackIdL_DZ_v[0-9]+'),#2018/2017
    HLT_TripleMu_12_10_5Paths = cms.vstring('HLT_TripleMu_12_10_5_v[0-9]+'),#2018/2017
    HLT_TripleMu_10_5_5_DZPaths = cms.vstring('HLT_TripleMu_10_5_5_DZ_v[0-9]+'),#2018/2017
    HLT_Ele32_WPTight_GsfPaths = cms.vstring('HLT_Ele32_WPTight_Gsf_v[0-9]+'),#2018
    HLT_Ele35_WPTight_GsfPaths = cms.vstring('HLT_Ele35_WPTight_Gsf_v[0-9]+'),#2017
    HLT_Ele38_WPTight_GsfPaths = cms.vstring('HLT_Ele38_WPTight_Gsf_v[0-9]+'),#2017
    HLT_Ele40_WPTight_GsfPaths = cms.vstring('HLT_Ele40_WPTight_Gsf_v[0-9]+'),#2017
    #HLT_IsoMu20Paths = cms.vstring('HLT_IsoMu20_v[0-9]+'),
    #HLT_IsoTkMu20Paths = cms.vstring('HLT_IsoTkMu20_v[0-9]+'),
    HLT_IsoMu24Paths = cms.vstring('HLT_IsoMu24_v[0-9]+'),#2018
    HLT_IsoMu27Paths = cms.vstring('HLT_IsoMu27_v[0-9]+'),#2017
    #HLT_IsoMu24444Paths = cms.vstring('HLT_IsoMu24_v[0-9]+'),
    #HLT_IsoMuTkMu24Paths = cms.vstring('HLT_IsoTkMu24_v[0-9]+'),
    #HLT_IsoTkMu22Paths = cms.vstring('HLT_IsoTkMu22_v[0-9]+'),
    #HLT_Mu50Paths = cms.vstring('HLT_Mu50_v[0-9]+'),
    #HLT_Mu45_eta2p1Paths = cms.vstring('HLT_Mu45_eta2p1_v[0-9]+'),
    trigResultsSrc = cms.InputTag("TriggerResults", "", "HLT"),
    trigPrescaleSrc = cms.InputTag("patTrigger"),

    checkPrescale = cms.bool(False),
    ignoreMissing = cms.untracked.bool(True),
    )

#triggerBranches_2016G = cms.PSet(
#    trigNames = cms.vstring(),# 'doubleMu', 'doubleMuDZ', 'doubleE',
#                              # 'singleESingleMu', 'singleMuSingleE',
#                              # 'tripleE', 'doubleESingleMu', 'doubleMuSingleE',
#                              # 'tripleMu', 'singleE', 'singleIsoMu',
#                              # 'singleIsoMu20', 'singleMu',),
#    doubleMuPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v[0-9]+',
#                                'HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v[0-9]+'),
#    doubleMuDZPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v[0-9]+',
#                                  'HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v[0-9]+'),
#    doubleEPaths = cms.vstring('HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
#                               'HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+',
#                               'HLT_DoubleEle33_CaloIdL_MW_v[0-9]+'),
#    singleESingleMuPaths = cms.vstring('HLT_Mu8_TrkIsoVVL_Ele17_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
#                                       'HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
#                                       'HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),
#    singleMuSingleEPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
#                                       'HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
#                                       'HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),
#    tripleEPaths = cms.vstring('HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v[0-9]+'),
#    doubleESingleMuPaths = cms.vstring('HLT_Mu8_DiEle12_CaloIdL_TrackIdL_v[0-9]+'),
#    doubleMuSingleEPaths = cms.vstring('HLT_DiMu9_Ele9_CaloIdL_TrackIdL_v[0-9]+'),
#    tripleMuPaths = cms.vstring('HLT_TripleMu_12_10_5_v[0-9]+'),
#    singleEPaths = cms.vstring('HLT_Ele35_WPTight_Gsf_v[0-9]+',
#                               'HLT_Ele32_WPTight_v[0-9]+',
#                               'HLT_Ele38_WPTight_Gsf_v[0-9]+',),
#    singleIsoMu20Paths = cms.vstring('HLT_IsoMu20_v[0-9]+',
#                                     'HLT_IsoTkMu20_v[0-9]+',),
#    singleIsoMuPaths = cms.vstring('HLT_IsoMu22_v[0-9]+',
#                                   'HLT_IsoTkMu22_v[0-9]+',
#                                   'HLT_IsoMu24_v[0-9]+',
#                                   'HLT_IsoTkMu24_v[0-9]+',),
#    singleMuPaths = cms.vstring('HLT_Mu50_v[0-9]+',
#                                'HLT_Mu45_eta2p1_v[0-9]+',),
#
#    trigResultsSrc = cms.InputTag("TriggerResults", "", "HLT"),
#    trigPrescaleSrc = cms.InputTag("patTrigger"),
#
#    checkPrescale = cms.bool(False),
#    )
#
#triggerBranches_2016H = cms.PSet(
#    trigNames = cms.vstring(),# 'doubleMu', 'doubleMuDZ', 'doubleE',
#                              # 'singleESingleMu', 'singleMuSingleE',
#                              # 'tripleE', 'doubleESingleMu', 'doubleMuSingleE',
#                              # 'tripleMu', 'singleE', 'singleIsoMu',
#                              # 'singleIsoMu20', 'singleMu',),
#    doubleMuPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_v[0-9]+',
#                                'HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_v[0-9]+'),
#    doubleMuDZPaths = cms.vstring('HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_v[0-9]+',
#                                  'HLT_Mu17_TrkIsoVVL_TkMu8_TrkIsoVVL_DZ_v[0-9]+'),
#    doubleEPaths = cms.vstring('HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_v[0-9]+',
#                               'HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+',
#                               'HLT_DoubleEle33_CaloIdL_MW_MW_v[0-9]+',
#                               'HLT_DoubleEle33_CaloIdL_MW_v[0-9]+'),
#    singleESingleMuPaths = cms.vstring('HLT_Mu8_TrkIsoVVL_Ele23_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),
#    singleMuSingleEPaths = cms.vstring('HLT_Mu23_TrkIsoVVL_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v[0-9]+'),
#    tripleEPaths = cms.vstring('HLT_Ele16_Ele12_Ele8_CaloIdL_TrackIdL_v[0-9]+'),
#    doubleESingleMuPaths = cms.vstring('HLT_Mu8_DiEle12_CaloIdL_TrackIdL_v[0-9]+'),
#    doubleMuSingleEPaths = cms.vstring('HLT_DiMu9_Ele9_CaloIdL_TrackIdL_v[0-9]+'),
#    tripleMuPaths = cms.vstring('HLT_TripleMu_12_10_5_v[0-9]+'),
#    singleEPaths = cms.vstring('HLT_Ele35_WPTight_Gsf_v[0-9]+',
#                               'HLT_Ele32_WPTight_v[0-9]+',
#                               'HLT_Ele38_WPTight_Gsf_v[0-9]+',),
#    singleIsoMu20Paths = cms.vstring('HLT_IsoMu20_v[0-9]+',
#                                     'HLT_IsoTkMu20_v[0-9]+',),
#    singleIsoMuPaths = cms.vstring('HLT_IsoMu22_v[0-9]+',
#                                   'HLT_IsoTkMu22_v[0-9]+',
#                                   'HLT_IsoMu24_v[0-9]+',
#                                   'HLT_IsoTkMu24_v[0-9]+',),
#    singleMuPaths = cms.vstring('HLT_Mu50_v[0-9]+',
#                                'HLT_Mu45_eta2p1_v[0-9]+',),
#
#    trigResultsSrc = cms.InputTag("TriggerResults", "", "HLT"),
#    trigPrescaleSrc = cms.InputTag("patTrigger"),
#
#    checkPrescale = cms.bool(False),
#    )
