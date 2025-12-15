from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase
from UWVV.Utilities.helpers import mapObjects, parseChannels

import FWCore.ParameterSet.Config as cms


class GenZZXsecFlow(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isDressed'):
            self.isDressed = kwargs.pop('isDressed', False)
        super(GenZZXsecFlow, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(GenZZXsecFlow, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'selection':
            # select and cross clean gen jets
            mod = cms.EDProducer(
                "GenJetCleaner",
                src=step.getObjTag('j'),
                preselection=cms.string('pt > 20. && abs(eta) < 4.7'),
                checkOverlaps = cms.PSet(
                    electrons = cms.PSet(
                        src=step.getObjTag('e'),
                        preselection=cms.string(''),
                        deltaR=cms.double(0.4),
                        ),
                    muons = cms.PSet(
                        src=step.getObjTag('m'),
                        preselection=cms.string(''),
                        deltaR=cms.double(0.4),
                        ),
                    ),
                finalCut = cms.string(''),
                )
            step.addModule('genJetCleaner', mod, 'j')

        if stepName == 'intermediateStateCreation':
            zEEMod = cms.EDProducer(
                'PATCandViewShallowCloneCombiner',
                decay = cms.string('{0}@+ {0}@-'.format(step.getObjTagString('e'))),
                roles = cms.vstring('e1', 'e2'),
                cut = cms.string(self.__class__.getZEECuts()),
                checkCharge = cms.bool(True),
                setPdgId = cms.int32(23),
                )

            zMuMuMod = cms.EDProducer(
                'PATCandViewShallowCloneCombiner',
                decay = cms.string('{0}@+ {0}@-'.format(step.getObjTagString('m'))),
                roles = cms.vstring('m1', 'm2'),
                cut = cms.string(self.__class__.getZMMCuts()),
                checkCharge = cms.bool(True),
                setPdgId = cms.int32(23),
                )

            step.addModule("zEECreation", zEEMod, 'ee')
            step.addModule("zMuMuCreation", zMuMuMod, 'mm')

        if stepName == 'initialStateCreation':
            #pdb.set_trace()
            for chan in parseChannels('zz'):
                z1Name = 'z{}1'.format(chan[0])
                z2Name = 'z{}{}'.format(chan[2], 2 if chan[0] == chan[2] else 1)
                mod = cms.EDProducer(
                    'PATCandViewShallowCloneCombiner',
                    decay = cms.string('{0} {1}'.format(step.getObjTagString(chan[:2]),
                                                        step.getObjTagString(chan[2:]))),
                    roles = cms.vstring(z1Name, z2Name),
                    cut = cms.string(('4. < daughter("{}").mass && '
                                      '4. < daughter("{}").mass').format(z1Name, z2Name)),
                    checkCharge = cms.bool(False),
                    setPdgId = cms.int32(25),
                    )

                step.addModule(chan+'GenProducer', mod, chan)

        if stepName == 'initialStateEmbedding':
            for chan in parseChannels('zz'):
                mod = cms.EDProducer(
                    'AlternateDaughterInfoEmbedder',
                    src = step.getObjTag(chan),
                    names = cms.vstring(*mapObjects(chan)),
                    fsrLabel = cms.string(""),
                    )
                step.addModule(chan+'AlternatePairs', mod, chan)

        if stepName == 'initialStateSelection':
            for chan in parseChannels('zz'):
                cleaner = cms.EDProducer(
                    "GenZZCleaner",
                    src = step.getObjTag(chan),
                    l1PtCut = cms.double(20.),
                    l2PtCut = cms.double(10.),
                    l3PtCut = cms.double(5.),
                    l4PtCut = cms.double(5.),
                    etaCut = cms.double(2.5),
                    ossfMassCut = cms.double(4.),
                    z1MassMin = cms.double(40.),
                    z1MassMax = cms.double(120.),
                    z2MassMin = cms.double(4.),
                    z2MassMax = cms.double(120.),
                    )
                step.addModule(chan+'GenZZCleaner', cleaner, chan)

                xsecMod = cms.EDAnalyzer(
                    "GenZZXsecAnalyzer",
                    src = step.getObjTag(chan),
                    dressed = cms.bool(self.isDressed),
                    names = cms.vstring(*mapObjects(chan)),
                    label = cms.string(chan + "xsec"),
                )
                step.addModule(f'{chan}Xsec', xsecMod)

        return step

    @classmethod
    def getZEECuts(cls):
        return ('daughter("e1").pt > 5 && '
                'daughter("e2").pt > 5 && '
                'abs(daughter("e1").eta) < 2.5 && '
                'abs(daughter("e2").eta) < 2.5')


    @classmethod
    def getZMMCuts(cls):
        return ('daughter("m1").pt > 5 && '
                'daughter("m2").pt > 5 && '
                'abs(daughter("m1").eta) < 2.5 && '
                'abs(daughter("m2").eta) < 2.5')
