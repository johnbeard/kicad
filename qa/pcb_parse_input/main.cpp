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
#include <class_board.h>

#include <properties.h>
#include <eagle_plugin.h>
#include <legacy_plugin.h>
#include <pcad2kicadpcb_plugin/pcad_plugin.h>

#include <wx/cmdline.h>
#include <wx/wfstream.h>

#include <stdstream_line_reader.h>
#include <scoped_timer.h>

#include <functional>
#include <map>

/**
 * A function that parses something and returns a BOARD_ITEM.
 */
using PARSE_FUNCTION = std::function<BOARD_ITEM*()>;


using PARSE_DURATION = std::chrono::microseconds;


/**
 * Parse something with some function, and report the duration, errors
 * and success.
 * @param  aFunc    the parsing function
 * @param  aVerbose print extra info, like durations
 * @return          parsed a valid BOARD_ITEM
 */
bool instrumented_parse( PARSE_FUNCTION aFunc, bool aVerbose )
{
    BOARD_ITEM* board = nullptr;
    PARSE_DURATION duration {};

    try
    {
        SCOPED_TIMER<PARSE_DURATION> timer( duration );
        board = aFunc();
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

/**
 * Parse a PCB file from the given input stream with the specified plugin.
 *
 * @param  aStream  the stream to read from
 * @param  aName    the stream name
 * @param  aVerbose whether to print verbose info
 * @return          parsed OK?
 */
bool parse(PLUGIN& aPlugin, wxInputStream& aStream, const wxString& aName, bool aVerbose )
{
    return instrumented_parse([&aPlugin, &aStream, &aName](){
        return aPlugin.Load( aStream, aName, nullptr, nullptr );
    }, aVerbose );
}


/**
 * Parse a .kicad_mod file from the given stream. This uses the parser
 * manually, as there is no stream-based access to this part of the plugin.
 *
 * @param  aStream  the stream to read from
 * @param  aName    the stream name
 * @param  aVerbose whether to print verbose info
 * @return          parsed OK?
 */
bool parse_kicad_mod( wxInputStream& aStream, const wxString& aName, bool aVerbose )
{
    INPUTSTREAM_LINE_READER reader( &aStream, aName );

    PCB_PARSER parser;
    parser.SetLineReader( &reader );

    return instrumented_parse([&parser](){
        return parser.Parse();
    }, aVerbose );
}


/**
 * The static commmand line parameters
 */
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


/**
 * Possible return codes for this program
 */
enum RET_CODES
{
    OK = 0,             ///< Returned OK
    BAD_CMDLINE = 1,    ///< Command line is invalid
    PARSE_FAILED = 2,   ///< One or more files failed to parse
};


const static std::map< std::string, IO_MGR::PCB_FILE_T > type_map = {
    { "kicad_pcb", IO_MGR::PCB_FILE_T::KICAD_SEXP },
    { "kicad_mod", IO_MGR::PCB_FILE_T::FILE_TYPE_NONE },
    { "kicad_brd", IO_MGR::PCB_FILE_T::LEGACY },
    { "eagle_brd", IO_MGR::PCB_FILE_T::EAGLE },
    { "pcad_pcb", IO_MGR::PCB_FILE_T::PCAD },
};


/**
 * Load the plugin from the IO_MGR based on the given type string.
 *
 * If plugin is returned, the caller must release it.
 */
static PLUGIN* GetPluginForCmdLine( const std::string& aType )
{
    PLUGIN* plugin = nullptr;
    auto found = type_map.find( aType );

    if( found != type_map.end() )
    {
        plugin = IO_MGR::PluginFind( found->second );
    }
    else
    {
        std::stringstream err;
        err << "Unknown filetype: " << aType;
        throw std::runtime_error( err.str() );
    }

    return plugin;
}


static std::string GetTypeDescText()
{
    std::stringstream desc;

    desc << "the type of file to parse (one of ";

    auto it = type_map.begin();
    desc << (it++)->first;

    for (; it != type_map.end(); it++)
    {
        desc << ", " << it->first;
    }

    desc << ")";

    return desc.str();
}


static std::vector<std::string> GetInputStrings( const wxCmdLineParser& aClParser )
{
    std::vector<std::string> inputs;

    const auto file_count = aClParser.GetParamCount();

    if ( file_count == 0 )
    {
        // Empty string -> use stdin
        inputs.push_back("");
    }
    else
    {
        // Parse 'n' files given on the command line
        // (this is useful for input minimisation (e.g. afl-tmin) as
        // well as manual testing
        for( unsigned i = 0; i < file_count; i++ )
        {
            const auto filename = aClParser.GetParam( i );
            inputs.push_back( filename.ToStdString() );
        }
    }

    return inputs;
}


int main(int argc, char** argv)
{
#ifdef __AFL_COMPILER
    __AFL_INIT();
#endif

    wxMessageOutput::Set(new wxMessageOutputStderr);
    wxCmdLineParser cl_parser( argc, argv );
    cl_parser.SetDesc( g_cmdLineDesc );

    cl_parser.AddOption( "t", "file-type", GetTypeDescText(),
        wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL );

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

    wxString file_type = "kicad_pcb";
    cl_parser.Found( "file-type", &file_type );

    PLUGIN* plugin = nullptr;

    try
    {
        plugin = GetPluginForCmdLine( file_type.ToStdString() );
    }
    catch( const std::exception& e )
    {
        std::cerr << e.what() << std::endl;
        return BAD_CMDLINE;
    }

    unsigned failed_cnt = 0;
    PARSE_DURATION duration;

    const auto inputs = GetInputStrings( cl_parser );

    for( const auto& input: inputs )
    {
        // Parse the file provided on stdin - used by AFL to drive the
        // program
        // while (__AFL_LOOP(2))
        std::unique_ptr<wxInputStream> in_stream;

        if( input.size() == 0 )
            in_stream = std::make_unique<wxFFileInputStream>( stdin );
        else
            in_stream = std::make_unique<wxFFileInputStream>( input );

        if( verbose )
            std::cout << "Parsing: " << input << std::endl;

        bool file_ok;
        if( plugin )
            file_ok = parse( *plugin, *in_stream, input, verbose );
        else
            file_ok = parse_kicad_mod( *in_stream, input, verbose );

        if( !file_ok )
        {
            std::cerr << "Parse failed: " << input << std::endl;
            failed_cnt++;
        }
    }

    IO_MGR::PluginRelease( plugin );

    if( failed_cnt )
    {
        std::cerr << failed_cnt << " files failed to parse" << std::endl;
        return RET_CODES::PARSE_FAILED;
    }

    return RET_CODES::OK;
}