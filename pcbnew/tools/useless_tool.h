/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _RATSNEST_TOOL_H
#define _RATSNEST_TOOL_H

#include <tools/pcb_tool.h>
#include <tools/tool_menu.h>

class PCB_EDIT_FRAME;

class USELESS_TOOL : public PCB_TOOL
{
public:
    USELESS_TOOL();
    ~USELESS_TOOL();

    /// @copydoc TOOL_BASE::Reset()
    void Reset( RESET_REASON aReason ) override;

    /// @copydoc TOOL_BASE::Init()
    bool Init() override;

    /// @copydoc TOOL_BASE::SetTransitions()
    void SetTransitions() override;

private:

    /// Move selected left interactive tool
    int moveLeft( const TOOL_EVENT& aEvent );

    /// Internal function to perform the move left action
    void moveLeftInt();

    /// Add a fixed size circle
    int fixedCircle( const TOOL_EVENT& aEvent );

    /// Menu model displayed by the tool.
    TOOL_MENU m_menu;
};

#endif // _RATSNEST_TOOL_H
