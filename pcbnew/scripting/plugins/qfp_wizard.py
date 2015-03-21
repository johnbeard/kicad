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
import pcbnew

import math

from HelpfulFootprintWizardPlugin import HelpfulFootprintWizardPlugin as HFWP
import PadArray as PA


class NewApiCompat:
    #Please remove this code when new kicad python API will be the standard
    #Please DO NOT USE THIS CLASS IN OTHER SCRIPT WITHOUT ASKING
    #Ask on IRC if not sure.
    def __init__(self):
        self.layer_dict = {
            pcbnew.BOARD_GetStandardLayerName(n):
            n for n in range(pcbnew.LAYER_ID_COUNT)
        }

        self.layer_names = {
            s: n for n, s in self.layer_dict.iteritems()
        }

    def _get_layer(self, s):
        """Get layer id from layer name"""
        return self.layer_dict[s]

    def _to_LayerSet(self, layers):
        """Create LayerSet used for defining pad layers"""
        bitset = 0
        for l in layers:
            bitset |= 1 << self._get_layer(l)
        hexset = '{0:013x}'.format(bitset)
        lset = pcbnew.LSET()
        lset.ParseHex(hexset, len(hexset))
        return lset

    def _from_LayerSet(self, layerset):
        mask = [c for c in layerset.FmtBin() if c in ('0', '1')]
        mask.reverse()
        ids = [i for i, c in enumerate(mask) if c == '1']
        return tuple(self.layer_names[i] for i in ids)


class ThermalViasArray(PA.PadGridArray):
    def NamingFunction(self, x, y):
        return self.firstPadNum


class FootprintWithThermalPad(HFWP):

    def GenerateParameterList(self):

        self.AddParam("TPad", "tpad", self.uBool, True)
        self.AddParam("TPad", "W", self.uMM, 5)
        self.AddParam("TPad", "L", self.uMM, 5)

        self.AddParam("TPaste", "tpaste", self.uBool, True)
        self.AddParam("TPaste", "box rows", self.uNatural, 4)
        self.AddParam("TPaste", "box cols", self.uNatural, 4)
        self.AddParam("TPaste", "percent", self.uNatural, 50)

        self.AddParam("TVias", "tvias", self.uBool, True)
        self.AddParam("TVias", "rows", self.uNatural, 3)
        self.AddParam("TVias", "cols", self.uNatural, 3)
        self.AddParam("TVias", "drill", self.uMM, 0.3)
        self.AddParam("TVias", "size", self.uMM, 0.6)
        self.AddParam("TVias", "pitch", self.uMM, 1)

    def CheckParameters(self):

        self.CheckParamBool("TPad", "*tpad")

        self.CheckParamBool("TVias", "*tvias")

        self.CheckParamBool("TPaste", "*tpaste")
        self.CheckParamInt("TPaste", "*box rows", min_value=2)
        self.CheckParamInt("TPaste", "*box cols", min_value=2)
        self.CheckParamInt("TPaste", "*percent")

    def DrawThermalPadSolderPaste(self, x, y, rows, cols, percent):
        # Calculate the paste area given percentage
        x_total_size = x / (math.sqrt(1/(percent/100)))
        y_total_size = y / (math.sqrt(1/(percent/100)))

        x_box = x_total_size / cols
        y_box = y_total_size / cols

        x_spacer = (x - cols * x_box) / (cols - 1)
        y_spacer = (y - rows * y_box) / (rows - 1)

        x_step = x_spacer + x_box
        y_step = y_spacer + y_box

        # Use PAD as Paste only but Kicad complains
        # Is it a valid use ?
        pastepad = PA.PadMaker(self.module).SMDPad(y_box, x_box,
                                                   pcbnew.PAD_RECT)
        only_paste = pcbnew.LSET(pcbnew.F_Paste)
        pastepad.SetLayerSet(only_paste)

        array = ThermalViasArray(pastepad, cols, rows, x_step, y_step)
        #array.SetFirstPadInArray(self.parameters["Pads"]["*nbpads"]+1)
        array.SetFirstPadInArray('~')
        array.AddPadsToModule(self.draw)

    def DrawThermalPad(self):

        tpad = self.parameters["TPad"]
        pads = self.parameters["Pads"]
        tvias = self.parameters["TVias"]
        tpaste = self.parameters["TPaste"]

        if tpad["*tpad"]:

            thermal_pad = PA.PadMaker(self.module).SMDPad(
                tpad["W"], tpad["L"], pcbnew.PAD_RECT
            )

            # Use new kicad python api to have compatible layer set type
            compat = NewApiCompat()
            no_paste_lset = compat._to_LayerSet(('F.Cu', 'F.Mask'))
            thermal_pad.SetLayerSet(no_paste_lset)

            origin = pcbnew.wxPoint(0, 0)
            array = PA.PadLineArray(thermal_pad, 1, 0, False)
            array.SetFirstPadInArray(pads["*npads"] + 1)
            array.AddPadsToModule(self.draw)

            if tvias["*tvias"]:
                via_size = tvias["size"]
                via_drill = tvias["drill"]
                via_rows = tvias["*rows"]
                via_cols = tvias["*cols"]
                via_pitch = tvias["pitch"]
                thermal_via = PA.PadMaker(self.module).THRoundPad(
                    via_size, via_drill
                )

                array = ThermalViasArray(thermal_via, via_cols,
                                         via_rows, via_pitch, via_pitch)

                array.SetFirstPadInArray(pads["*npads"] + 1)
                array.AddPadsToModule(self.draw)

            if tpaste["*tpaste"]:
                self.DrawThermalPadSolderPaste(
                    tpad["L"], tpad["W"],
                    tpaste["*box rows"], tpaste["*box cols"],
                    tpaste["*percent"]
                )


class QFPNWizardBase(FootprintWithThermalPad):
    """
    This is the basic wizard for QFN/P wizards. Subclass from it to
    provide alternative ways of specifying the geometry
    """

    def GetReferencePrefix(self):
        return "U"

    def GenerateParameterList(self):

        FootprintWithThermalPad.GenerateParameterList(self)

        self.AddParam("Pads", "npads", self.uNatural, 100)
        self.AddParam("Pads", "oval", self.uBool, True)

        # pad spacing and pad sizes
        self.AddParam("Pads", "pad pitch", self.uMM, 0.5)
        self.AddParam("Pads", "pad width", self.uMM, 0.25)
        self.AddParam("Pads", "pad length", self.uMM, 1.5)

        self.AddParam("Pads", "pad extension", self.uMM, 0.3)

        self.AddParam("Pads", "package width", self.uMM, 14)
        self.AddParam("Pads", "package length", self.uMM, 14)

    def CheckParameters(self):

        FootprintWithThermalPad.CheckParameters(self)

        self.CheckParamInt("Pads", "*npads", is_multiple_of=4)
        self.CheckParamBool("Pads", "*oval")

    def _PadsPerRow(self):
        return self.parameters["Pads"]["*npads"] // 4

    def _RowLength(self):
        pitch = self.parameters["Pads"]["pad pitch"]
        return (self._PadsPerRow() - 1) * pitch

    def DrawQFNP(self, h_pitch, v_pitch):
        """
        Common method to draw the main features of the package.

        You need to calculate the "pitches" for the pads - this is the
        centre-to-centre distance between two opposite rows, NOT
        including the pad extension
        """

        self.DrawThermalPad()

        pads = self.parameters["Pads"]

        # add extensions
        fillet = pads["pad extension"]
        h_pitch += fillet
        v_pitch += fillet

        print h_pitch, v_pitch

        # draw the 4 rows of pads

        pad_pitch = pads["pad pitch"]
        pad_length = self.parameters["Pads"]["pad length"] + fillet
        pad_width = self.parameters["Pads"]["pad width"]

        pads_per_row = self._PadsPerRow()
        row_len = self._RowLength()

        pad_shape = pcbnew.PAD_OVAL if pads["*oval"] else pcbnew.PAD_RECT

        h_pad = PA.PadMaker(self.module).SMDPad(
            pad_width, pad_length, shape=pad_shape)
        v_pad = PA.PadMaker(self.module).SMDPad(
            pad_length, pad_width, shape=pad_shape)

        #left row
        pin1Pos = pcbnew.wxPoint(-h_pitch / 2, 0)
        array = PA.PadLineArray(h_pad, pads_per_row, pad_pitch, True,
                                pin1Pos)
        array.SetFirstPadInArray(1)
        array.AddPadsToModule(self.draw)

        #bottom row
        pin1Pos = pcbnew.wxPoint(0, v_pitch / 2)
        array = PA.PadLineArray(v_pad, pads_per_row, pad_pitch, False,
                                pin1Pos)
        array.SetFirstPadInArray(pads_per_row + 1)
        array.AddPadsToModule(self.draw)

        #right row
        pin1Pos = pcbnew.wxPoint(h_pitch / 2, 0)
        array = PA.PadLineArray(h_pad, pads_per_row, -pad_pitch, True,
                                pin1Pos)
        array.SetFirstPadInArray(2*pads_per_row + 1)
        array.AddPadsToModule(self.draw)

        #top row
        pin1Pos = pcbnew.wxPoint(0, -v_pitch / 2)
        array = PA.PadLineArray(v_pad, pads_per_row, -pad_pitch, False,
                                pin1Pos)
        array.SetFirstPadInArray(3*pads_per_row + 1)
        array.AddPadsToModule(self.draw)

        #reference and value
        text_size = pcbnew.FromMM(1.2)  # IPC nominal

        text_offset = v_pitch / 2 + text_size + pad_length / 2

        self.draw.Value(0, -text_offset, text_size)
        self.draw.Reference(0, text_offset, text_size)

        # draw the silkscreen
        lim_x = pads["package width"] / 2
        lim_y = pads["package length"] / 2
        inner = (self._RowLength() / 2) + pads['pad pitch']
        outer = (h_pitch + pads['pad length'] + pads['pad extension']) / 2

        # top right
        self.draw.Polyline(
            [(inner, -lim_y), (lim_x, -lim_y), (lim_x, -inner)])
        # bottom left
        self.draw.Polyline(
            [(-inner, lim_y), (-lim_x, lim_y), (-lim_x, inner)])
        # bottom right
        self.draw.Polyline(
            [(inner, lim_y), (lim_x, lim_y), (lim_x, inner)])

        #top left - extra "tail" for pin 1
        self.draw.Polyline(
            [(-inner, -lim_y), (-lim_x, -lim_y),
             (-lim_x, -inner), (-outer, -inner)]
        )

        # TODO - need options for pin 1 polarity?


class QFPWizard(QFPNWizardBase):
    """
    Specified according to the pitch of the rows - i.e. the
    centre-to-centre distance of the pads. The pad extension is not
    included in this calculation.

    The package outline is an independent size to the pad layout
    and only affects the silk screen
    """

    def GetName(self):
        return "QFP"

    def GetDescription(self):
        return "QFP (Quad Flat Pack) Wizard"

    def GetValue(self):
        return "QFP %d" % self.parameters["Pads"]["*npads"]

    def GenerateParameterList(self):

        QFPNWizardBase.GenerateParameterList(self)

        self.AddParam("Pads", "vertical pitch", self.uMM, 15)
        self.AddParam("Pads", "horizontal pitch", self.uMM, 15)

    def CheckParameters(self):

        QFPNWizardBase.CheckParameters(self)

        self.CheckParamInt("Pads", "*npads", is_multiple_of=4)
        self.CheckParamBool("Pads", "*oval")

    def BuildThisFootprint(self):

        pads = self.parameters["Pads"]

        # this method gives us the pad pitch directly
        h_pitch = pads["horizontal pitch"]
        v_pitch = pads["vertical pitch"]

        self.DrawQFNP(h_pitch, v_pitch)

QFPWizard().register()


class QFNWizard(QFPNWizardBase):
    """
    QFN Wizard - specified by the size of the package body,
    with the pads protruding inwards from the edge. The pad extension
    is added on otside the body boundary.
    """

    def GetName(self):
        return "QFN"

    def GetDescription(self):
        return "QFN (Quad Flat No-leads) Wizard"

    def GetValue(self):
        pads = self.parameters["Pads"]
        return "QFN%d_%dx%dmm" % (pads["*npads"],
                                  pads["package width"],
                                  pads["package length"])

    def GenerateParameterList(self):

        QFPNWizardBase.GenerateParameterList(self)

        # override with different defaults
        self.AddParam("Pads", "npads", self.uNatural, 16)

        self.AddParam("Pads", "pitch", self.uMM, 0.5)
        self.AddParam("Pads", "pad width", self.uMM, 0.25)
        self.AddParam("Pads", "pad length", self.uMM, 0.4)

        self.AddParam("Pads", "package width", self.uMM, 4)
        self.AddParam("Pads", "package length", self.uMM, 4)

        self.AddParam("TPad", "W", self.uMM, 2.6)
        self.AddParam("TPad", "L", self.uMM, 2.6)

    def CheckParameters(self):
        QFPNWizardBase.CheckParameters(self)

    def BuildThisFootprint(self):
        pads = self.parameters["Pads"]

        pad_len = pads["pad length"]

        # pitch of the pads is such that the pads end at the package body
        v_pitch = pads["package length"] - pad_len
        h_pitch = pads["package width"] - pad_len

        self.DrawQFNP(h_pitch, v_pitch)

QFNWizard().register()
