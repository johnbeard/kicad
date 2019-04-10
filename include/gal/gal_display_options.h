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

#ifndef GAL_DISPLAY_OPTIONS_H__
#define GAL_DISPLAY_OPTIONS_H__

#include <signal/signal.h>

class wxConfigBase;
class wxString;

namespace KIGFX
{
    /**
     * GRID_STYLE: Type definition of the grid style
     */
    enum class GRID_STYLE
    {
        DOTS,       ///< Use dots for the grid
        LINES,      ///< Use lines for the grid
        SMALL_CROSS ///< Use small cross instead of dots for the grid
    };

    enum class OPENGL_ANTIALIASING_MODE
    {
        NONE,
        SUBSAMPLE_HIGH,
        SUBSAMPLE_ULTRA,
        SUPERSAMPLING_X2,
        SUPERSAMPLING_X4,
    };

    enum class CAIRO_ANTIALIASING_MODE
    {
        NONE,
        FAST,
        GOOD,
        BEST,
    };

    class GAL_DISPLAY_OPTIONS
    {
    public:
        GAL_DISPLAY_OPTIONS();

        SIGNAL::signal<void( const GAL_DISPLAY_OPTIONS& )> m_sig_on_changed;

        void ReadConfig ( wxConfigBase* aCfg, const wxString& aBaseName );
        void WriteConfig( wxConfigBase* aCfg, const wxString& aBaseName );

        void NotifyChanged();

        OPENGL_ANTIALIASING_MODE gl_antialiasing_mode;

        CAIRO_ANTIALIASING_MODE cairo_antialiasing_mode;

        ///> The grid style to draw the grid in
        KIGFX::GRID_STYLE m_gridStyle;

        ///> Thickness to render grid lines/dots
        double m_gridLineWidth;

        ///> Minimum pixel distance between displayed grid lines
        double m_gridMinSpacing;

        ///> Whether or not to draw the coordinate system axes
        bool m_axesEnabled;

        ///> Fullscreen crosshair or small cross
        bool m_fullscreenCursor;

        ///> Force cursor display
        bool m_forceDisplayCursor;

        ///> The pixel scale factor (>1 for hi-DPI scaled displays)
        double m_scaleFactor;
    };

}

#endif

