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

#ifndef INCLUDE_GLOBAL_FP_LIB_LOADER_H_
#define INCLUDE_GLOBAL_FP_LIB_LOADER_H_

#include <frame_startup_actions.h>
#include <wxstruct.h>    // EDA_BASE_FRAME

/**
 * Class GLOBAL_LIB_LOADER
 *
 * A frame start action that will load the global library
 * table, start the library init prompt if not, and also
 * prompt the user if the library is empty
 */
class GLOBAL_LIB_LOADER : public FRAME_STARTUP_ACTIONS::ACTION
{
public:
    GLOBAL_LIB_LOADER( EDA_BASE_FRAME& aFrame, int aManagerMenuId );

    /*!
     * Static function to show an information dialog that the footprint
     * lib needs configuration because there are not footprints found
     *
     * Placed in this class more as a namespace than as a member
     *
     * @param frame - the EDA_BASE_FRAME that the dialog is associated with
     * @param aIsWarning - trie if this is a warning dialog, false if
     *        is just an informational reminder
     * @param aManagerMenuId - the menu ID for opening the footprint
     *        library manager
     */
    static void ShowNoFootprintsDialog( EDA_BASE_FRAME& aFrame,
            bool aIsWarning,
            int aManagerMenuId );

    /**
     * Attempt to load/create the global FP library,
     *
     * @return true if a global library is created, false if not
     *         (false is fatal and triggers exit)
     */
    bool DoEventAction() override;

private:

    EDA_BASE_FRAME& m_frame;
    int m_managerMenuId;
};

#endif /* INCLUDE_GLOBAL_FP_LIB_LOADER_H_ */
