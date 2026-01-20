import FWCore.ParameterSet.Config as cms


wzCountBranches = cms.PSet(
    uints=cms.PSet(
        nCBVIDTightElec=cms.string('? hasUserInt("nCBVIDTightElec") ? userInt("nCBVIDTightElec") : 999'),
        nCBVIDMediumElec=cms.string('? hasUserInt("nCBVIDMediumElec") ? userInt("nCBVIDMediumElec") : 999'),
        nCBVIDLooseElec=cms.string('? hasUserInt("nCBVIDLooseElec") ? userInt("nCBVIDLooseElec") : 999'),
        nCBVIDHLTSafeElec=cms.string('? hasUserInt("nCBVIDHLTSafeElec") ? userInt("nCBVIDHLTSafeElec") : 999'),
        nCBVIDHLTSafeElecNoIP=cms.string(
            '? hasUserInt("nCBVIDHLTSafeElecNoIP") ? userInt("nCBVIDHLTSafeElecNoIP") : 999'
        ),
        nCBVIDVetoElec=cms.string('? hasUserInt("nCBVIDVetoElec") ? userInt("nCBVIDVetoElec") : 999'),
        nCBVIDVetoElecNoIP=cms.string('? hasUserInt("nCBVIDVetoElecNoIP") ? userInt("nCBVIDVetoElecNoIP") : 999'),
        nWWLooseElec=cms.string('? hasUserInt("nWWLooseElec") ? userInt("nWWLooseElec") : 999'),
        nWWLooseCBVIDMedElec=cms.string('? hasUserInt("nWWLooseCBVIDMedElec") ? userInt("nWWLooseCBVIDMedElec") : 999'),
        nWZTightMuon=cms.string('? hasUserInt("nWZTightMuon") ? userInt("nWZTightMuon") : 999'),
        nWZMediumMuon=cms.string('? hasUserInt("nWZMediumMuon") ? userInt("nWZMediumMuon") : 999'),
        nWZLooseMuon=cms.string('? hasUserInt("nWZLooseMuon") ? userInt("nWZLooseMuon") : 999'),
    ),
)

zzCountBranches = cms.PSet(
    uints=cms.PSet(
        nZZLooseElec=cms.string('? hasUserInt("nZZLooseElectrons") ? userInt("nZZLooseElectrons") : 999'),
        nZZTightElec=cms.string('? hasUserInt("nZZTightElectrons") ? userInt("nZZTightElectrons") : 999'),
        nZZTightIsoElec=cms.string('? hasUserInt("nZZTightIsoElectrons") ? userInt("nZZTightIsoElectrons") : 999'),
        nZZLooseMu=cms.string('? hasUserInt("nZZLooseMuons") ? userInt("nZZLooseMuons") : 999'),
        nZZTightMu=cms.string('? hasUserInt("nZZTightMuons") ? userInt("nZZTightMuons") : 999'),
        nZZTightIsoMu=cms.string('? hasUserInt("nZZTightIsoMuons") ? userInt("nZZTightIsoMuons") : 999'),
    ),
)
