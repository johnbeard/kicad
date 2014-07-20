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

"""
Base classes for SMD and THT headers
"""

from __future__ import division

import pcbnew
from pcbnew import FromMM as fmm

import HelpfulFootprintWizardPlugin as HFPW
import PadArray as PA

class LemoSocket(HFPW.HelpfulFootprintWizardPlugin):

    class LemoTypeError(ValueError):
        pass

    def GetName(self):
        return "Lemo B-series Socket"

    def GetDescription(self):
        return "Lemo B-series Socket"

    def GetValue(self):
        n = self.parameters['Pads']['*pins']
        size = self.parameters['Pads']['*size']
        model = self.parameters['Pads']['*model']
        contact = self.parameters['Pads']['*contact']

        return "Connector_Lemo_%s-%s-3%02d-xx%s" % (model, size, n, contact)

    def GetReferencePrefix(self):
        return "J"

    def GenerateParameterList(self):

        self.AddParam("Pads", "pins", self.uNatural, 3)
        self.AddParam("Pads", "size", self.uString, "OB")
        self.AddParam("Pads", "model", self.uString, "EZG")
        self.AddParam("Pads", "contact", self.uString, "N")

    def CheckParameters(self):

        self.CheckParamInt("Pads", "*pins", min_value=2)

        known_sizes = [0, '0B', '1B', '2B', '3B', '4B', '5B']

        if self.parameters['Pads']['*size'] not in known_sizes:
            self.parameter_errors['Pads']['*size'] = (
                "Size must be one of %s" % known_sizes)

        # hack fix 00 strings - this is annoying!
        if self.parameters['Pads']['*size'] == 0:
            self.parameters['Pads']['*size'] = "00"

        known_models = ['EZG', 'EYG', 'XPF', 'ECG']

        if self.parameters['Pads']['*model'] not in known_models:
            self.parameter_errors['Pads']['*model'] = (
                "Model must be one of %s" % known_models)

    @staticmethod
    def get_contact_types_for_model(model):

        if model in ["EZG", "EYG", "XPF"]:
            return ["N"] # "D" is the same as far as footrprint goes
        elif model in ["ECG"]:
            return ["N", "V"]
        else:
            raise LemoSocket.LemoTypeError("Unknown model: %s" % model)

    @staticmethod
    def get_sizes_for_model(model, contact_type):
        sizes = []

        def raise_if_not_stright(contact):
            if contact_type not in ["D", "N"]:
                raise LemoSocket.LemoTypeError(
                    "EZG/EYG only have straight contacts"
                )

        if model in ["EZG", "EYG"]:
            raise_if_not_stright(contact_type)
            sizes = ["00", "0B", "1B", "2B"]
        elif model == "XPF":
            raise_if_not_stright(contact_type)
            sizes = ["0B"]
        elif model == "ECG" and contact_type in ["D", "N"]:
            sizes = ["00", "0B", "1B", "2B", "3B", "4B", "5B"]
        elif model == "ECG" and contact_type == "V":
            sizes = ["00", "0B", "1B", "2B", "3B"]

        return sizes

    @staticmethod
    def get_bk_pin_counts_for_size(size, contact):

        if size == "00":
            counts = [2, 3, 4]
        elif size == "0B":
            if contact in ["D", "N"]:
                counts = [2, 3, 4, 5, 6, 7, 9]
            else:
                counts = [2, 3, 4, 5, 6, 7]

        elif size == "1B":
            counts = [2, 3, 4, 5, 6, 7, 8, 10, 14, 16]
        elif size == "2B":

            if contact in ["D", "N"]:
                counts = [2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 19, 32]
            else:
                counts = [2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 19]
        elif size == "3B":
            if contact in ["D", "N"]:
                counts = [3, 4, 5, 6, 7, 8, 9, 10, 12, 14, 16, 18, 20, 22, 24, 26, 30]
            else:
                counts = [3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 18, 20, 24, 30]
        elif size == "4B":
            counts = [16, 20, 24, 30, 40, 48]
        elif size == "5B":
            counts = [50, 54, 64]

        else:
            raise LemoSocket.LemoTypeError("Unknown size: %s" % size)

        return counts

    def get_straight_pin_size(self, n, size):

        pin_size = 0.8  # this is most usual

        if ((size == "00") or (size == "0B" and n > 3)
                or (size == "1B" and n > 9)
                or (size == "2B" and n > 19)
                or (size == "3B" and n > 19)
                or (size == "4B")
                or (size == "5B")):
            pin_size = 0.6

        return fmm(pin_size)

    def layout_straight_pads(self, n, size):

        pin_size = self.get_straight_pin_size(n, size)

        pad = PA.PadMaker(self.module).THRoundPad(pin_size + fmm(0.4), pin_size)

        # each ring list of tuples (number in ring, angle) for each ring,
        # then the dias for each ring
        pad_info = {
            2:  ([(2, 0)],
                 {'00': [1.2],
                  '0B': [2.2],
                  '1B': [2.8],
                  '2B': [4.4]}),
            3:  ([(3, 0)],
                 {'00': [1.35],
                  '0B': [2.3],
                  '1B': [3.0],
                  '2B': [4.6],
                  '3B': [5.6]}),
            4:  ([(4, 45)],
                 {'00': [1.6],
                  '0B': [2.5],
                  '1B': [3.1],
                  '2B': [5.0],
                  '3B': [6.]}),
            5:  ([(5, 0)],
                  {'0B': [2.8],
                  '1B': [3.4],
                  '2B': [5.2],
                  '3B': [6.7]}),
            6:  ([(6, 30)] if (size in ['0B', '1B']) else [(5, 36), (1, 0)],
                 {'0B': [3.0],
                  '1B': [3.7],
                  '2B': [5.6, 0],
                  '3B': [7.1, 0]}),
            7:  ([(6, 30), (1, 0)],
                 {'0B': [3.0, 0],
                  '1B': [3.7, 0],
                  '2B': [5.8, 0],
                  '3B': [7.08, 0]}),
            8:  ([(8, 0)] if (size in ['2B', '3B']) else [(7, 0), (1, 0)],
                 {'1B': [3.8, 0],
                  '2B': [6.4],
                  '3B': [7.5]}),
            9:  ([(8, 0), (1, 0)],
                 {'0B': [3.2, 0],
                  '3B': [7.5, 0]}),
            10:  ([(9, 360/18), (2, 0)],
                 {'1B': [3.95, 1.4],
                  '2B': [6.3, 2.15],
                  '3B': [7.9, 2.8]}),
            12:  ([(8, 22.5), (4, 45)],
                 {'2B': [6.5, 2.8],
                  '3B': [8.2, 3.4]}),
            14:  ([(10, 360/20), (4, 0)] if (size != '1B') else
                        [(10, 360/20), (1, 0), (1, 90), (1, 180), (1, 270)],
                 {'1B': [4.4, 1.9, 1.8, 1.9, 1.8],
                  '2B': [6.5, 2.65],
                  '3B': [8.2, 3.4]}),
            16:  ([(11, 0), (5, 360/10)] if (size in ['1B']) else [(11, 360/22), (5, 0)],
                 {'1B': [4.4, 2.0],
                  '2B': [6.6, 3.1],
                  '3B': [8.4, 3.86],
                  '4B': [10.5, 5]}),
            18:  ([(12, 360/24), (6, 0)],
                 {'2B': [6.7, 3.5],
                  '3B': [8.4, 4.34]}),
            19:  ([(12, 360/24), (6, 0), (1, 0)],
                 {'2B': [6.7, 3.5, 0]}),
            20:  ([(13, 0), (7, 360/14)],
                 {'3B': [8.62, 4.78],
                  '4B': [11, 6]}),
            22:  ([(14, 360/28), (8, 360/16)],
                 {'3B': [8.9, 5]}),
            24:  ([(14, 360/28), (8, 0), (2, -90)],
                 {'3B': [8.8, 5.3, 1.8],
                  '4B': [11.1, 6.65, 2.2]}),
            26:  ([(14, 360/28), (9, 360/18), (3, 0)],
                 {'2B': [7, 4.2, 1.6],
                  '3B': [9, 5.6, 2.1]}),
            # this one is a bit of a mess...
            30:  ([(16, 360/32), (10, 0), (1, 0), (1, 90), (1, 180), (1, 270)]
                    if (size == '3B') else
                    [(16, 360/32), (1, 0), (1, 36), (1, 36*2), (1, 36*3),
                     (1, 36*4), (1, 36*5), (1, 36*6), (1, 36*7),
                     (1, 36*8), (1, 36*9), (4, 0)],
                 {'3B': [9, 5.7, 2.2, 2.6, 2.2, 2.6],
                  '4B': [11.4, 7.4, 7.2, 7.4, 7.4, 7.2, 7.4, 7.2, 7.4, 7.4, 7.2, 3.1]}),
            32:  ([(17, 360/34), (10, 360/20), (5, 0)],
                 {'2B': [7, 4.6, 2.2]}),
            40:  ([(20, 360/40), (13, 0), (7, 0)],
                 {'4B': [11.64, 7.94, 4.24]}),
            48:  ([(21, 360/42), (15, 0), (9, -360/18), (3, 0)], # 5Bs also have odd 3rd ring numbers
                 {'4B': [12, 8.65, 5.3, 2]}),
            50:  ([(23, 360/46), (15, 360/30), (9, 360/18), (3, 0)],
                 {'5B': [19.2, 13.73, 8.36, 3.1]}),
            54:  ([(24, 360/48), (16, 360/32), (10, 360/20), (4, 0)],
                 {'5B': [19.4, 14.2, 9, 3.8]}),
            64:  ([(26, 360/52), (19, 360/38), (13, 360/26), (6,0)],
                 {'5B': [19.84,14.96, 10.08, 5.05]}),
        }

        try:
            rings, dias = pad_info[n]
        except IndexError:
            raise LemoSocket.LemoTypeError("Unknown pin count: %d" % n)

        # get the right ring sizes
        dias = dias[size]

        # and now do each ring in turn
        pin_num = 1
        for i in range(len(rings)):
            ring_dia = dias[i]
            n_in_ring, angle = rings[i]

            array = PA.PadCircleArray(pad, n_in_ring, fmm(ring_dia)/2,
                                      angle_offset=angle,
                                      clockwise=False)
            array.SetFirstPadInArray(pin_num)

            array.AddPadsToModule(self.draw)

            pin_num += n_in_ring

    @staticmethod
    def get_elbow_pin_size(n, size):

        pin_size = 0.7  # this is most usual

        if (size == "00"):  # all 00's are 0.6
            pin_size = 0.6
        elif ((size == "1B" and n < 4)
                or (size == "2B" and n < 8)
                or (size == "3B" and n < 11)):
            pin_size = 0.9

        return fmm(pin_size)

    def layout_elbow_pin_pads(self, n, size):

        pin_size = self.get_elbow_pin_size(n, size)
        pad = PA.PadMaker(self.module).THRoundPad(pin_size + fmm(0.4), pin_size)

        # nearly all layouts based on a 1.27mm grid, but not all
        x1 = 1.27
        x2 = 1.27 * 2
        x3 = 1.27 * 3
        x4 = 1.27 * 4
        x5 = 1.27 * 5
        x6 = 1.27 * 6

        if n == 2:
            pads = [[-1.27,0], [1.27, 0]]
        elif n == 3:
            sep = 2.54 if (size != "2B") else 1.27
            pads = [[-1.27, 0], [1.27, sep], [1.27, -sep]]

        elif n == 4:
            sep = 1.27 if (size != "2B") else 3.50/2
            pads = [[-1.27, sep], [1.27, sep], [1.27, -sep],
                        [-1.27, -sep]]
        elif n == 5:

            if size in ["0B", "2B"]:
                sep = 1.27 if size == "0B" else 2.54
                pads = [[-2.54, 0], [0, sep], [2.54, sep],
                        [2.54, -sep], [0, -sep]]
            else:
                pads = [[-2.54, 0], [0, 2.54], [2.54, 1.27],
                        [2.54, -1.27], [0, -2.54]]
        elif n == 6:

            if size in ["0B", "1B"]:
                pads = [[-2.54, 1.27], [0, 2.54], [2.54, 1.27],
                        [2.54, -1.27], [0, -2.54], [-2.54, -1.27]]
            else:
                pads = [[-2.54, 1.27], [0, 2.54], [2.54, 0],
                        [0, -2.54], [-2.54, -1.27], [0, 0]]

        elif n == 7:

            pads = [[-2.54, 1.27], [0, 2.54], [2.54, 1.27],
                    [2.54, -1.27], [0, -2.54], [-2.54, -1.27],
                    [0, 0]]

        elif n == 8:
            if size == "1B":
                pads = [[-2.54, 0], [-2.54, 1.84], [0, 1.84],
                    [2.54, 0.92], [2.54, -0.92], [0, -1.84],
                    [-2.54, -1.84], [0, 0]]
            else:
                pads = [[-2.54, 0], [-2.54, 2.54], [0, 2.54],
                        [2.54, 2.54], [2.54, 0], [2.54, -2.54],
                        [0, -2.54], [-2.54, -2.54]]

        elif n == 10:

            pads = [[-x3, x1], [-x1, x2], [x1, x2], [x3, x1],
                    [x3, -x1], [x1, -x2], [-x1, -x2], [-x3, -x1],
                    [-x1, 0], [x1, 0]]

        elif n == 12:

            pads = [[-x3, x1], [-x1, x3], [x1, x3], [x3, x1],
                    [x3, -x1], [x1, -x3], [-x1, -x3], [-x3, -x1],
                    [-x1, x1], [x1, x1], [x1, -x1], [-x1, -x1]]

        elif n == 14: # two diagrams in the datasheet, but they are the same?

            pads = [[-x4, x1], [-x2, x2], [0, x3], [x2, x2], [x4, x1],
                    [x4, -x1], [x2, -x2], [0, -x3], [-x2, -x2], [-x4, -x1],
                    [-x2, 0], [0, x1], [x2, 0], [0, -x1]]

        elif n == 16:

            pads = [[-x4, x1], [-x2, x2], [0, x3], [x2, x3], [x4, x2],
                    [x4, 0], [x4, -x2], [x2, -x3], [0, -x3], [-x2, -x2], [-x4, -x1],
                    [-x2, 0], [0, x1], [x2, x1], [x2, -x1], [0, -x1]]

        elif n == 18:

            if size == "2B":
                pads = [[-x3, x1], [-x2, x2], [-x1, x3], [x1, x3], [x2, x2], [x3, x1],
                        [x3, -x1], [x2, -x2], [x1, -x3], [-x1, -x3], [-x2, -x2], [-x3, -x1],
                        [-x2, 0],
                        [-x1, x1], [x1, x1], [x1, -x1], [-x1, -x1],
                        [x2, 0]]
            else:
                pads = [[-x4, x1], [-x4, x3], [-x2, x4], [x2, x4], [x4, x3], [x4, x1],
                        [x4, -x1], [x4, -x3], [x2, -x4], [-x2, -x4], [-x4, -x3], [-x4, -x1],
                        [-x2, 0], [-x2, x2], [x2, x2], [x2, 0], [x2, -x2], [-x2, -x2]]

        elif n == 19:
            pads = [[-x3, x1], [-x2, x2], [-x1, x3], [x1, x3], [x2, x2], [x3, x1],
                    [x3, -x1], [x2, -x2], [x1, -x3], [-x1, -x3], [-x2, -x2], [-x3, -x1],
                    [-x2, 0],
                    [-x1, x1], [x1, x1], [x1, -x1], [-x1, -x1],
                    [x2, 0], [0, 0]]

        elif n == 20:
            pads = [[-x4, 0], [-x4, x2], [-x2, x3], [0, x4], [x2, x4], [x4, x3], [x4, x1],
                    [x4, -x1], [x4, -x3], [x2, -x4], [0, -x4], [-x2, -x3], [-x4, -x2],
                    [-x2, -x1], [-x2, x1], [0, x2], [x2, x2], [x2, 0], [x2, -x2], [0, -x2]]

        elif n == 24:
            pads = [[-x4, x1], [-x4, x3], [-x2, x4], [0, x5], [x2, x4], [x4, x3], [x4, x1],
                    [x4, -x1], [x4, -x3], [x2, -x4], [0, -x5], [-x2, -x4], [-x4, -x3], [-x4, -x1],
                    [-x2, 0], [-x2, x2], [0, x3], [x2, x2],
                    [x2, 0], [x2, -x2], [0, -x3], [-x2, -x2],
                    [0, -x1], [0, x1]]

        elif n == 30:

            pads = [[-x6, x1], [-x6, x3], [-x4, x4], [-x2, x4],
                    [x2, x4], [x4, x4], [x6, x3], [x6, x1],
                    [x6, -x1], [x6, -x3], [x4, -x4], [x2, -x4],
                    [-x2, -x4], [-x4, -x4], [-x6, -x3], [-x6, -x1],
                    [-x4, 0], [-x4, x2], [-x2, x2],
                    [x2, x2], [x4, x2], [x4, 0],
                    [x4, -x2], [x2, -x2], [-x2, -x2], [-x4, -x2],
                    [-x2, 0], [0, x1], [x2, 0], [0, -x1]]

        else:
            pads = []

        pads = [[fmm(x[0]), fmm(x[1])] for x in pads]

        # and now do each ring in turn
        pin_num = 1
        for pad_loc in pads:

            array = PA.PadCustomArray(pad, pads)

            array.AddPadsToModule(self.draw)

    def ezg_support_pins(self, size):
        # four support pins
        support_spacings = {'00': fmm(5.08),
                            '0B': fmm(7.62),
                            '1B': fmm(7.62),
                            '2B': fmm(10.16)
                            }

        support_spacing = support_spacings[size]

        #solder pin spacing
        if size == "00":
            pad = PA.PadMaker(self.module).THRoundPad(fmm(1.5), fmm(0.8))
        else:
            pad = PA.PadMaker(self.module).NPTHRoundPad(fmm(1.8))

        supports = PA.PadGridArray(pad, 2, 2, support_spacing, support_spacing)
        supports.SetPinNames("")
        supports.AddPadsToModule(self.draw)

    def ezg_outline(self, size):
        # build silkscreen outline
        body_size = {'00': fmm(7),
                    '0B': fmm(10),
                    '1B': fmm(12),
                    '2B': fmm(15)
            }[size]

        self.draw.NotchedBox(0, 0, body_size, body_size,
                             fmm(1.5), -fmm(0.5))

        self.text_top = -body_size/2 - fmm(0.5)
        self.text_btm = body_size/2

    def ecg_straight_outline(self, size):

        r = {'00': fmm(7),
             '0B': fmm(9),
             '1B': fmm(12),
             '2B': fmm(15),
             '3B': fmm(18),
             '4B': fmm(25),
             '5B': fmm(35)
            }[size]

        r /= 2

        notch_w = r / 4
        notch_h = notch_w * 0.5

        self.draw.NotchedCircle(0,0, r, notch_w, -notch_h)

        self.text_top = -r - notch_h
        self.text_btm = r

    def xpf_support_pins(self):
        """
        This only exists for OB
        """
        pad = PA.PadMaker(self.module).THRoundPad(fmm(1.5), fmm(1))

        array = [[fmm(-2.54), fmm(-2.54)], [fmm(2.54), fmm(-2.54)],
        [fmm(-2.54), fmm(2.54)]]

        array = PA.PadCustomArray(pad, array)
        array.SetPinNames("")
        array.AddPadsToModule(self.draw)

    def xpf_outline(self):

        r = fmm(5.5)
        self.draw.NotchedCircle(0,0, r, fmm(1.5), -fmm(0.5))

        self.text_top = -r - fmm(0.5)
        self.text_btm = r

    #####
    # Right angle pins, barrel socket
    #####

    def BuildThisFootprint(self):

        n = self.parameters['Pads']['*pins']
        size = self.parameters['Pads']['*size']
        model = self.parameters['Pads']['*model']
        contact = self.parameters['Pads']['*contact']

        if model in ["EZG", "EYG"]:
            self.ezg_support_pins(size)

            # layout the pads
            self.layout_straight_pads(n, size)

            self.ezg_outline(size)

        elif model in ["XPF"]:
            self.xpf_support_pins()

            # layout the pads
            self.layout_straight_pads(n, size)

            self.xpf_outline()

        elif model in ["ECG"]:

            self.ecg_straight_outline(size)

            # layout the pads
            if contact in ["D", "N"]:
                self.layout_straight_pads(n, size)
            else:
                self.layout_elbow_pin_pads(n, size)

        #reference and value
        text_size = fmm(1.2)  # IPC nominal
        text_offset_y = fmm(1)
        self.draw.Value(0, self.text_btm + text_size, text_size)
        self.draw.Reference(0, self.text_top - text_size, text_size)
