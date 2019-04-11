/*
* This program source code file is part of KICAD, a free EDA CAD application.
*
* Copyright (C) 2016-2017 Kicad Developers, see change_log.txt for contributors.
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

#include <gal/gal_display_options.h>

#include <wx/config.h>

#include <config_map.h>
#include <dpi_scaling.h>
#include <pgm_base.h>

using namespace KIGFX;

/*
 * Config option strings
 */
static const wxString GalGridStyleConfig( "GridStyle" );
static const wxString GalGridLineWidthConfig( "GridLineWidth" );
static const wxString GalGridMaxDensityConfig( "GridMaxDensity" );
static const wxString GalGridAxesEnabledConfig( "GridAxesEnabled" );
static const wxString GalFullscreenCursorConfig( "CursorFullscreen" );
static const wxString GalForceDisplayCursorConfig( "ForceDisplayCursor" );


static const UTIL::CFG_MAP<KIGFX::GRID_STYLE> gridStyleConfigVals =
{
    { KIGFX::GRID_STYLE::DOTS,       0 },
    { KIGFX::GRID_STYLE::LINES,      1 },
    { KIGFX::GRID_STYLE::SMALL_CROSS,2 },
};


GAL_DISPLAY_OPTIONS::OPTIONS::OPTIONS()
    : gl_antialiasing_mode( OPENGL_ANTIALIASING_MODE::NONE ),
      cairo_antialiasing_mode( CAIRO_ANTIALIASING_MODE::NONE ),
      m_gridStyle( GRID_STYLE::DOTS ),
      m_gridLineWidth( 1.0 ),
      m_gridMinSpacing( 10.0 ),
      m_axesEnabled( false ),
      m_fullscreenCursor( false ),
      m_forceDisplayCursor( false ),
      m_scaleFactor( DPI_SCALING::GetDefaultScaleFactor() )
{}


void GAL_DISPLAY_OPTIONS::ReadAppConfig( wxConfigBase& aCfg, const wxString& aBaseName )
{
    const wxString baseName = aBaseName + GAL_DISPLAY_OPTIONS_KEY;

    long readLong; // Temp value buffer

    aCfg.Read( baseName + GalGridStyleConfig, &readLong,
            static_cast<long>( KIGFX::GRID_STYLE::DOTS ) );
    m_options.m_gridStyle = UTIL::GetValFromConfig( gridStyleConfigVals, readLong );

    aCfg.Read( baseName + GalGridLineWidthConfig, &m_options.m_gridLineWidth, 1.0 );
    aCfg.Read( baseName + GalGridMaxDensityConfig, &m_options.m_gridMinSpacing, 10 );
    aCfg.Read( baseName + GalGridAxesEnabledConfig, &m_options.m_axesEnabled, false );
    aCfg.Read( baseName + GalFullscreenCursorConfig, &m_options.m_fullscreenCursor, false );
    aCfg.Read( baseName + GalForceDisplayCursorConfig, &m_options.m_forceDisplayCursor, true );

    notifyChanged();
}


void GAL_DISPLAY_OPTIONS::ReadCommonConfig( wxConfigBase& aCommonConfig, wxWindow* aWindow )
{
    int temp;
    aCommonConfig.Read(
            GAL_ANTIALIASING_MODE_KEY, &temp, (int) KIGFX::OPENGL_ANTIALIASING_MODE::NONE );
    m_options.gl_antialiasing_mode = (KIGFX::OPENGL_ANTIALIASING_MODE) temp;

    aCommonConfig.Read(
            CAIRO_ANTIALIASING_MODE_KEY, &temp, (int) KIGFX::CAIRO_ANTIALIASING_MODE::NONE );
    m_options.cairo_antialiasing_mode = (KIGFX::CAIRO_ANTIALIASING_MODE) temp;

    {
        const DPI_SCALING dpi{ &aCommonConfig, aWindow };
        m_options.m_scaleFactor = dpi.GetScaleFactor();
    }

    notifyChanged();
}


void GAL_DISPLAY_OPTIONS::ReadConfig( wxConfigBase& aCommonConfig, wxConfigBase& aAppConfig,
        const wxString& aBaseCfgName, wxWindow* aWindow )
{
    ReadAppConfig( aAppConfig, aBaseCfgName );

    ReadCommonConfig( aCommonConfig, aWindow );
}


void GAL_DISPLAY_OPTIONS::WriteConfig( wxConfigBase& aCfg, const wxString& aBaseName )
{
    const wxString baseName = aBaseName + GAL_DISPLAY_OPTIONS_KEY;

    aCfg.Write( baseName + GalGridStyleConfig,
                 UTIL::GetConfigForVal( gridStyleConfigVals, m_options.m_gridStyle ) );

    aCfg.Write( baseName + GalGridLineWidthConfig, m_options.m_gridLineWidth );
    aCfg.Write( baseName + GalGridMaxDensityConfig, m_options.m_gridMinSpacing );
    aCfg.Write( baseName + GalGridAxesEnabledConfig, m_options.m_axesEnabled );
    aCfg.Write( baseName + GalFullscreenCursorConfig, m_options.m_fullscreenCursor );
    aCfg.Write( baseName + GalForceDisplayCursorConfig, m_options.m_forceDisplayCursor );
}


void GAL_DISPLAY_OPTIONS::Update( const OPTIONS& aNewOptions )
{
    m_options = aNewOptions;
    notifyChanged();
}


void GAL_DISPLAY_OPTIONS::Update()
{
    notifyChanged();
}


void GAL_DISPLAY_OPTIONS::notifyChanged()
{
    m_sig_on_changed( m_options );
}


void GAL_DISPLAY_OPTIONS::ToggleCursorStyle()
{
    m_options.m_fullscreenCursor = !m_options.m_fullscreenCursor;
    notifyChanged();
}


void GAL_DISPLAY_OPTIONS::SetAxesEnabled( bool aVisible )
{
    m_options.m_axesEnabled = aVisible;
    notifyChanged();
}


void GAL_DISPLAY_OPTIONS::ToggleForceDisplayCursor()
{
    m_options.m_forceDisplayCursor = !m_options.m_forceDisplayCursor;
    notifyChanged();
}


void GAL_DISPLAY_OPTIONS::SetAntiAliasingModes(
        OPENGL_ANTIALIASING_MODE aOpenGLMode, CAIRO_ANTIALIASING_MODE aCairoMode )
{
    m_options.gl_antialiasing_mode = aOpenGLMode;
    m_options.cairo_antialiasing_mode = aCairoMode;
    notifyChanged();
}