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


#include <common.h>
#include <profile.h>

#include <kicad_plugin.h>
#include <class_module.h>

#include <qa_utils/utility_registry.h>

#include <wx/cmdline.h>

#include <cstdio>
#include <string>
#include <thread>


using PARSE_DURATION = std::chrono::microseconds;


static const wxCmdLineEntryDesc g_cmdLineDesc[] = {
    { wxCMD_LINE_SWITCH, "h", "help", _( "displays help on the command line parameters" ).mb_str(),
            wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
    { wxCMD_LINE_SWITCH, "v", "verbose", _( "print loading information" ).mb_str() },
    {
        wxCMD_LINE_OPTION,
        "r",
        "repeat",
        _( "times to load each library (default=1)" ).mb_str(),
        wxCMD_LINE_VAL_NUMBER,
        wxCMD_LINE_PARAM_OPTIONAL
    },
    { wxCMD_LINE_PARAM, nullptr, nullptr, _( "input libraries" ).mb_str(), wxCMD_LINE_VAL_STRING,
            wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE },
    { wxCMD_LINE_NONE }
};


enum LOADER_RET_CODES
{
    PARSE_FAILED = KI_TEST::RET_CODES::TOOL_SPECIFIC,
};


struct LIB_LOAD_THREAD_CONTEXT
{
    // The shared PCB_IO plugin
    PLUGIN& m_plugin;

    // The li path to load FPs from
    wxString m_lib_path;

    // How many times to repeatedly load FPs
    long m_reps;
};


void lib_load_thread_fn(LIB_LOAD_THREAD_CONTEXT& aContext)
{
    for (int i = 0; i < aContext.m_reps; ++i)
    {
        std::cout << "Loading from :" << aContext.m_lib_path << std::endl;

        wxArrayString footprint_names;
        aContext.m_plugin.FootprintEnumerate(footprint_names, aContext.m_lib_path, true, nullptr);

        for (const auto fp_name : footprint_names)
        {
            std::unique_ptr<MODULE> mod {
                aContext.m_plugin.FootprintLoad(aContext.m_lib_path, fp_name, nullptr)
            };

            if (!mod)
            {
                // This is odd, as the FP was reported by FootprintEnumerate
                std::cout << "Module not found: " << fp_name << std::endl;
            }
            else
            {
                // std::cout << "Found module" << std::endl;
            }
        }
    }
}


int lib_loader_main_func( int argc, char** argv )
{
    wxMessageOutput::Set( new wxMessageOutputStderr );
    wxCmdLineParser cl_parser( argc, argv );
    cl_parser.SetDesc( g_cmdLineDesc );
    cl_parser.AddUsageText(
            _( "This program loads footprints from libraries. This allows "
               "profiling, benchmarking and debugging of library loading. "
               "For loading of individual footprints, rather than whole "
               "libraries, use the pcb_parser tool." ) );

    int cmd_parsed_ok = cl_parser.Parse();
    if( cmd_parsed_ok != 0 )
    {
        // Help and invalid input both stop here
        return ( cmd_parsed_ok == -1 ) ? KI_TEST::RET_CODES::OK : KI_TEST::RET_CODES::BAD_CMDLINE;
    }

    long reps = 1;
    cl_parser.Found("repeat", &reps);

    // int threads = 2;
    // int repeat = 1000;

    PCB_IO pcb_io(CTL_FOR_LIBRARY);


    std::vector<LIB_LOAD_THREAD_CONTEXT> contexts;

    const auto lib_count = cl_parser.GetParamCount();
    for (unsigned i = 0; i < lib_count; ++i)
    {
        contexts.push_back({
            pcb_io,
            cl_parser.GetParam( i ).ToStdString(),
            reps
        });
    }

    std::vector<std::thread> threads;

    for (auto& ctx : contexts)
    {
        std::thread thread(lib_load_thread_fn, std::ref(ctx));

        // keep the thread so we can join it
        threads.push_back(std::move(thread));
    }

    // join all the threads
    for (auto& thread : threads)
    {
        if (thread.joinable())
            thread.join();
    }

    return KI_TEST::RET_CODES::OK;
}


static bool registered = UTILITY_REGISTRY::Register({
    "lib_loader",
    "Load libraries",
    lib_loader_main_func,
});