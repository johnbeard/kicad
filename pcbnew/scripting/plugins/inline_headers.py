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
from math import copysign
import pcbnew
from pcbnew import FromMM as fmm

import ConnectorWizard as CW
import PadArray as PA

class SMDInlineHeaderWithWings(CW.ConnectorWizard):

    def IsSMD(self):
        return True

    def HasSupportWings(self):
        return False

    def BuildThisFootprint(self):

        prm = self.GetComponentParams();

        row_length = (self.N() - 1) * abs(prm['d'])

        pad_h = prm['B'] + self.HandSolderingExt()

        pad = PA.PadMaker(self.module).SMDPad(pad_h, prm['A'])

        off = -prm['D'] / 2 - prm['E'] - pad_h / 2;

        pos = pcbnew.wxPoint(0, off)
        array = PA.PadLineArray(pad, self.N(), prm['d'], False, pos)
        array.AddPadsToModule(self.draw)

        # supports
        support_pitch = row_length + 2 * prm['F'] + prm['C']

        pos = pcbnew.wxPoint(0, 0)
        pad = PA.PadMaker(self.module).SMDPad(prm['D'], prm['C'])
        array = PA.PadLineArray(pad, 2, support_pitch, False, pos)
        array.SetPinNames("")
        array.AddPadsToModule(self.draw)

        # reverse graphics for pin 1 on right
        if prm['d'] < 0:
            self.draw.TransformFlipOrigin(self.draw.flipX)

        # silk line
        self.draw.SetWidth(fmm(0.2))

        pad_clearance = self.draw.GetWidth() * 1.5
        inner_vertical_x = -row_length/2 - prm['A']/2 - pad_clearance
        outer_vertical_x = -support_pitch/2 - prm['C']/2 - pad_clearance

        top_y = -prm['D'] / 2 - prm['E'] - pad_h
        half_pad_h = prm['D'] / 2

        if prm['H'] < 0:
                silk_bottom_y = half_pad_h + pad_clearance
        else:
                silk_bottom_y = half_pad_h + prm['H']

        silk_shoulder_top_y = -half_pad_h - prm['E']
        silk_shoulder_x = -row_length/2 - prm['G']
        silk_shoulder_btm_y = -half_pad_h - pad_clearance

        main_body_x = row_length/2 + prm['G']
        main_body_top = -prm['D']/2 - prm['E']
        main_body_btm = prm['D']/2 + prm['H']

        pts = [ [inner_vertical_x,            top_y],
                [inner_vertical_x,            silk_shoulder_top_y],
                [silk_shoulder_x,             silk_shoulder_top_y],
                [silk_shoulder_x,             silk_shoulder_btm_y],
                [outer_vertical_x,            silk_shoulder_btm_y],
                [outer_vertical_x,            silk_bottom_y]
            ]

        # dent inwards if the body does not extend past the support lands
        if prm['H'] < 0:
            dent_x = -support_pitch/2 + prm['C']/2 + pad_clearance

            pts.extend(
                  [ [dent_x,    silk_bottom_y],
                    [dent_x,    main_body_btm],
                    [-dent_x,   main_body_btm],
                    [-dent_x,   silk_bottom_y],
                  ])

        pts.extend(
              [ [-outer_vertical_x, silk_bottom_y],
                [-outer_vertical_x, silk_shoulder_btm_y]
              ])

        self.draw.Polyline(pts)

        # marker arrow
        self.draw.MarkerArrow(-row_length / 2, top_y - fmm(0.5),
                              direction=self.draw.dirS)

        # assembly layer
        self.draw.SetLayer(pcbnew.F_Adhes)

        pts = [ [-main_body_x,    main_body_top],
                [-main_body_x,    main_body_btm],
                [ main_body_x,    main_body_btm],
                [ main_body_x,    main_body_top],
                [-main_body_x,    main_body_top]]

        self.draw.Polyline(pts)

        # Draw support wings on the assembly layer
        if self.HasSupportWings():
                wing_outer_x = main_body_x + prm['C']

                pts = [ [ main_body_x,    -prm['D']/2],
                        [ wing_outer_x,   -prm['D']/2],
                        [ wing_outer_x,    prm['D']/2],
                        [ main_body_x,     prm['D']/2] ]

                self.draw.Polyline(pts, mirrorX=0)

        # assembly layer - pin 1
        pin1_x = -row_length/2

        #make sure we can fit horizontally
        pin1_mark_size = min(abs(abs(main_body_x) - abs(pin1_x)), fmm(3))

        # and vertically
        pin1_mark_size = min(
            pin1_mark_size,
            abs(0.5 * (main_body_top - main_body_btm))
        )

        print pin1_mark_size

        pts = [ [pin1_x - pin1_mark_size,  main_body_top],
                [pin1_x,                   main_body_top + pin1_mark_size],
                [pin1_x + pin1_mark_size,  main_body_top]]

        self.draw.Polyline(pts)

        if prm['d'] < 0:
            self.draw.PopTransform()

        size = self.GetTextSize()

        #texts
        textY = prm['D']/2 + prm['E'] + size
        self.draw.Reference(row_length/2 + prm['C'] + fmm(1), -textY, size)
        self.draw.Value(0, textY, size)

        self.SetModuleDescription()


class ThtRaHeaderShrouded(CW.ConnectorWizard):

    def IsRightAngled(self):
        return True

    def BuildThisFootprint(self):
        prm = self.GetComponentParams()

        row_length = (self.N() - 1) * prm['d']

        pad = PA.PadMaker(self.module).THPad(prm['B'], prm['B'], drill=prm['b'])
        firstPad = PA.PadMaker(self.module).THPad(prm['B'], prm['B'], drill=prm['b'], shape=pcbnew.PAD_RECT)

        pos = pcbnew.wxPoint(0, 0)
        array = PA.PadLineArray(pad, self.N(), prm['d'], False, pos)
        array.SetFirstPadType(firstPad)
        array.AddPadsToModule(self.draw)

        # silk screen line
        topY = -prm['E1']
        bottomY = prm['E2']
        boxX = row_length/2 + prm['D1']

        pts = [ [-boxX,  -prm['B']],
                [-boxX,  prm['E2']],
                [boxX,   prm['E2']],
                [boxX,   prm['B']]
              ]

        self.draw.Polyline(pts)

        self.DrawPin1MarkerAndTexts(prm['E1'], prm['E2'], -row_length/2)

        self.draw.SetLayer(pcbnew.F_Adhes)

        bodyH = bottomY - topY
        setback = min(fmm(2), bodyH/2)
        self.draw.BoxWithDiagonalAtCorner(0, (bottomY + topY)/2, boxX*2,
                                            bodyH, setback)

        #origin to pin 1
        self.module.SetPosition(pcbnew.wxPoint(-row_length/2, fmm(4)))

        self.SetModuleDescription()

        self.module.MoveAnchorPosition(pcbnew.wxPoint(row_length/2, 0))

class ThtVerticalHeader(CW.ConnectorWizard):

    def BuildThisFootprint(self):
        prm = self.GetComponentParams()

        row_length = (self.N() - 1) * prm['d']

        pad = PA.PadMaker(self.module).THPad(prm['B'], prm['B'], drill=prm['b'])
        firstPad = PA.PadMaker(self.module).THPad(prm['B'], prm['B'], drill=prm['b'], shape=pcbnew.PAD_RECT)

        pos = pcbnew.wxPoint(0, 0)
        array = PA.PadLineArray(pad, self.N(), prm['d'], False, pos)
        array.SetFirstPadType(firstPad)
        array.AddPadsToModule(self.draw)

        # silk screen box
        topY = -prm['E1']
        bottomY = prm['E2']
        boxW = row_length + 2 * prm['D1']
        bodyH = bottomY - topY
        bodyYMid = (topY + bottomY) / 2

        self.draw.BoxWithOpenCorner(0, bodyYMid, boxW, bodyH, flip=self.draw.flipY)

        self.DrawPin1MarkerAndTexts(prm['E1'], prm['E2'], -row_length / 2)

        self.draw.SetLayer(pcbnew.F_Adhes)

        self.draw.BoxWithDiagonalAtCorner(0, bodyYMid, boxW, bodyH, flip=self.draw.flipY)

        #origin to pin 1
        self.module.MoveAnchorPosition(pcbnew.wxPoint(row_length/2, 0))

        self.SetModuleDescription()
