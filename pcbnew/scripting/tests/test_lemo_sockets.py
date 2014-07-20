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

from plugins.lemo_sockets import LemoSocket as Lemo

def test_footprints():
    """
    Perform the test
    """

    test_brd = TB.set_up_simple_test_board()

    for model in ("EZG", "XPF", "ECG"):

        for contact in Lemo.get_contact_types_for_model(model):

            for size in Lemo.get_sizes_for_model(model, contact):

                pin_counts = Lemo.get_bk_pin_counts_for_size(size, contact)

                if pin_counts:
                    for pins in pin_counts:

                        test_brd.add_footprint(Lemo(), {
                            "*pins": pins,
                            "*size": size,
                            "*model": model,
                            "*contact": contact
                        })

                    test_brd.new_row()

    test_brd.save_board_and_modules()

if __name__ == "__main__":
    test_footprints()

