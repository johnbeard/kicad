/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2009 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 1992-2011 KiCad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file gerbview_config.cpp
 * @brief GerbView configuration.
*/

#include <fctsys.h>
#include <id.h>
#include <common.h>
#include <class_drawpanel.h>
#include <config_params.h>
#include <gerbview_frame.h>
#include <hotkeys.h>
#include <widgets/paged_dialog.h>
#include <dialogs/panel_gerbview_settings.h>
#include <dialogs/panel_gerbview_display_options.h>


EDA_BASE_FRAME::HOTKEY_PREFS_CONFIG GERBVIEW_FRAME::getHotkeyPrefsConfigs()
{
    return {
        GerbviewHokeysDescr,
        GerbviewHokeysDescr,
        _( "gerbview" )
    };
}


void GERBVIEW_FRAME::Process_Config( wxCommandEvent& event )
{
    int      id = event.GetId();

    switch( id )
    {
    case wxID_PREFERENCES:
        ShowPreferences();
        break;

    case ID_PREFERENCES_HOTKEY_SHOW_CURRENT_LIST:
        // Display current hotkey list for GerbView.
        DisplayHotkeyList();
        break;

    default:
        wxMessageBox( wxT( "GERBVIEW_FRAME::Process_Config error" ) );
        break;
    }
}


void GERBVIEW_FRAME::InstallPreferences( PAGED_DIALOG* aParent )
{
    wxTreebook* book = aParent->GetTreebook();

    book->AddPage( new PANEL_GERBVIEW_SETTINGS( this, book ), _( "Gerbview" ) );
    book->AddSubPage( new PANEL_GERBVIEW_DISPLAY_OPTIONS( this, book ), _( "Display Options" ) );
}


PARAM_CFG_ARRAY& GERBVIEW_FRAME::GetConfigurationSettings()
{
    if( !m_configSettings.empty() )
        return m_configSettings;

    m_configSettings.push_back( new PARAM_CFG_INT( true, wxT( "DrawModeOption" ),
                                                   &m_displayMode, 2, 0, 2 ) );
    m_configSettings.push_back( new PARAM_CFG_SETCOLOR(
            true, wxT( "DCodeColorEx" ),
            &g_ColorsSettings.m_LayersColors[LAYER_DCODES], WHITE ) );
    m_configSettings.push_back( new PARAM_CFG_SETCOLOR(
            true, wxT( "NegativeObjectsColorEx" ),
            &g_ColorsSettings.m_LayersColors[LAYER_NEGATIVE_OBJECTS], DARKGRAY ) );
    m_configSettings.push_back( new PARAM_CFG_SETCOLOR(
            true, wxT( "GridColorEx" ),
            &g_ColorsSettings.m_LayersColors[LAYER_GERBVIEW_GRID], DARKGRAY ) );
    m_configSettings.push_back( new PARAM_CFG_SETCOLOR(
            true, wxT( "WorksheetColorEx" ),
            &g_ColorsSettings.m_LayersColors[ LAYER_WORKSHEET], DARKRED ) );
    m_configSettings.push_back( new PARAM_CFG_SETCOLOR(
            true, wxT( "BackgroundColorEx" ),
            &g_ColorsSettings.m_LayersColors[LAYER_PCB_BACKGROUND], BLACK ) );
    m_configSettings.push_back( new PARAM_CFG_BOOL(
            true, wxT( "DisplayPolarCoordinates" ),
            &m_DisplayOptions.m_DisplayPolarCood, false ) );

    // Default colors for layers 0 to 31
    static const COLOR4D color_default[] = {
        COLOR4D( GREEN ),     COLOR4D( BLUE ),         COLOR4D( LIGHTGRAY ), COLOR4D( MAGENTA ),
        COLOR4D( RED ),       COLOR4D( DARKGREEN ),    COLOR4D( BROWN ),     COLOR4D( MAGENTA ),
        COLOR4D( LIGHTGRAY ), COLOR4D( BLUE ),         COLOR4D( GREEN ),     COLOR4D( CYAN ),
        COLOR4D( LIGHTRED ),  COLOR4D( LIGHTMAGENTA ), COLOR4D( YELLOW ),    COLOR4D( RED ),
        COLOR4D( BLUE ),      COLOR4D( BROWN ),        COLOR4D( LIGHTCYAN ), COLOR4D( RED ),
        COLOR4D( MAGENTA ),   COLOR4D( CYAN ),         COLOR4D( BROWN ),     COLOR4D( MAGENTA ),
        COLOR4D( LIGHTGRAY ), COLOR4D( BLUE ),         COLOR4D( GREEN ),     COLOR4D( DARKCYAN ),
        COLOR4D( YELLOW ),    COLOR4D( LIGHTMAGENTA ), COLOR4D( YELLOW ),    COLOR4D( LIGHTGRAY ),
    };

    // List of keywords used as identifiers in config.
    // They *must* be static const and not temporarily created,
    // because the parameter list that use these keywords does not store them,
    // just points to them.
    static const wxChar* keys[] = {
        wxT("ColorLayer0Ex"),  wxT("ColorLayer1Ex"),  wxT("ColorLayer2Ex"),  wxT("ColorLayer3Ex"),
        wxT("ColorLayer4Ex"),  wxT("ColorLayer5Ex"),  wxT("ColorLayer6Ex"),  wxT("ColorLayer7Ex"),
        wxT("ColorLayer8Ex"),  wxT("ColorLayer9Ex"),  wxT("ColorLayer10Ex"), wxT("ColorLayer11Ex"),
        wxT("ColorLayer12Ex"), wxT("ColorLayer13Ex"), wxT("ColorLayer14Ex"), wxT("ColorLayer15Ex"),
        wxT("ColorLayer16Ex"), wxT("ColorLayer17Ex"), wxT("ColorLayer18Ex"), wxT("ColorLayer19Ex"),
        wxT("ColorLayer20Ex"), wxT("ColorLayer21Ex"), wxT("ColorLayer22Ex"), wxT("ColorLayer23Ex"),
        wxT("ColorLayer24Ex"), wxT("ColorLayer25Ex"), wxT("ColorLayer26Ex"), wxT("ColorLayer27Ex"),
        wxT("ColorLayer28Ex"), wxT("ColorLayer29Ex"), wxT("ColorLayer30Ex"), wxT("ColorLayer31Ex"),
    };

    wxASSERT( DIM(keys) == DIM(color_default) );
    wxASSERT( DIM(keys) <= DIM(g_ColorsSettings.m_LayersColors) && DIM(keys) <= DIM(color_default) );

    for( unsigned i = 0; i < DIM(keys);  ++i )
    {
        COLOR4D* prm = &g_ColorsSettings.m_LayersColors[ GERBER_DRAW_LAYER( i ) ];

        PARAM_CFG_SETCOLOR* prm_entry =
            new PARAM_CFG_SETCOLOR( true, keys[i], prm, color_default[i] );

        m_configSettings.push_back( prm_entry );
    }

    return m_configSettings;
}
