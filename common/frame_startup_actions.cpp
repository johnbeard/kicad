/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <frame_startup_actions.h>


FRAME_STARTUP_ACTIONS::FRAME_STARTUP_ACTIONS( wxWindow& aWindow ) :
    m_window( aWindow )
{
    m_window.Bind( wxEVT_SHOW, &FRAME_STARTUP_ACTIONS::onShow, this );
}


void FRAME_STARTUP_ACTIONS::AddAction( std::unique_ptr<ACTION> aAction )
{
    m_actionList.push_back( std::move( aAction ) );
}


void FRAME_STARTUP_ACTIONS::onShow( wxShowEvent& event )
{
    m_window.Unbind( wxEVT_SHOW, &FRAME_STARTUP_ACTIONS::onShow, this );

    bool fatalError = false;

    for( auto& check : m_actionList )
    {
        if( !check->DoEventAction() )
        {
            fatalError = true;
            break;
        }
    }

    if( fatalError )
    {
        m_window.Close();
    }

    // all checks done, we can clear the list
    m_actionList.clear();
}
