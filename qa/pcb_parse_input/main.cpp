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


#include <kicad_plugin.h>
#include <pcb_parser.h>
#include <richio.h>
#include <class_board_item.h>

#include <wx/cmdline.h>

#include <stdstream_line_reader.h>
#include <scoped_timer.h>

using PARSE_DURATION = std::chrono::microseconds;

/**
 * Parse a PCB or footprint file from the given input stream
 *
 * @param aStream the input stream to read from
 * @return success, duration (in us)
 */
bool parse(std::istream& aStream, bool aVerbose )
{
    // Take input from stdin
    STDISTREAM_LINE_READER reader;
    reader.SetStream( aStream );

    PCB_PARSER parser;

    parser.SetLineReader( &reader );

    BOARD_ITEM* board = nullptr;

    PARSE_DURATION duration {};

    try
    {
        SCOPED_TIMER<PARSE_DURATION> timer( duration );
        board = parser.Parse();
    }
    catch( const IO_ERROR& parse_error )
    {
        std::cerr << parse_error.Problem() << std::endl;
        std::cerr << parse_error.Where() << std::endl;
    }

    if( aVerbose )
    {
        std::cout << "Took: " << duration.count() << "us" << std::endl;
    }

    return board != nullptr;
}


static const wxCmdLineEntryDesc g_cmdLineDesc [] =
{
    { wxCMD_LINE_SWITCH, "h", "help",
        _( "displays help on the command line parameters" ),
        wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
    { wxCMD_LINE_SWITCH, "v", "verbose",
        _( "print parsing information") },
    { wxCMD_LINE_PARAM, nullptr, nullptr,
        _( "input file" ),
        wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE }
};


enum RET_CODES
{
    OK = 0,
    BAD_CMDLINE = 1,
    PARSE_FAILED = 2,
};


int main(int argc, char** argv)
{
#ifdef __AFL_COMPILER
    __AFL_INIT();
#endif

    wxMessageOutput::Set(new wxMessageOutputStderr);
    wxCmdLineParser cl_parser( argc, argv );
    cl_parser.SetDesc( g_cmdLineDesc );
    cl_parser.AddUsageText( _("This program parses PCB files, either from the "
        "stdin stream or from the given filenames. This can be used either for "
        "standalone testing of the parser or for fuzz testing." ) );

    int cmd_parsed_ok = cl_parser.Parse();
    if( cmd_parsed_ok != 0 )
    {
        // Help and invalid input both stop here
        return ( cmd_parsed_ok == -1 ) ? RET_CODES::OK : RET_CODES::BAD_CMDLINE;
    }

    const bool verbose = cl_parser.Found( "verbose" );

    bool ok = true;
    PARSE_DURATION duration;

    const auto file_count = cl_parser.GetParamCount();

    if ( file_count == 0 )
    {
        // Parse the file provided on stdin - used by AFL to drive the
        // program
        // while (__AFL_LOOP(2))
        {
            ok = parse( std::cin, verbose );
        }
    }
    else
    {
        // Parse 'n' files given on the command line
        // (this is useful for input minimisation (e.g. afl-tmin) as
        // well as manual testing
        for( unsigned i = 0; i < file_count; i++ )
        {
            const auto filename = cl_parser.GetParam( i );

            if( verbose )
                std::cout << "Parsing: " << filename << std::endl;

            std::ifstream fin;
            fin.open( filename );

            ok = ok && parse( fin, verbose );
        }
    }

    if( !ok )
        return RET_CODES::PARSE_FAILED;

    return RET_CODES::OK;
}