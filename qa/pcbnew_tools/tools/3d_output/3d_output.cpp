/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 KiCad Developers, see CHANGELOG.TXT for contributors.
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

#include "3d_output.h"

#include <class_board.h>

#include <pcbnew_utils/board_file_utils.h>

#include <qa_utils/scoped_timer.h>
#include <qa_utils/stdstream_line_reader.h>

#include <common.h>

#include <wx/cmdline.h>

#include <cstdio>
#include <string>


using OUT3D_DURATION = std::chrono::microseconds;


/**
 * Simple data used when running a render
 */
struct OUTPUT_3D_PARAMS
{
    ///> Verbose printing of processing
    bool m_verbose;

    ///> Print timings during execution
    bool m_print_times;

    ///> File to write the output to
    std::string output_filename;
};


/**
 * Class to handle
 */
class OUTPUT_3D_RUNNER
{
public:

    OUTPUT_3D_RUNNER( const OUTPUT_3D_PARAMS& aParams )
        : m_params( aParams )
    {}

    bool Run( const BOARD& aBoard )
    {
        return true;
    }

private:

    const OUTPUT_3D_PARAMS& m_params;
};


static const wxCmdLineEntryDesc g_cmdLineDesc[] = {
    {
            wxCMD_LINE_SWITCH,
            "h",
            "help",
            _( "displays help on the command line parameters" ).mb_str(),
            wxCMD_LINE_VAL_NONE,
            wxCMD_LINE_OPTION_HELP,
    },
    {
            wxCMD_LINE_SWITCH,
            "v",
            "verbose",
            _( "print parsing information" ).mb_str(),
    },
    {
            wxCMD_LINE_OPTION,
            "r",
            "repeat",
            _( "times to repeat the render" ).mb_str(),
            wxCMD_LINE_VAL_NUMBER,
            wxCMD_LINE_PARAM_OPTIONAL,
    },
    {
            wxCMD_LINE_SWITCH,
            "t",
            "timings",
            _( "print rendering timings" ).mb_str(),
    },
    {
            wxCMD_LINE_PARAM,
            nullptr,
            nullptr,
            _( "input file" ).mb_str(),
            wxCMD_LINE_VAL_STRING,
            wxCMD_LINE_PARAM_OPTIONAL,
    },
    { wxCMD_LINE_NONE }
};

/**
 * Tool=specific return codes
 */
enum RENDER_RET_CODES
{
    ///> Failed to parse the board
    PARSE_FAILED = KI_TEST::RET_CODES::TOOL_SPECIFIC,

    ///> Failure during render
    RENDER_FAILED,
};


int output_3d_main( int argc, char** argv )
{
#ifdef __AFL_COMPILER
    __AFL_INIT();
#endif

    wxMessageOutput::Set( new wxMessageOutputStderr );
    wxCmdLineParser cl_parser( argc, argv );
    cl_parser.SetDesc( g_cmdLineDesc );
    cl_parser.AddUsageText(
            _( "This program produces 3D renders of PCB files. "
               "This can be used for debugging, fuzz testing or development, etc." ) );

    int cmd_parsed_ok = cl_parser.Parse();
    if( cmd_parsed_ok != 0 )
    {
        // Help and invalid input both stop here
        return ( cmd_parsed_ok == -1 ) ? KI_TEST::RET_CODES::OK : KI_TEST::RET_CODES::BAD_CMDLINE;
    }

    const bool verbose = cl_parser.Found( "verbose" );

    std::string filename;

    if( cl_parser.GetParamCount() )
    {
        filename = cl_parser.GetParam( 0 ).ToStdString();
    }

    std::unique_ptr<BOARD> board = KI_TEST::ReadBoardFromFileOrStream( filename );

    if( !board )
        return RENDER_RET_CODES::PARSE_FAILED;

    OUTPUT_3D_PARAMS exec_context{
        verbose,
        cl_parser.Found( "timings" ),
    };

    long repeats = 1;
    cl_parser.Found( "repeats", &repeats );

    OUTPUT_3D_RUNNER runner( exec_context );

    bool ok = true;

    for( long i = 0; i < repeats; ++i )
    {
        if( verbose )
        {
            std::cout << wxString::Format( "Render number %l of %l )" ) << std::endl;
        }

        ok &= runner.Run( *board );
    }

    if( !ok )
    {
        return RENDER_RET_CODES::RENDER_FAILED;
    }

    return KI_TEST::RET_CODES::OK;
}


/*
 * Define the tool interface
 */
KI_TEST::UTILITY_PROGRAM output_3d_tool = {
    "3d_output",
    "Produce 3D renders of boards",
    output_3d_main,
};