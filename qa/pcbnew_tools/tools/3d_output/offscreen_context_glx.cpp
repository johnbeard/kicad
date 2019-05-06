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

#include <cstdlib>

#include <GL/gl.h>
#include <GL/glx.h>

/**
 * Offscreen GL context for GLX (i.e. Linux) systems.
 */
class OFFSCREEN_CTX_GLX: public OFFSCREEN_CONTEXT
{
public:

    OFFSCREEN_CTX_GLX();

    bool DummyCtx();

    /**
     * Create a dummy X window without showing it. (without 'mapping' it)
     *
     * This will set m_xdisplay and m_openGLContext if it succeeds
     * @return true if success
     */
    bool CreateDummyWindow();

private:

    GLXContext m_openGLContext;
    Display *m_xdisplay;
    Window m_xwindow;
}

OFFSCREEN_CTX_GLX::OFFSCREEN_CTX_GLX(int width, int height) :
    OFFSCREEN_CONTEXT( aWidth, aHeight ),
    m_openGLContext(nullptr), m_xdisplay(nullptr), m_xwindow(0)
{}


bool OFFSCREEN_CTX_GLX::DummyCtx()
{
    // This will alter ctx.openGLContext and ctx.xdisplay and ctx.xwindow if successfull
    int major;
    int minor;
    bool result = false;

    m_xdisplay = XOpenDisplay(nullptr);

    if (ctx.xdisplay == nullptr)
    {
        std::cerr << "Unable to open a connection to the X server.\n";
        auto dpyenv = std::getenv("DISPLAY");
        std::cerr << "DISPLAY=" << (dpyenv?dpyenv:"") << "\n";
        return false;
    }

    // glxQueryVersion is not always reliable. Use it, but then
    // also check to see if GLX 1.3 functions exist

    glXQueryVersion(ctx.xdisplay, &major, &minor);
    if (major==1 && minor<=2 && glXGetVisualFromFBConfig==nullptr) {
        std::cerr << "Error: GLX version 1.3 functions missing. "
            << "Your GLX version: " << major << "." << minor << std::endl;
    } else {
        result = create_glx_dummy_window(ctx);
    }

    if (!result) XCloseDisplay(ctx.xdisplay);
    return result;
}

static XErrorHandler original_xlib_handler = nullptr;

static bool XCreateWindow_failed = false;

/**
 * Custom X error handler function
 */
static int XCreateWindow_error(Display *dpy, XErrorEvent *event)
{
    std::cerr << "XCreateWindow failed: XID: " << event->resourceid
         << " request: " << static_cast<int>(event->request_code)
         << " minor: " << static_cast<int>(event->minor_code) << "\n";

    char description[1024];
    XGetErrorText( dpy, event->error_code, description, 1023 );

    std::cerr << " error message: " << description << "\n";

    XCreateWindow_failed = true;
    return 0;
}

/*
 * This purposely does not use glxCreateWindow, to avoid crashes,
 * "failed to create drawable" errors, and Mesa "WARNING: Application calling 
 * GLX 1.3 function when GLX 1.3 is not supported! This is an application bug!"
 * This function will alter ctx.openGLContext and ctx.xwindow if successfull
 */
bool OFFSCREEN_CTX_GLX::CreateDummyWindow()
{
    // Cargo-culted from OpenSCAD
    int attributes[] = {
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PIXMAP_BIT | GLX_PBUFFER_BIT,
        GLX_RENDER_TYPE,   GLX_RGBA_BIT,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, true,
        None
    };

    Display* dpy = ctx.m_xdisplay;

    int num_returned = 0;

    auto fbconfigs = glXChooseFBConfig( dpy, DefaultScreen(dpy), attributes, &num_returned );
    if (fbconfigs == nullptr)
    {
        std::cerr << "glXChooseFBConfig failed\n";
        return false;
    }

    auto visinfo = glXGetVisualFromFBConfig( dpy, fbconfigs[0] );

    if (visinfo == nullptr)
    {
        std::cerr << "glXGetVisualFromFBConfig failed\n";
        XFree(fbconfigs);
        return false;
    }

    // can't depend on xWin==nullptr at failure. use a custom Xlib error handler instead.
    original_xlib_handler = XSetErrorHandler(XCreateWindow_error);

    auto root = DefaultRootWindow(dpy);
    XSetWindowAttributes xwin_attr;
    auto width = ctx.width;
    auto height = ctx.height;
    xwin_attr.background_pixmap = None;
    xwin_attr.background_pixel = 0;
    xwin_attr.border_pixel = 0;
    xwin_attr.colormap = XCreateColormap( dpy, root, visinfo->visual, AllocNone);
    xwin_attr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
    unsigned long int mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

    auto xWin = XCreateWindow( dpy, root, 0, 0, width, height,
                               0, visinfo->depth, InputOutput,
                               visinfo->visual, mask, &xwin_attr );

    // Window xWin = XCreateSimpleWindow( dpy, DefaultRootWindow(dpy), 0,0,42,42, 0,0,0 );

    XSync(dpy, false);
    if (XCreateWindow_failed)
    {
        XFree(visinfo);
        XFree(fbconfigs);
        return false;
    }

    XSetErrorHandler(original_xlib_handler);

    // Most programs would call XMapWindow here. But we don't, to keep the window hidden
    // XMapWindow( dpy, xWin );

    GLXContext context = glXCreateNewContext(dpy, fbconfigs[0], GLX_RGBA_TYPE, nullptr, true);

    if( context == nullptr )
    {
        std::cerr << "glXCreateNewContext failed\n";
        XDestroyWindow(dpy, xWin);
        XFree(visinfo);
        XFree(fbconfigs);
        return false;
    }

    //GLXWindow glxWin = glXCreateWindow( dpy, fbconfigs[0], xWin, nullptr );

    if ( !glXMakeContextCurrent( dpy, xWin, xWin, context ) )
    {
        //if (!glXMakeContextCurrent( dpy, glxWin, glxWin, context )) {
        std::cerr << "glXMakeContextCurrent failed\n";
        glXDestroyContext(dpy, context);
        XDestroyWindow(dpy, xWin);
        XFree(visinfo);
        XFree(fbconfigs);
        return false;
    }

    m_openGLContext = context;
    m_xwindow = xWin;

    XFree(visinfo);
    XFree(fbconfigs);

    return true;
}
