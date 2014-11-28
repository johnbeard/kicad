/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 John Beard, john.j.beard@gmail.com
 * Copyright (C) 1992-2014 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <wxPcbStruct.h>
#include <base_units.h>

#include <module_editor_frame.h>

#include "dialog_move_exact.h"

// initialise statics
DIALOG_MOVE_EXACT::MOVE_EXACT_OPTIONS DIALOG_MOVE_EXACT::m_options;

DIALOG_MOVE_EXACT::DIALOG_MOVE_EXACT( PCB_BASE_FRAME* aParent,
                                      wxPoint& translation, double& rotation ):
    DIALOG_MOVE_EXACT_BASE( aParent ),
    m_translation( translation ),
    m_rotation( rotation )
{
    // set the unit labels
    m_xUnit->SetLabelText( GetAbbreviatedUnitsLabel( g_UserUnit ) );
    m_yUnit->SetLabelText( GetAbbreviatedUnitsLabel( g_UserUnit ) );
    // rotation is always degrees
    m_rotUnit->SetLabelText( _( "deg" ) );

    // tabbing goes through the entries in sequence
    m_yEntry->MoveAfterInTabOrder( m_xEntry );
    m_rotEntry->MoveAfterInTabOrder( m_yEntry );

    // and set up the entries according to the saved options
    m_polarCoords->SetValue( m_options.polarCoords );
    m_xEntry->SetValue( wxString::FromDouble( m_options.entry1 ) );
    m_yEntry->SetValue( wxString::FromDouble( m_options.entry2 ) );
    m_rotEntry->SetValue( wxString::FromDouble( m_options.entryRotation ) );
}

DIALOG_MOVE_EXACT::~DIALOG_MOVE_EXACT()
{
}

/*!
 * Convert a given Cartesian point into a polar representation.
 *
 * Linear units are not considered, the answer is in the same units as given
 * Angle is returned in degrees
 */
void DIALOG_MOVE_EXACT::ToPolarDeg( double x, double y, double& r, double& q )
{
    // convert to polar coordinates
    r = hypot ( x, y );

    q = ( r != 0) ? RAD2DEG( atan2( y, x ) ) : 0;
}

bool DIALOG_MOVE_EXACT::GetTranslationInIU ( wxPoint& val, bool polar )
{
    // entries in user units
    double ent1, ent2;
    bool ok = m_xEntry->GetValue().ToDouble( &ent1 );
    ok = ok && m_yEntry->GetValue().ToDouble( &ent2 );

    if( !ok )
        return false;

    if( polar )
    {
        val.x = From_User_Unit( g_UserUnit, ent1 * cos( DEG2RAD( ent2 ) ) );
        val.y = From_User_Unit( g_UserUnit, ent1 * sin( DEG2RAD( ent2 ) ) );
    }
    else
    {
        // direct read
        val.x = From_User_Unit( g_UserUnit, ent1 );
        val.y = From_User_Unit( g_UserUnit, ent2 );
    }

    return true;
}

void DIALOG_MOVE_EXACT::OnPolarChanged( wxCommandEvent& event )
{
    bool newPolar = m_polarCoords->IsChecked();
    wxPoint val;

    // get the value as previously stored
    bool ok = GetTranslationInIU( val, !newPolar );

    // invalid entries - bail out
    if( !ok )
        return;

    if( m_polarCoords->IsChecked() )
    {
        // convert to polar coordinates
        double r, q;
        ToPolarDeg( val.x, val.y, r, q);

        PutValueInLocalUnits( *m_xEntry, round( r / 10.0) * 10 );
        m_xLabel->SetLabelText( wxT( "r:" ) );

        m_yEntry->SetValue( wxString::FromDouble( q ) );
        m_yLabel->SetLabelText( wxT( "\u03b8:" ) ); // theta

        m_yUnit->SetLabelText( _( "deg" ) );
    }
    else
    {
        // vector is already in Cartesian, so just render out

        // note - round off the last decimal place (10nm) to prevent
        // rounding causing errors when round-tripping
        PutValueInLocalUnits( *m_xEntry, round( val.x / 10.0) * 10 );
        m_xLabel->SetLabelText( wxT( "x:" ) );

        PutValueInLocalUnits( *m_yEntry, round( val.y / 10.0) * 10 );
        m_yLabel->SetLabelText( wxT( "y:" ) );

        m_yUnit->SetLabelText( GetAbbreviatedUnitsLabel( g_UserUnit ) );
    }
}

void DIALOG_MOVE_EXACT::OnClear( wxCommandEvent& event )
{
    wxObject* obj = event.GetEventObject();
    wxTextCtrl* entry = NULL;

    if( obj == m_clearX )
    {
        entry = m_xEntry;
    }
    else if( obj == m_clearY )
    {
        entry = m_yEntry;
    }
    else if( obj == m_clearRot )
    {
        entry = m_rotEntry;
    }

    if( entry )
        entry->SetValue( "0" );
}

void DIALOG_MOVE_EXACT::OnCancelClick( wxCommandEvent& event )
{
    EndModal( MOVE_ABORT );
}
void DIALOG_MOVE_EXACT::OnOkClick( wxCommandEvent& event )
{
    bool ok = m_rotEntry->GetValue().ToDouble( &m_rotation );
    m_rotation *= 10.0f;

    // for the output, we only deliver a Cartesian vector
    ok = ok && GetTranslationInIU( m_translation, m_polarCoords->IsChecked() );

    if( ok )
    {
        // save the settings
        m_options.polarCoords = m_polarCoords->GetValue();
        m_xEntry->GetValue().ToDouble( &m_options.entry1 );
        m_yEntry->GetValue().ToDouble( &m_options.entry2 );
        m_rotEntry->GetValue().ToDouble( &m_options.entryRotation );

        EndModal( MOVE_OK );
    }
}

