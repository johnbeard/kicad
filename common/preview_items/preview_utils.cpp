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

#include <preview_items/preview_utils.h>
#include <gal/graphics_abstraction_layer.h>
#include <base_units.h>
#include <pcb_painter.h>
#include <view/view.h>

double KIGFX::PREVIEW::PreviewOverlayDeemphAlpha( bool aDeemph )
{
    return aDeemph ? 0.5 : 1.0;
}


static wxString getDimensionUnit( EDA_UNITS_T aUnits )
{
    switch( aUnits )
    {
    case INCHES:
        return _( "\"" );

    case MILLIMETRES:
        return _( "mm" );

    case DEGREES:
        return _( "deg" );

    case UNSCALED_UNITS:
        break;
    // no default: handle all cases
    }

    return wxEmptyString;
}


static wxString formatPreviewDimension( double aVal, EDA_UNITS_T aUnits )
{
    int precision = 4;

    // show a sane precision for the preview, which doesn't need to
    // be accurate down to the nanometre
    switch( aUnits )
    {
    case MILLIMETRES:
        precision = 2;  // 10um
        break;
    case INCHES:
        precision = 4;  // 1mil
        break;
    case DEGREES:
        precision = 1;  // 0.1deg (limit of formats anyway)
        break;
    case UNSCALED_UNITS:
        break;
    }

    const wxString fmtStr = wxString::Format( "%%.%df", precision );

    wxString str = wxString::Format( fmtStr, To_User_Unit( aUnits, aVal ) );

    const wxString symbol = getDimensionUnit( aUnits );

    if( symbol.size() )
        str << " " << symbol;

    return str;
}


wxString KIGFX::PREVIEW::DimensionLabel( const wxString& prefix,
            double aVal, EDA_UNITS_T aUnits )
{
    wxString str;

    if( prefix.size() )
        str << prefix << ": ";

    str << formatPreviewDimension( aVal, aUnits );
    return str;
}


void KIGFX::PREVIEW::SetConstantGlyphHeight( KIGFX::GAL& aGal, double aHeight )
{
    aHeight /= aGal.GetWorldScale();

    auto glyphSize = aGal.GetGlyphSize();
    glyphSize = glyphSize * ( aHeight / glyphSize.y );
    aGal.SetGlyphSize( glyphSize );
}


void KIGFX::PREVIEW::DrawTextNextToCursor( KIGFX::VIEW* aView,
        const VECTOR2D& aCursorPos, const VECTOR2D& aTextQuadrant,
        const std::vector<wxString>& aStrings )
{
    auto gal = aView->GetGAL();
    auto glyphSize = gal->GetGlyphSize();
    auto rs = static_cast<KIGFX::PCB_RENDER_SETTINGS*>( aView->GetPainter()->GetSettings() );

    const auto lineSpace = glyphSize.y * 0.2;
    auto linePitch = glyphSize.y + lineSpace;

    // radius string goes on the right of the cursor centre line
    // with a small horizontal offset (enough to keep clear of a
    // system cursor if present)
    auto textPos = aCursorPos;

    // if the text goes above the cursor, shift it up
    if( aTextQuadrant.y > 0 )
    {
        textPos.y -= linePitch * ( aStrings.size() + 1 );
    }

    if( aTextQuadrant.x < 0 )
    {
        gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_LEFT );
        textPos.x += 15.0 / gal->GetWorldScale();
    }
    else
    {
        gal->SetHorizontalJustify( GR_TEXT_HJUSTIFY_RIGHT );
        textPos.x -= 15.0 / gal->GetWorldScale();
    }

    gal->SetStrokeColor( rs->GetLayerColor( LAYER_AUX_ITEMS ).WithAlpha(
                            PreviewOverlayDeemphAlpha( true ) ) );
    gal->SetIsFill( false );

    // write strings top-to-bottom
    for( const auto& str : aStrings )
    {
        textPos.y += linePitch;
        gal->BitmapText( str, textPos, 0.0 );
    }
}


static const double ANGLE_EPSILON = 1e-9;


static bool angleIsSpecial( double aRadians )
{
    return std::fabs( std::remainder( aRadians, M_PI_4 ) ) < ANGLE_EPSILON;
}


/**
 * Get the colour for a "special" item, for example a line that's
 * snapped to a special angle.
 *
 * @param aRs the current render settings
 * @return the colour for the special item
 */
static COLOR4D getSpecialItemColor( KIGFX::PCB_RENDER_SETTINGS& aRs )
{
    static const COLOR4D greenDarkBg { 0.5, 1.0, 0.5, 1.0 };
    static const COLOR4D greenLightBg { 0.0, 0.7, 0.0, 1.0 };

    return aRs.IsBackgroundDark() ? greenDarkBg : greenLightBg;
}


void KIGFX::PREVIEW::DrawPreviewLine( KIGFX::VIEW *aView,
        const VECTOR2I& aStart, const VECTOR2I& aEnd, bool aDim,
        bool aHighlightSpecialAngles )
{
    auto gal = aView->GetGAL();
    auto rs = static_cast<KIGFX::PCB_RENDER_SETTINGS*>( aView->GetPainter()->GetSettings() );

    const auto vec = aEnd - aStart;
    COLOR4D strokeColor = rs->GetLayerColor( LAYER_AUX_ITEMS );

    if( aHighlightSpecialAngles && angleIsSpecial( vec.Angle() ) )
        strokeColor = getSpecialItemColor( *rs );

    gal->SetStrokeColor( strokeColor.WithAlpha( PreviewOverlayDeemphAlpha( aDim ) ) );
    gal->DrawLine( aStart, aEnd );
}


void KIGFX::PREVIEW::DrawPreviewArc( KIGFX::VIEW *aView,
        const VECTOR2I& aOrigin, double aRad, double aStartAngle,
        double aEndAngle, bool aDim, bool aFill, bool aHighlightSpecialAngles )
{
    auto gal = aView->GetGAL();
    auto rs = static_cast<KIGFX::PCB_RENDER_SETTINGS*>( aView->GetPainter()->GetSettings() );

    auto color = rs->GetLayerColor( LAYER_AUX_ITEMS );

    if( aHighlightSpecialAngles && angleIsSpecial( aStartAngle - aEndAngle ) )
        color = getSpecialItemColor( *rs );

    gal->SetIsStroke( true );
    gal->SetIsFill( aFill );
    gal->SetStrokeColor( color.WithAlpha( PreviewOverlayDeemphAlpha( aDim ) ) );

    if( aFill )
        gal->SetFillColor( color.WithAlpha( 0.2 ) );

    // draw the angle reference arc
    gal->DrawArc( aOrigin, aRad, -aStartAngle, -aEndAngle );
}


void KIGFX::PREVIEW::DrawPreviewCircle( KIGFX::VIEW *aView,
        const VECTOR2I& aOrigin, double aRad, bool aDim, bool aFill )
{
    auto gal = aView->GetGAL();
    auto rs = static_cast<KIGFX::PCB_RENDER_SETTINGS*>( aView->GetPainter()->GetSettings() );

    auto color = rs->GetLayerColor( LAYER_AUX_ITEMS );

    gal->SetStrokeColor( color.WithAlpha( PreviewOverlayDeemphAlpha( aDim ) ) );
    gal->SetIsStroke( true );

    gal->SetIsFill( aFill );

    if( aFill )
        gal->SetFillColor( color.WithAlpha( 0.2 ) );

    gal->DrawCircle( aOrigin, aRad );
}