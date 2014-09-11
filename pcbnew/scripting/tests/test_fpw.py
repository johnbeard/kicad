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
Test BGA, QFP and SDIP footprints
"""

from pcbnew import FromMM as fmm

import test_board as TB

import plugins.bga_wizard as BGA
import plugins.qfp_wizard as QFP
import plugins.sdip_wizard as SDIP

def test_footprints():
    """
    Perform the test
    """

    test_brd = TB.set_up_simple_test_board()

    test_brd.add_footprint(BGA.BGAWizard(), {
        "pad pitch": fmm(1),
        "pad size": fmm(0.5),
        "*row count": 5,
        "*column count": 5,
        "outline x margin": fmm(1),
        "outline y margin": fmm(1)
    })

    for oval in [True, False]:
        test_brd.add_footprint(QFP.QFPWizard(), {
            "*n": 64,
            "pad pitch": fmm(0.5),
            "pad width": fmm(0.25),
            "pad length": fmm(1.5),
            "vertical pitch": fmm(15),
            "horizontal pitch": fmm(15),
            "*oval": oval,
            "package width": fmm(14),
            "package height": fmm(14)
        })

    test_brd.new_row()

    for rows in [1, 2]:
        for ssi in [True, False]:
            test_brd.add_footprint(SDIP.SDIPWizard(), {
                "*n": 6,
                "*silk screen inside": ssi,
                "*row count": rows,
                "pad pitch": fmm(2.54),
                "pad width": fmm(1.5),
                "pad length": fmm(3.8),
                "row spacing": fmm(7.62),
                "drill size": fmm(1),
                "outline x margin": fmm(0.5),
                "outline y margin": fmm(1)
            })

    test_brd.save_board_and_modules()

if __name__ == "__main__":
    test_footprints()
