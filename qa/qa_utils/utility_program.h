/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef UTILITY_PROGRAM_H
#define UTILITY_PROGRAM_H

#include <functional>

/**
 * Return codes for tools
 */
enum RET_CODES
{
    /// Tool exited OK
    OK = 0,

    /// The command line was not correct for the tool
    BAD_CMDLINE = 1,

    /// The tool asked for was not found
    UNKNOWN_TOOL = 2,

    /// Tools can define their own statuses from here onwards
    TOOL_SPECIFIC = 10,
};

struct UTILITY_PROGRAM
{
    /// A function that provides the program for a given command line
    using FUNC = std::function<int( int argc, char** argv )>;

    /// The name of the program (this is used to select one)
    std::string m_name;

    /// Description of the program
    std::string m_desc;

    /// The function to call to run the program
    FUNC m_func;
};

#endif // UTILITY_PROGRAM_H