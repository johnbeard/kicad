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

#ifndef INCLUDE_FRAME_ON_START_ACTION_H_
#define INCLUDE_FRAME_ON_START_ACTION_H_

#include <wxstruct.h>

/**
 * Class FRAME_STARTUP_ACTIONS
 *
 * Object that manages startup actions for a wxWindow frame. This is
 * useful when the actions need user input or produce results that
 * depends on the frame being shown and active.
 *
 * When the frame is first shown, a list of actions is performed.
 * If an action fails and it reports that a failure is fatal, the
 * window is closed.
 */
class FRAME_STARTUP_ACTIONS
{
public:

    /**
     * Class ACTION
     *
     * A class to represent and handle a one-off action to run when a wxWindow is
     * first shown.
     */
    class ACTION
    {
    public:

        virtual ~ACTION()
        {}

        /**
         * Function DoStartAction
         *
         * Implement this function to perform your startup action.
         *
         * @return true if successful, o false if a (fatal) error
         * ocurred and the window should be closed (for example if
         * a critical resource could not be acquired).
         */
        virtual bool DoEventAction() = 0;
    };

    FRAME_STARTUP_ACTIONS( wxWindow& aWindow );

    void AddAction( std::unique_ptr<ACTION> aAction );

private:

    void onShow( wxShowEvent& event );

    wxWindow& m_window;
    std::vector<std::unique_ptr<ACTION> > m_actionList;
};


#endif /* INCLUDE_FRAME_ON_START_ACTION_H_ */
