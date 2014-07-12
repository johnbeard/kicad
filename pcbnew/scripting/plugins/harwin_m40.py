from __future__ import division

import pcbnew
from pcbnew import FromMM as fmm

import ConnectorWizard as CW
import inline_headers

class HarwinM40():

    def Manufacturer(self):
        return "Harwin"

    def PartRangeName(self):
        return "M40"

    def GetDescription(self):
        return "Harwin M40 1.00mm shrouded header"

    def GetName(self):
        return "Harwin M40"

class HarwinM40SMDHeader(HarwinM40,
                        inline_headers.SMDInlineHeaderWithWings):

    def HaveRaOption(self):
        return True

    def GetValue(self):
        pn = "401" if self.RightAngled() else "301"
        ref = "-%s%02d46" % (pn, self.N())

        return CW.ConnectorWizard.GetValue(self, ref)

    def GetComponentParams(self):

        return {
            'A': fmm(0.6),
            'B': fmm(1.5) if self.RightAngled() else fmm(1.6),
            'C' : fmm(1.2),
            'D' : fmm(1.8),
            'E' : fmm(2.2) if self.RightAngled() else fmm(0.6),
            'F' : fmm(0.7),
            'G' : fmm(1.5),
            'H' : fmm(0.3) if self.RightAngled() else fmm(-0.2),
            'J' : fmm(2.2),
            'd' : fmm(1.0) if self.RightAngled() else fmm(-1.0),
            }
