import FWCore.ParameterSet.Config as cms


process = cms.Process("Print4Mu")

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(
        "/store/mc/RunIIFall15MiniAODv2/GluGluHToZZTo4L_M2500_13TeV_powheg2_JHUgenV6_pythia8/MINIAODSIM/PU25nsData2015v1_76X_mcRun2_asymptotic_v12-v1/60000/02C0EC1D-F3E4-E511-ADCA-AC162DA603B4.root"
    ),
)

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(100))

process.selectMuons = cms.EDFilter(
    "PATMuonRefSelector",
    src=cms.InputTag("slimmedMuons"),
    cut=cms.string("pt > 5. && abs(eta) < 2.4"),
    filter=cms.bool(False),
)

process.zMuMu = cms.EDProducer(
    "PATCandViewShallowCloneCombiner",
    decay=cms.string("selectMuons@+ selectMuons@-"),
    roles=cms.vstring("m1", "m2"),
    cut=cms.string('abs(daughter("m1").masterClone.eta) < 2.4 && abs(daughter("m2").masterClone.eta) < 2.4'),
    checkCharge=cms.bool(True),
)


process.zz4Mu = cms.EDProducer(
    "PATCandViewShallowCloneCombiner",
    decay=cms.string("zMuMu zMuMu"),
    roles=cms.vstring("Z1", "Z2"),
    cut=cms.string('4. < daughter("Z1").masterClone.mass < 120. && 4. < daughter("Z2").masterClone.mass < 120.'),
    checkCharge=cms.bool(False),
)

process.makeCands = cms.Sequence(process.selectMuons * process.zMuMu * process.zz4Mu)

process.printCands = cms.EDAnalyzer(
    "CandidatePrinter",
    src=cms.InputTag("zz4Mu"),
)

process.p = cms.Path(process.makeCands * process.printCands)
