from UWVV.AnalysisTools.AnalysisFlowBase import AnalysisFlowBase

import FWCore.ParameterSet.Config as cms

class ZZCrossCleaning(AnalysisFlowBase):
    def __init__(self, *args, **kwargs):
        if not hasattr(self, 'isMC'):
            self.isMC = kwargs.pop('isMC', True)
        if not hasattr(self, 'debug'):
            self.debug = kwargs.pop('debug', False)
        super(ZZCrossCleaning, self).__init__(*args, **kwargs)

    def makeAnalysisStep(self, stepName, **inputs):
        step = super(ZZCrossCleaning, self).makeAnalysisStep(stepName, **inputs)

        if stepName == 'selection':
            if self.debug:
                step.addBasicCounter('e', "preCrossCleaningCounting", nPreCrossCleanedElectrons="")
            # remove electrons near tight muons
            step.addCrossSelector(
                'e', 
                '', # no further basic selection here
                m={
                    'deltaR' : 0.05,
                    'selection' : 'userFloat("{}Tight") > 0.5'.format(self.getZZIDLabel()),
                }
            )
            if self.debug:
                step.addBasicCounter('e', "crossCleaningCounting", nCrossCleanedElectrons="")

            # remove jets close to tight, isolated electrons and muons
            step.addCrossSelector(
                'j',
                '', # no further basic selection here
                e={
                    'deltaR' : 0.4,
                    'selection' : ('userFloat("{}Tight") > 0.5 && '
                                   'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                   self.getZZIsoLabel()),
                },
                m={
                    'deltaR' : 0.4,
                    'selection' : ('userFloat("{}Tight") > 0.5 && '
                                   'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                   self.getZZIsoLabel()),
                },
            )

            if self.isMC:
                step.addCrossSelector(
                    'j_jesUp',
                    '', # no further basic selection here
                    e={
                        'deltaR' : 0.4,
                        'selection' : ('userFloat("{}Tight") > 0.5 && '
                                       'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                       self.getZZIsoLabel()),
                    },
                    m={
                        'deltaR' : 0.4,
                        'selection' : ('userFloat("{}Tight") > 0.5 && '
                                       'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                       self.getZZIsoLabel()),
                    },
                )
                step.addCrossSelector(
                    'j_jesDown',
                    '', # no further basic selection here
                    e={
                        'deltaR' : 0.4,
                        'selection' : ('userFloat("{}Tight") > 0.5 && '
                                       'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                       self.getZZIsoLabel()),
                    },
                    m={
                        'deltaR' : 0.4,
                        'selection' : ('userFloat("{}Tight") > 0.5 && '
                                       'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                       self.getZZIsoLabel()),
                    },
                )
                step.addCrossSelector(
                    'j_jerUp',
                    '', # no further basic selection here
                    e={
                        'deltaR' : 0.4,
                        'selection' : ('userFloat("{}Tight") > 0.5 && '
                                       'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                       self.getZZIsoLabel()),
                    },
                    m={
                        'deltaR' : 0.4,
                        'selection' : ('userFloat("{}Tight") > 0.5 && '
                                       'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                       self.getZZIsoLabel()),
                    },
                )
                step.addCrossSelector(
                    'j_jerDown',
                    '', # no further basic selection here
                    e={
                        'deltaR' : 0.4,
                        'selection' : ('userFloat("{}Tight") > 0.5 && '
                                       'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                       self.getZZIsoLabel()),
                    },
                    m={
                        'deltaR' : 0.4,
                        'selection' : ('userFloat("{}Tight") > 0.5 && '
                                       'userFloat("{}") > 0.5').format(self.getZZIDLabel(),
                                                                       self.getZZIsoLabel()),
                    },
                )

        return step
