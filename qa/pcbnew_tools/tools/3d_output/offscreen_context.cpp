/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2019 KiCad Developers, see CHANGELOG.TXT for contributors.
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

#include "offscreen_context.h"


OFFSCREEN_CONTEXT::OFFSCREEN_CONTEXT( int aHeight, int aWidth )
        : m_height( aHeight ), m_width( aWidth ), m_fbo( nullptr )
{}


OFFSCREEN_CONTEXT* OFFSCREEN_CONTEXT::Setup()
{
    GLenum err = glewInit(); // must come after Context creation and before FBO construction

    if( err != GLEW_OK )
    {
        std::cerr << "Unable to init GLEW: " << glewGetErrorString(err) << "\n";
        return false;
    }

    m_fbo = fbo_new();

    if ( !fbo_init( m_fbo, m_width, m_height ) )
    {
        return false;
    }

    return true;
}