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

import test_board as TB

import plugins.molex_picoblade as MPB
import plugins.harwin_m40 as HM40

def test_footprints():
    """
    Perform the test
    """

    test_brd = TB.set_up_simple_test_board()

    for n in range(2,16):
        test_brd.add_footprint(MPB.MolexSmdHeader(), {
            "*n": n,
            "*ra": False,
            "hand soldering ext": 0,
        })

    test_brd.new_row()

    for n in range(2,16):
        test_brd.add_footprint(MPB.MolexSmdHeader(), {
            "*n": n,
            "*ra": True,
            "hand soldering ext": 0,
        })

    test_brd.new_row()

    for n in range(2,16):
        test_brd.add_footprint(MPB.MolexThtVertHeader(), {
            "*n": n,
            "hand soldering ext": 0,
        })

    test_brd.new_row()

    for n in range(2,16):
        test_brd.add_footprint(MPB.MolexThtRaHeader(), {
            "*n": n,
            "hand soldering ext": 0,
        })

    test_brd.new_row()

    for n in range(2,6):
        test_brd.add_footprint(HM40.HarwinM40SMDHeader(), {
            "*n": n,
            "*ra": False,
            "hand soldering ext": 0,
        })

    test_brd.new_row()

    for n in range(2,6):
        test_brd.add_footprint(HM40.HarwinM40SMDHeader(), {
            "*n": n,
            "*ra": True,
            "hand soldering ext": 0,
        })

    test_brd.new_row()

    test_brd.save_board_and_modules()

if __name__ == "__main__":
    test_footprints()

