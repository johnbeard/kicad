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

#ifndef OFFSCREEN_CONTEXT__H
#define OFFSCREEN_CONTEXT__H


#include "framebuffer_object.h"


class OFFSCREEN_CONTEXT
{
public:

    OFFSCREEN_CONTEXT( int aHeight, int aWidth);

    bool Setup();

private:
    ///> Target size
    int m_height;
    int m_width;

    ///> Framebuffer Object
    FRAMEBUFFER_OBJECT*  m_fbo;

}

// struct OffscreenContext
// {
//     OffscreenContext(int width, int height) :
//         openGLContext(nullptr), xdisplay(nullptr), xwindow(0),
//         width(width), height(height),
//         fbo(nullptr) {}
//     GLXContext openGLContext;
//     Display *xdisplay;
//     Window xwindow;
//     int width;
//     int height;
//     fbo_t *fbo;
// };

#endif // OFFSCREEN_CONTEXT__H
