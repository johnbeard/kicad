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

#include <global_fp_lib_loader.h>

#include <pcbnew_id.h>
#include <fp_lib_table.h>
#include <dialogs/dialog_multibutton.h>
#include <confirm.h>


/**
 * Function showFootprintLibHelperDialog
 *
 * Helper function to pop up a Cancel/Help/FP Manager dialog and
 * dispatch events to the EDA_BASE_FRAME if needed
 */
static void showFootprintLibHelperDialog( EDA_BASE_FRAME& aFrame,
        bool aWarning, const wxString& aTitle, const wxString& aMsg,
        int aManagerMenuId )
{
    DIALOG_MULTIBUTTON dialog( &aFrame, aTitle );

    dialog.AddText( aMsg );

    dialog.AddButton( wxID_CANCEL, wxEmptyString, true );
    dialog.AddButton( wxID_HELP, _( "Open Manual" ) );
    dialog.AddButton( wxID_OK, _( "Footprint Library Manager" ) );

    dialog.SetHintImage( aWarning ? wxART_WARNING : wxART_INFORMATION );

    dialog.AdjustSize();

    int ret = dialog.ShowModal();

    switch( ret )
    {
    case wxID_OK:
        aFrame.PostCommandMenuEvent( aManagerMenuId );
        break;

    case wxID_HELP:
        aFrame.PostCommandMenuEvent( wxID_HELP );
        break;

    default:
        break;
    }
}


/**
 * Class GLOBAL_FP_LOADER_UI_PROVIDER
 *
 * Implemention of the SETUP_UI_PROVIDER template class to
 * provide UI prompts for the current frame
 */
class GLOBAL_FP_LOADER_UI_PROVIDER : public FP_LIB_TABLE::SETUP_UI_PROVIDER
{
public:

    GLOBAL_FP_LOADER_UI_PROVIDER( EDA_BASE_FRAME& aFrame ) :
        m_frame( aFrame )
    {}

    void ShowError( const wxString& msg ) override
    {
        DisplayError( &m_frame, msg );
    }

    virtual INIT_ACTION PromptForInitAction() override
    {
        const wxString msg = _(
                "No global footprint library table was found.\n\n"
                "Would you like to initialise an empty table, "
                "copy an example table, or exit and create the table "
                "yourself?\n\n" );

        DIALOG_MULTIBUTTON dialog( &m_frame,
                _( "No Global Footprint Library Table" ) );

        dialog.AddText( msg );

        dialog.SetHintImage( wxART_ERROR );

        dialog.AddButton( wxID_EXIT, wxEmptyString, true );
        dialog.AddButton( (int) INIT_ACTION::COPY_EXAMPLE, _( "Copy example table" ) );
        dialog.AddButton( (int) INIT_ACTION::CREATE_EMPTY, _( "Create empty table" ) );

        dialog.AdjustSize();

        int ret = dialog.ShowModal();

        if( ret == (int) INIT_ACTION::COPY_EXAMPLE )
            return INIT_ACTION::COPY_EXAMPLE;
        else if( ret == (int) INIT_ACTION::CREATE_EMPTY )
            return INIT_ACTION::CREATE_EMPTY;

        return INIT_ACTION::EXIT;
    }

private:

    EDA_BASE_FRAME& m_frame;
};


GLOBAL_LIB_LOADER::GLOBAL_LIB_LOADER( EDA_BASE_FRAME& aFrame,
        int aManagerMenuId ) :
    m_frame( aFrame ),
    m_managerMenuId( aManagerMenuId )
{
}


bool GLOBAL_LIB_LOADER::DoEventAction()
{
    GLOBAL_FP_LOADER_UI_PROVIDER uiProv( m_frame );

    bool needsInit;

    if( !FP_LIB_TABLE::LoadGlobalTable( GFootprintTable, needsInit, uiProv ) )
    {
        // no global library found or created
        return false;
    }

    // so we have a global library, but if there anything in it?
    // if empty, prompt for config - gently if the user just
    // created a default table, or more loudly if we would normally
    // have expected a more configured state
    if( GFootprintTable.GetLogicalLibs().size() == 0 )
    {
        ShowNoFootprintsDialog( m_frame, !needsInit, m_managerMenuId );
    }

    // whether or not the user configured it, we at least have
    // a global table now
    return true;
}


// The globally-accessible dialog function
void GLOBAL_LIB_LOADER::ShowNoFootprintsDialog( EDA_BASE_FRAME& aFrame,
        bool aIsWarning,
        int aManagerMenuId )
{
    wxString msg, title;

    if( aIsWarning )
    {
        // text to show if this is a error dialog
        title = _( "No Footprints in Library" );

        msg = _( "No footprints could be loaded from the library.\n\n"
                 "Use the Footprint Libraries Manager to add libraries "
                 "or check existing library configuration."
                );
    }
    else
    {
        // text to show is this is only an informational dialog
        msg = _( "You must configure the library table "
                 "to include all footprint libraries you want to use.\n\n"
                 "This can be done with the Footprint Library Manager."
                );

        title = _( "Footprint Library Table Needs Configuration" );
    }

    msg << "\n\n";
    msg << _( "Refer to the Pcbnew and Cvpcb manual sections "
              "\"Managing Footprint Libraries\" "
              "for more details." );

    showFootprintLibHelperDialog( aFrame, aIsWarning, title, msg, aManagerMenuId );
}
