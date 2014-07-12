#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#
from __future__ import division

import math

import pcbnew
import HelpfulFootprintWizardPlugin as HFPW


class ConnectorWizard(HFPW.HelpfulFootprintWizardPlugin):

    # connector options
    SMD = "SMD"
    VERT = "Vert"
    RA = "RA"

    def GenerateParameterList(self):
        if self.HaveRaOption():
            self.AddParam("Pads", "ra", self.uBool, False)

        if (self.HaveSMDOption()):
            self.AddParam("Pads", "smd", self.uBool, False)

        self.AddParam("Pads", "n", self.uNatural, 4)

        self.AddParam("Pads", "hand soldering ext", self.uMM, 0);

    def CheckParameters(self):
        if (self.HaveRaOption()):
            self.CheckParamBool("Pads", "*ra")

        if (self.HaveSMDOption()):
            self.CheckParamBool("Pads", "*smd")

        self.CheckParamInt("Pads", "*n")

    def GetReferencePrefix(self):
        return "J"

    def HaveRaOption(self):
        return False;

    def RightAngled(self):
        return self.HaveRaOption() and self.parameters["Pads"]["*ra"]

    def HandSolderingExt(self):
        return self.parameters["Pads"]["hand soldering ext"]

    def HaveSMDOption(self):
        return False;

    def IsSMD(self):
        return self.HaveSMDOption() and self.parameters["Pads"]["*smd"]

    def Manufacturer(self):
        return None

    def PartRangeName(self):
        return None

    def N(self):
        return self.parameters["Pads"]["*n"]

    def GetValue(self, partRef):

        ref = "Connector"

        if self.Manufacturer():
            ref += "_" + self.Manufacturer()

        if self.PartRangeName():
            ref += "_" + self.PartRangeName()

        ref += partRef

        return ref

    def GetWays(self):
        return self.N()

    def SetModuleDescription(self):
        var = []

        var.append("Right-angled" if self.RightAngled() else "Vertical")

        if self.IsSMD():
            var.append("SMD")

        s = "%s" % (self.GetDescription())

        if var:
            s += ". " + ", ".join(var)

        s += ". %s ways" % self.GetWays()

        self.module.SetDescription(s)

    def DrawPin1MarkerAndTexts(self, topMargin, bottomMargin, markerX):

        refOnTop = topMargin < bottomMargin

        markerOffset = pcbnew.FromMM(0.5)
        markerY = -topMargin if refOnTop else bottomMargin
        markerY += math.copysign(markerOffset, markerY)

        markerDir = self.draw.dirS if refOnTop else self.draw.dirN

        self.draw.MarkerArrow(markerX, markerY, direction=markerDir)

        topTextY = -topMargin
        bottomTextY = bottomMargin

        refY = topTextY if refOnTop else bottomTextY
        valY = topTextY if not refOnTop else bottomTextY

        size = self.GetTextSize()

        refY += math.copysign(size, refY)
        valY += math.copysign(size, valY)

        self.draw.Reference(0, refY, size)
        self.draw.Value(0, valY, size)
