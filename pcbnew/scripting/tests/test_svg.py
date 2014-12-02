import pcbnew
from pcbnew import FromMM as FMM

import os, sys
import argparse
import re
import itertools

# hack to avoid futzing with pythonpaths for the test
sys.path.append(os.path.join(os.path.dirname(os.path.realpath(__file__)), os.pardir))

import plugins.svg_path as SVG
import test_board as TB


if __name__ == "__main__":


    parser = argparse.ArgumentParser(description='Convert an SVG to a KiCad footprint.')
    parser.add_argument('footprints', metavar='F', type=str, nargs='+',
                   help='SVG files to convert')
    parser.add_argument('--scale', '-s', metavar='S', type=float, default=10,
                   help='SVG units per mm')

    args = parser.parse_args()

    filename = "/tmp/svgtest.kicad_pcb"
    mod_path = "/tmp/svgtest"

    tb = TB.TestBoard(filename, mod_path)

    for fp in args.footprints:
        tb.add_footprint(SVG.SVGPathConverter(), {
            "*filename": fp,
            "*SVG units per mm": args.scale
        })

    tb.new_row()

    tb.save_board_and_modules()

