/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 2017 Kicad Developers, see change_log.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef PREVIEW_PREVIEW_UTILS__H_
#define PREVIEW_PREVIEW_UTILS__H_

#include <common.h>
#include <gal/color4d.h>
#include <math/vector2d.h>

namespace KIGFX
{
class GAL;
class VIEW;

namespace PREVIEW
{

/**
 * Default alpha of "de-emphasised" features (like previously locked-in
 * lines
 */
double PreviewOverlayDeemphAlpha( bool aDeemph = true );


/**
 * Get a formatted string showing a dimension to a sane precision
 * with an optional prefix and unit suffix.
 */
wxString DimensionLabel( const wxString& prefix, double aVal,
                         EDA_UNITS_T aUnits );

/**
 * Set the GAL glyph height to a constant scaled value, so that it
 * always looks the same on screen
 *
 * @param aGal the GAL to draw on
 * @param aHeight the height of the glyph, in pixels
 */
void SetConstantGlyphHeight( KIGFX::GAL& aGal, double aHeight );

/**
 * Draw strings next to the cursor
 *
 * @param aGal the GAL to draw on
 * @param aCursorPos the position of the cursor to draw next to
 * @param aTextQuadrant a vector pointing to the quadrant to draw the
 * text in
 * @param aStrings list of strings to draw, top to bottom
 */
void DrawTextNextToCursor( KIGFX::VIEW* aView,
        const VECTOR2D& aCursorPos, const VECTOR2D& aTextQuadrant,
        const std::vector<wxString>& aStrings );

/**
 * Draw a simple line suitable for use in a preview item.
 *
 * @param aView the view to draw in
 * @param aStart line start point
 * @param aEnd line end point
 * @param aDim draw the line "dimmed" (e.g. for already-locked-in items)
 * @param aHighlightSpecialAngles highlight special angles
 */
void DrawPreviewLine( KIGFX::VIEW *aView,
        const VECTOR2I& aStart, const VECTOR2I& aEnd, bool aDim,
        bool aHighlightSpecialAngles );

/**
 * Draw an arc suitable for use in a preview item
 *
 * @param aView the view to draw in
 * @param aOrigin the arc origin
 * @param aRad the arc radius
 * @param aStartAngle the arc start angle (in radians)
 * @param aEndAngle the arc end angle (in radians)
 * @param aDim draw the arc "dimmed"
 * @param aFill fill the arc
 * @param aHighlightSpecialAngles highlight the arc when the angle is "special"
 */
void DrawPreviewArc( KIGFX::VIEW *aView,
        const VECTOR2I& aOrigin, double aRad, double aStartAngle,
        double aEndAngle, bool aDim, bool aFill, bool aHighlightSpecialAngles );

/**
 * Draw an circle suitable for use in a preview item
 *
 * @param aView the view to draw in
 * @param aOrigin the arc origin
 * @param aRad the arc radius
 * @param aDim draw the arc "dimmed"
 * @param aFill fill the arc
 */
void DrawPreviewCircle( KIGFX::VIEW *aView,
        const VECTOR2I& aOrigin, double aRad, bool aDim, bool aFill );

} // PREVIEW
} // KIGFX

#endif  // PREVIEW_PREVIEW_UTILS__H_
