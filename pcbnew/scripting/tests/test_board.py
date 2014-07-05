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
Simple helpers for automating generation of footprints with
footprint wizards, either for testing or for producing footprints
for libraries
"""

import pcbnew
from pcbnew import FromMM as fmm

import os
import sys
import re
import argparse
import tempfile

# hack to avoid futzing with pythonpaths for the test
sys.path.append(
    os.path.join(
        os.path.dirname(os.path.realpath(__file__)), os.pardir))


class TestBoard(object):
    """
    Very basic and hacky test harness for stuffing modules into a PCB
    file and then ripping them out into a directory
    """

    def __init__(self, output_board_file=None, output_mod_dir=None):
        """
        Set the board up, along with default geometries
        """

        # defualt geometry
        self.margin = fmm(30)

        self.pos = [0, 0]
        self.sep = [fmm(5), fmm(5)]

        # this is a hack until we can get a real bounding box
        self.cell = [fmm(20), fmm(20)]

        self.board = pcbnew.BOARD()

        page = self.board.GetPageSettings()

        self.lim = [page.GetWidthIU() - (self.margin),
                    page.GetHeightIU() - (self.margin)]

        self.filename = output_board_file
        self.moddir = output_mod_dir

    def set_grid_geometry(self, cell, sep):
        """
        Set a new layout geometry (cell sizes and separations, etc)

        This will hopefully not be needed when we can work out the
        actual bounding box
        """
        self.cell = cell
        self.sep = sep

    def new_row(self):
        """
        Start a new row of modules
        """
        self.pos = [0, self.pos[1] + self.cell[1] + self.sep[1]]

    def add_module(self, mod):
        """
        Add a module to the board at the current position,
        wrapping to the next line if we ran out of page
        """

        if self.pos[0] > self.lim[0] - self.cell[0]:
            self.pos = [0, self.pos[1] + self.cell[1]]

        mod.SetPosition(pcbnew.wxPoint(
            self.margin + self.pos[0], self.margin + self.pos[1]))

        # this segfaults :-(
        # print mod.GetFootprintRect()
        # self.x += mod.GetBoundingBox().GetWidth() + self.xsep

        self.board.Add(mod)

        self.pos[0] = self.pos[0] + self.cell[0] + self.sep[0]

    def add_footprint(self, footprint, params):
        """
        Create a module from the given wizard and parameters
        and then place it onto the board
        """

        for page in range(footprint.GetNumParameterPages()):

            param_list = footprint.GetParameterNames(page)
            val_list = footprint.GetParameterValues(page)

            for key, val in params.iteritems():
                if key in param_list:
                    val_list[param_list.index(key)] = val

            footprint.SetParameterValues(page, val_list)

        module = footprint.GetModule()

        self.add_module(module)

    def save_board_and_modules(self):
        """
        Save the board to the given file
        """
        if not self.filename:
            return

        self.board.Save(self.filename, pcbnew.IO_MGR.KICAD)
        self.rip_out_and_save_modules()

    def rip_out_and_save_modules(self):
        """
        Hack to rip out modules from board.
        Be very careful trusting this to treat modules nicely!
        Surely there must be a better way to get a module into a file
        without resorting to this?
        """
        if not self.filename or not self.moddir:
            return

        brd = open(self.filename, 'r')

        if not os.path.isdir(self.moddir):
            os.makedirs(self.moddir)

        mod = ''
        in_mod = False

        for line in brd:
            if line.startswith("  (module"):
                in_mod = True

                # remove unwanted elements
                line = re.sub(r"\(t(stamp|edit).*?\)", "", line)

                ref = line.split()[1]

            if in_mod:
                if not line.startswith("    (at "):
                    mod += line[2:].rstrip() + "\n"

                if line.startswith("  )"):
                    in_mod = False

                    mod_file = os.path.join(
                        self.moddir, "%s.%s" % (ref, "kicad_mod"))

                    ofile = open(mod_file, 'w')
                    ofile.write(mod)
                    ofile.close()

                    mod = ''


def set_up_simple_test_board():
    """
    Very simple board setup for basic "place modules onto PCB"-type tests
    """

    default_pcb_file = os.path.join(tempfile.gettempdir(), 'test.kicad_pcb')
    default_mod_dir = os.path.join(tempfile.gettempdir(), 'kicad_test_mods')

    parser = argparse.ArgumentParser(
        description='Convert an SVG to a KiCad footprint.')

    parser.add_argument(
        '--board', '-b', metavar='B', type=str,
        default=default_pcb_file,
        help='Board file to output'
    )
    parser.add_argument(
        '--moddir', '-d', metavar='D', type=str,
        default=default_mod_dir,
        help='Directory to output the modules to'
    )

    args = parser.parse_args()

    print "Test started:\n"
    print "\tOutputting file to: %s" % args.board
    print "\tOutputting modules to: %s" % args.moddir
    print "\n"

    test_brd = TestBoard(args.board, args.moddir)

    return test_brd
