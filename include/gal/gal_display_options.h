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
class wxWindow;


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

    /**
     * A model of the options that govern the display of GAL canvases.
     *
     * This class contains two parts: the raw data in the OPTIONS struct, and
     * the signal-slot wrapper around it.
     *
     * Access to the (const) data is permitted, but to change the data, client
     * code must use the public interfaces, which enforce correct notification
     * of listeners.
     *
     * A holder of a const reference may subscribe for updates, but may not
     * modify the data (so cannot trigger updates).
     */
    class GAL_DISPLAY_OPTIONS
    {
    public:
        /**
         * Gal display options data members
         */
        class OPTIONS
        {
        public:
            OPTIONS();

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

        /**
         * Read GAL config options from applicaton-level config
         * @param aCfg      the application config base
         * @param aBaseName the application's GAL options key prefix
         */
        void ReadAppConfig( wxConfigBase& aCfg, const wxString& aBaseName );

        /**
         * Read GAL config options from the common config store
         * @param aCommonConfig the common config store
         * @param aWindow       the wx parent window (used for DPI scaling)
         */
        void ReadCommonConfig( wxConfigBase& aCommonConfig, wxWindow* aWindow );

        /**
         * Read application and common configs
         * @param aCommonConfig the common config store
         * @param aCfg          the application config base
         * @param aBaseName     the application's GAL options key prefix
         * @param aWindow       the wx parent window (used for DPI scaling)
         */
        void ReadConfig( wxConfigBase& aCommonConfig, wxConfigBase& aAppCondfig,
                const wxString& aBaseCfgName, wxWindow* aWindow );

        void WriteConfig( wxConfigBase& aCfg, const wxString& aBaseName );

        /**
         * Read-only access to the raw options data
         *
         * Write access is made though the model mutation interfaces
         * @return reference to the options
         */
        const OPTIONS& GetOptions() const
        {
            return m_options;
        }

        /**
         * Update the model with a whole new set of options, useful when initialising
         * or setting from config.
         * @param aNewOptions the new options to set
         */
        void Update( const OPTIONS& aNewOptions );

        /**
         * Force a notification, even if nothing has changed.
         */
        void Update();

        /**
         * Cycles the cursor style (currently: full screen / small) and sends
         * notification.
         */
        void ToggleCursorStyle();

        /**
         * Enable/disable the axes
         */
        void SetAxesEnabled( bool aVisible );

        /**
         * Toggle whether or not the cursor is always shown
         */
        void ToggleForceDisplayCursor();

        /**
         * Set relevant anti-aliasing modes per canvas.
         */
        void SetAntiAliasingModes(
                OPENGL_ANTIALIASING_MODE aOpenGLMode, CAIRO_ANTIALIASING_MODE aCairoMode );

        /**
         * A signal that can be subscribed to for notifications when this object changes
         *
         * Mutable: this can be subscribed to even on a const GAL_DISPLAY_OPTIONS reference
         */
        mutable SIGNAL::signal<void( const OPTIONS& )> m_sig_on_changed;

    private:
        /**
         * The internal copy of the options data.
         */
        OPTIONS m_options;

        /**
         * Send notification about model changes to listeners
         */
        void notifyChanged();
    };

}

#endif

