from __future__ import division

import pcbnew
from pcbnew import FromMM as fmm
import math

import HelpfulFootprintWizardPlugin as HFPW

from lxml.etree import fromstring
import svg.path
from bezier_converter import biarc

ns = "{http://www.w3.org/2000/svg}"

class SVGPathConverter(HFPW.HelpfulFootprintWizardPlugin):

    def GetName(self):
        return "SVG Path"

    def GetDescription(self):
        return "Render an SVG's paths into a module"

    def GenerateParameterList(self):

        self.AddParam("SVG", "filename", self.uNatural, "")
        self.AddParam("SVG", "biarc tolerance", self.uNatural, "0.1")
        self.AddParam("SVG", "SVG units per mm", self.uNatural, "20")

    def GetValue(self):
        return "SVG"

    def GetReferencePrefix(self):
        return ""

    def extractSvgStyle(self, e):

        style = {}

        if 'style' not in e.attrib:
            return style

        styles = [x.strip() for x in e.attrib['style'].split(';')]

        for s in styles:
            k, v = s.split(':', 1)

            style[k] = v

        return style

    def SetStrokeWidth(self, e, scale, transform):
        style = self.extractSvgStyle(e)

        if 'stroke-width' in style:
            w = float(style['stroke-width'])
        else:
            w = 0.2 #default

        w = int(round(w * transform[0] * scale)) #assume isotropic scaling, there's not much we can do if not, that's up to the editor!

        self.draw.SetWidth(w)

    def BuildThisFootprint(self):

        f = open(self.parameters["SVG"]["*filename"])
        root = fromstring(f.read())

        self.segments = 0
        self.lines = 0
        self.arcs = 0

        # transform to internal units
        scaleToMM = fmm(1)/self.parameters["SVG"]["*SVG units per mm"]
        self.draw.TransformScaleOrigin(scaleToMM)

        self.module.Reference().SetVisible(False)
        self.module.Value().SetVisible(False)

        docWidth = float(root.attrib['width'])
        docHeight = float(root.attrib['height'])

        # place the drawing page around the origin
        self.draw.TransformTranslate(-docWidth/2, -docHeight/2)

        for e in root.findall('.//%spath' % ns):
            transform = self.collectTransforms(e)

            d = svg.path.parse_path(e.attrib['d'])

            self.SetStrokeWidth(e, scaleToMM, transform)

            self.addPathToModule(d, transform)

        for e in root.findall('.//%srect' % ns):
            transform = self.collectTransforms(e)

            x,y = float(e.attrib['x']), float(e.attrib['y'])
            w,h = float(e.attrib['width']), float(e.attrib['height'])

            self.draw.PushTransform(transform)
            self.draw.TransformTranslate(w/2, h/2) #svg rects are referenced from the corner, not centre

            self.SetStrokeWidth(e, scaleToMM, transform)

            self.draw.Box(x,y,w,h)

            self.draw.PopTransform(2)

            self.lines += 4
            self.segments += 4

        self.draw.PopTransform(2) #pop the scale xfrm

        #report
        print "SVG had %d segments" % self.segments
        print "%d arcs, %d lines, %d total segments" % (self.arcs, self.lines, self.arcs + self.lines)

    def collectTransforms(self, e):
        transforms = []

        if 'transform' in e.attrib:
            transforms.append(e.attrib['transform'])

        for a in e.xpath('ancestor::*' ):

            if 'transform' in a.attrib:
                transforms.append(a.attrib['transform'])

        transforms.reverse()

        matrix = self.convertTransformsToMatrix(transforms)

        return matrix

    def convertTransformsToMatrix(self, ts):

        m = [] # list of matrices
        for t in ts:

            s = t.index('(')
            e = t.index(')')

            c = [float(x) for x in t[s+1:e].split(',')]
            trans = t[:s]

            if trans == 'translate':
                matrix = [1, 0, c[0], 0, 1, c[1]]
            elif trans == 'scale':
                matrix = [c[0], 0, 0, 0, c[1], 0]
            elif trans == 'matrix':
                matrix = [c[0], c[2], c[4], c[1], c[3], c[5]]
            else:
                raise KeyError("Unknown transform: %s" % t)

            m.append(matrix)

        combined = self.draw._ComposeMatricesWithIdentity(m)

        return combined

    def addPathToModule(self, d, t):

        self.draw.PushTransform(t)

        self.segments += len(d)

        for seg in d:

            if isinstance(seg, svg.path.CubicBezier):

                p1 = (seg.start.real, seg.start.imag)
                p2 = (seg.control1.real, seg.control1.imag)
                p3 = (seg.control2.real, seg.control2.imag)
                p4 = (seg.end.real, seg.end.imag)

                bez = [p1, p2, p3, p4]

                self.approximateAndDrawBezier(bez)

            elif isinstance(seg, svg.path.Line):
                self.lines += 1
                self.draw.Line(seg.start.real, seg.start.imag, seg.end.real, seg.end.imag)

            elif isinstance(seg, svg.path.Arc):
                self.arcs += 1

                start = (seg.start.real, seg.start.imag)
                end = (seg.end.real, seg.end.imag)

            else:
                print "Skipping unknown segment: %s" % seg
                continue;

        self.draw.PopTransform()

    def approximateAndDrawBezier(self, bez):

        biarcs = biarc(bez)

        for arc in biarcs:

            if arc[1] == 'arc':

                self.arcs += 1

                angle = arc[3] * 1800 / math.pi

                self.draw.Arc(arc[2][0], arc[2][1], arc[0][0], arc[0][1], angle)

            elif arc[1] == 'line':

                self.lines += 1

                self.draw.Line(arc[0][0], arc[0][1], arc[2][0], arc[2][1])

            else:
                raise ValueError("Unknown arc type: %s" % arc)

