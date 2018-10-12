/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 Kicad Developers, see AUTHORS.txt for contributors.
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

#include <preview_items/circle_assistant.h>

#include <preview_items/preview_utils.h>
#include <gal/graphics_abstraction_layer.h>
#include <view/view.h>
#include <pcb_painter.h>

#include <common.h>
#include <base_units.h>

using namespace KIGFX::PREVIEW;

CIRCLE_ASSISTANT::CIRCLE_ASSISTANT( const TWO_POINT_GEOMETRY_MANAGER_NG& aManager, EDA_UNITS_T aUnits ) :
    EDA_ITEM( NOT_USED ),
    m_constructMan( aManager ),
    m_units( aUnits )
{
}


const BOX2I CIRCLE_ASSISTANT::ViewBBox() const
{
    BOX2I tmp;

    // no bounding box when no graphic shown
    if( m_constructMan.IsReset() )
        return tmp;

    // compute
    return tmp;
}


void CIRCLE_ASSISTANT::ViewDraw( int aLayer, KIGFX::VIEW* aView ) const
{
    auto& gal = *aView->GetGAL();
    auto rs = static_cast<KIGFX::PCB_RENDER_SETTINGS*>( aView->GetPainter()->GetSettings() );

    // not in a position to draw anything
    if( m_constructMan.IsReset() )
        return;

    return;
}