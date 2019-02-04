/**
 * @file dialog_symbol_remap.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2017-2018 KiCad Developers, see AUTHORS.txt for contributors.
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

#include <macros.h>
#include <pgm_base.h>
#include <kiface_i.h>
#include <project.h>
#include <confirm.h>
#include <reporter.h>
#include <wildcards_and_files_ext.h>
#include <wx_html_report_panel.h>

#include <class_library.h>
#include <sch_io_mgr.h>
#include <sch_sheet.h>
#include <sch_component.h>
#include <sch_screen.h>
#include <sch_edit_frame.h>
#include <symbol_lib_table.h>
#include <env_paths.h>

#include <dialog_symbol_remap.h>


DIALOG_SYMBOL_REMAP::DIALOG_SYMBOL_REMAP( SCH_EDIT_FRAME* aParent ) :
    DIALOG_SYMBOL_REMAP_BASE( aParent )
{
    m_remapped = false;

    if( !wxFileName::IsDirWritable( Prj().GetProjectPath() ) )
    {
        DisplayInfoMessage( this, _( "Remapping is not possible because you do not have "
                                     "write privileges to the project folder \"%s\"." ) );

        // Disable the remap button.
        m_remapped = true;
    }

    wxString text;

    text = _( "This schematic currently uses the project symbol library list look up method "
              "for loading library symbols.  KiCad will attempt to map the existing symbols "
              "to use the new symbol library table.  Remapping will change some project files "
              "and schematics may not be compatible with older versions of KiCad.  All files "
              "that are changed will be backed up to the \"remap_backup\" folder in the project "
              "folder should you need to revert any changes.  If you choose to skip this step, "
              "you will be responsible for manually remapping the symbols." );

    m_htmlCtrl->AppendToPage( text );
}


void DIALOG_SYMBOL_REMAP::OnRemapSymbols( wxCommandEvent& aEvent )
{
    SCH_EDIT_FRAME* parent = dynamic_cast< SCH_EDIT_FRAME* >( GetParent() );

    wxCHECK_RET( parent != nullptr, "Parent window is not type SCH_EDIT_FRAME." );

    wxBusyCursor busy;

    if( !backupProject( m_messagePanel->Reporter() ) )
        return;

    // Ignore the never show rescue setting for one last rescue of legacy symbol
    // libraries before remapping to the symbol library table.  This ensures the
    // best remapping results.
    parent->RescueLegacyProject( false );

    // The schematic is fully loaded, any legacy library symbols have been rescued.  Now
    // check to see if the schematic has not been converted to the symbol library table
    // method for looking up symbols.
    wxFileName prjSymLibTableFileName( Prj().GetProjectPath(),
                                       SYMBOL_LIB_TABLE::GetSymbolLibTableFileName() );

    // Delete the existing project symbol library table.
    if( prjSymLibTableFileName.FileExists() )
    {
        wxRemoveFile( prjSymLibTableFileName.GetFullPath() );
    }

    createProjectSymbolLibTable( m_messagePanel->Reporter() );
    Prj().SetElem( PROJECT::ELEM_SYMBOL_LIB_TABLE, NULL );
    Prj().SchSymbolLibTable();

    remapSymbolsToLibTable( m_messagePanel->Reporter() );

    // Remove all of the libraries from the legacy library list.
    wxString paths;
    wxArrayString libNames;

    PART_LIBS::LibNamesAndPaths( &Prj(), true, &paths, &libNames );

    // Reload the the cache symbol library.
    Prj().SetElem( PROJECT::ELEM_SCH_PART_LIBS, NULL );
    Prj().SchLibs();

    Raise();
    m_remapped = true;
}


size_t DIALOG_SYMBOL_REMAP::getLibsNotInGlobalSymbolLibTable( std::vector< PART_LIB* >& aLibs )
{
    PART_LIBS* libs = Prj().SchLibs();

    for( PART_LIBS_BASE::iterator it = libs->begin(); it != libs->end(); ++it )
    {
        // Ignore the cache library.
        if( it->IsCache() )
            continue;

        // Check for the obvious library name.
        wxString libFileName = it->GetFullFileName();

        if( !SYMBOL_LIB_TABLE::GetGlobalLibTable().FindRowByURI( libFileName ) )
            aLibs.push_back( &(*it) );
    }

    return aLibs.size();
}


void DIALOG_SYMBOL_REMAP::createProjectSymbolLibTable( REPORTER& aReporter )
{
    wxString msg;
    std::vector< PART_LIB* > libs;

    if( getLibsNotInGlobalSymbolLibTable( libs ) )
    {
        SYMBOL_LIB_TABLE prjLibTable;
        std::vector< wxString > libNames = SYMBOL_LIB_TABLE::GetGlobalLibTable().GetLogicalLibs();

        for( auto lib : libs )
        {
            wxString libName = lib->GetName();
            int libNameInc = 1;
            int libNameLen = libName.Length();

            // Spaces in the file name will break the symbol name because they are not
            // quoted in the symbol library file format.
            libName.Replace( " ", "-" );

            // Don't create duplicate table entries.
            while( std::find( libNames.begin(), libNames.end(), libName ) != libNames.end() )
            {
                libName = libName.Left( libNameLen );
                libName << libNameInc;
                libNameInc++;
            }

            wxString pluginType = SCH_IO_MGR::ShowType( SCH_IO_MGR::SCH_LEGACY );
            wxFileName fn = lib->GetFullFileName();

            // Use environment variable substitution where possible.  This is based solely
            // on the internal user environment variable list.  Checking against all of the
            // system wide environment variables is probably not a good idea.
            wxString fullFileName = NormalizePath( fn, &Pgm().GetLocalEnvVariables(), &Prj() );

            // Fall back to the absolute library path.
            if( fullFileName.IsEmpty() )
                fullFileName = lib->GetFullFileName();

            wxFileName tmpFn = fullFileName;

            // Don't add symbol libraries that do not exist.
            if( tmpFn.Normalize() && tmpFn.FileExists() )
            {
                msg.Printf( _( "Adding library \"%s\", file \"%s\" to project symbol library table." ),
                            libName, fullFileName );
                aReporter.Report( msg, REPORTER::RPT_INFO );

                prjLibTable.InsertRow( std::make_unique<SYMBOL_LIB_TABLE_ROW>(
                        libName, fullFileName, pluginType ) );
            }
            else
            {
                msg.Printf( _( "Library \"%s\" not found." ), fullFileName );
                aReporter.Report( msg, REPORTER::RPT_WARNING );
            }
        }

        // Don't save empty project symbol library table.
        if( !prjLibTable.IsEmpty() )
        {
            wxFileName fn( Prj().GetProjectPath(), SYMBOL_LIB_TABLE::GetSymbolLibTableFileName() );

            try
            {
                FILE_OUTPUTFORMATTER formatter( fn.GetFullPath() );
                prjLibTable.Format( &formatter, 0 );
            }
            catch( const IO_ERROR& ioe )
            {
                msg.Printf( _( "Failed to write project symbol library table. Error:\n  %s" ),
                            ioe.What() );
                aReporter.ReportTail( msg, REPORTER::RPT_ERROR );
            }

            aReporter.ReportTail( _( "Created project symbol library table.\n" ), REPORTER::RPT_INFO );
        }
    }
}


void DIALOG_SYMBOL_REMAP::remapSymbolsToLibTable( REPORTER& aReporter )
{
    wxString msg;
    SCH_SCREENS schematic;
    SCH_COMPONENT* symbol;
    SCH_ITEM* item;
    SCH_ITEM* nextItem;
    SCH_SCREEN* screen;

    for( screen = schematic.GetFirst(); screen; screen = schematic.GetNext() )
    {
        for( item = screen->GetDrawItems(); item; item = nextItem )
        {
            nextItem = item->Next();

            if( item->Type() != SCH_COMPONENT_T )
                continue;

            symbol = dynamic_cast< SCH_COMPONENT* >( item );

            if( !remapSymbolToLibTable( symbol ) )
            {
                msg.Printf( _( "No symbol \"%s\" found in symbol library table." ),
                            symbol->GetLibId().GetLibItemName().wx_str() );
                aReporter.Report( msg, REPORTER::RPT_WARNING );
            }
            else
            {
                msg.Printf( _( "Symbol \"%s\" mapped to symbol library \"%s\"." ),
                            symbol->GetLibId().GetLibItemName().wx_str(),
                            symbol->GetLibId().GetLibNickname().wx_str() );
                aReporter.Report( msg, REPORTER::RPT_ACTION );
                screen->SetModify();
            }
        }
    }

    aReporter.Report( _( "Symbol library table mapping complete!" ), REPORTER::RPT_INFO );
    schematic.UpdateSymbolLinks( true );
}


bool DIALOG_SYMBOL_REMAP::remapSymbolToLibTable( SCH_COMPONENT* aSymbol )
{
    wxCHECK_MSG( aSymbol != NULL, false, "Null pointer passed to remapSymbolToLibTable." );
    wxCHECK_MSG( aSymbol->GetLibId().GetLibNickname().empty(), false,
                 "Cannot remap symbol that is already mapped." );
    wxCHECK_MSG( !aSymbol->GetLibId().GetLibItemName().empty(), false,
                 "The symbol LIB_ID name is empty." );

    PART_LIBS* libs = Prj().SchLibs();

    for( PART_LIBS_BASE::iterator it = libs->begin(); it != libs->end(); ++it )
    {
        // Ignore the cache library.
        if( it->IsCache() )
            continue;

        LIB_ALIAS* alias = it->FindAlias( aSymbol->GetLibId().GetLibItemName().wx_str() );

        // Found in the same library as the old look up method assuming the user didn't
        // change the libraries or library ordering since the last time the schematic was
        // loaded.
        if( alias )
        {
            // Find the same library in the symbol library table using the full path and file name.
            wxString libFileName = it->GetFullFileName();

            const LIB_TABLE_ROW* row = Prj().SchSymbolLibTable()->FindRowByURI( libFileName );

            if( row )
            {
                LIB_ID id = aSymbol->GetLibId();

                id.SetLibNickname( row->GetNickName() );

                // Don't resolve symbol library links now.
                aSymbol->SetLibId( id, nullptr, nullptr );
                return true;
            }
        }
    }

    return false;
}


bool DIALOG_SYMBOL_REMAP::backupProject( REPORTER& aReporter )
{
    static wxString backupFolder = "rescue-backup";

    wxString tmp;
    wxString errorMsg;
    wxFileName srcFileName;
    wxFileName destFileName;
    wxFileName backupPath;
    SCH_SCREENS schematic;

    // Copy backup files to different folder so as not to pollute the project folder.
    destFileName.SetPath( Prj().GetProjectPath() );
    destFileName.AppendDir( backupFolder );
    backupPath = destFileName;

    if( !destFileName.DirExists() )
    {
        if( !destFileName.Mkdir() )
        {
            errorMsg.Printf( _( "Cannot create project remap back up folder \"%s\"." ),
                             destFileName.GetPath() );

            wxMessageDialog dlg( this, errorMsg, _( "Backup Error" ),
                                 wxYES_NO | wxCENTRE | wxRESIZE_BORDER | wxICON_QUESTION );
            dlg.SetYesNoLabels( wxMessageDialog::ButtonLabel( _( "Continue with Rescue" ) ),
                                wxMessageDialog::ButtonLabel( _( "Abort Rescue" ) ) );

            if( dlg.ShowModal() == wxID_NO )
                return false;
        }
    }

    // Time stamp to append to file name in case multiple remappings are performed.
    wxString timeStamp = wxDateTime::Now().Format( "-%Y-%m-%d-%H-%M-%S" );

    // Back up symbol library table.
    srcFileName.SetPath( Prj().GetProjectPath() );
    srcFileName.SetName( SYMBOL_LIB_TABLE::GetSymbolLibTableFileName() );
    destFileName = srcFileName;
    destFileName.AppendDir( backupFolder );
    destFileName.SetName( destFileName.GetName() + timeStamp );

    tmp.Printf( _( "Backing up file \"%s\" to file \"%s\"." ),
                srcFileName.GetFullPath(), destFileName.GetFullPath() );
    aReporter.Report( tmp, REPORTER::RPT_INFO );

    if( wxFileName::Exists( srcFileName.GetFullPath() )
      && !wxCopyFile( srcFileName.GetFullPath(), destFileName.GetFullPath() ) )
    {
        tmp.Printf( _( "Failed to back up file \"%s\".\n" ), srcFileName.GetFullPath() );
        errorMsg += tmp;
    }

    // Back up the schematic files.
    for( SCH_SCREEN* screen = schematic.GetFirst(); screen; screen = schematic.GetNext() )
    {
        destFileName = screen->GetFileName();
        destFileName.SetName( destFileName.GetName() + timeStamp );

        // Check for nest hierarchical schematic paths.
        if( destFileName.GetPath() != backupPath.GetPath() )
        {
            destFileName.SetPath( backupPath.GetPath() );

            wxArrayString srcDirs = wxFileName( screen->GetFileName() ).GetDirs();
            wxArrayString destDirs = wxFileName( Prj().GetProjectPath() ).GetDirs();

            for( size_t i = destDirs.GetCount(); i < srcDirs.GetCount(); i++ )
                destFileName.AppendDir( srcDirs[i] );
        }
        else
        {
            destFileName.AppendDir( backupFolder );
        }

        tmp.Printf( _( "Backing up file \"%s\" to file \"%s\"." ),
                    screen->GetFileName(), destFileName.GetFullPath() );
        aReporter.Report( tmp, REPORTER::RPT_INFO );

        if( !destFileName.DirExists() && !destFileName.Mkdir( wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL ) )
        {
            tmp.Printf( _( "Failed to create backup folder \"%s\"\n" ), destFileName.GetPath() );
            errorMsg += tmp;
            continue;
        }

        if( wxFileName::Exists( screen->GetFileName() )
          && !wxCopyFile( screen->GetFileName(), destFileName.GetFullPath() ) )
        {
            tmp.Printf( _( "Failed to back up file \"%s\".\n" ), screen->GetFileName() );
            errorMsg += tmp;
        }
    }

    // Back up the project file.
    destFileName = Prj().GetProjectFullName();
    destFileName.SetName( destFileName.GetName() + timeStamp );
    destFileName.AppendDir( backupFolder );

    tmp.Printf( _( "Backing up file \"%s\" to file \"%s\"." ),
                   Prj().GetProjectFullName(), destFileName.GetFullPath() );
    aReporter.Report( tmp, REPORTER::RPT_INFO );

    if( wxFileName::Exists( Prj().GetProjectFullName() )
      && !wxCopyFile( Prj().GetProjectFullName(), destFileName.GetFullPath() ) )
    {
        tmp.Printf( _( "Failed to back up file \"%s\".\n" ), Prj().GetProjectFullName() );
        errorMsg += tmp;
    }

    // Back up the cache library.
    srcFileName.SetPath( Prj().GetProjectPath() );
    srcFileName.SetName( Prj().GetProjectName() + "-cache" );
    srcFileName.SetExt( SchematicLibraryFileExtension );

    destFileName = srcFileName;
    destFileName.SetName( destFileName.GetName() + timeStamp );
    destFileName.AppendDir( backupFolder );

    tmp.Printf( _( "Backing up file \"%s\" to file \"%s\"." ),
                srcFileName.GetFullPath(), destFileName.GetFullPath() );
    aReporter.Report( tmp, REPORTER::RPT_INFO );

    if( srcFileName.Exists()
      && !wxCopyFile( srcFileName.GetFullPath(), destFileName.GetFullPath() ) )
    {
        tmp.Printf( _( "Failed to back up file \"%s\".\n" ), srcFileName.GetFullPath() );
        errorMsg += tmp;
    }

    // Back up the rescue symbol library if it exists.
    srcFileName.SetName( Prj().GetProjectName() + "-rescue" );
    destFileName.SetName( srcFileName.GetName() + timeStamp );

    tmp.Printf( _( "Backing up file \"%s\" to file \"%s\"." ),
                srcFileName.GetFullPath(), destFileName.GetFullPath() );
    aReporter.Report( tmp, REPORTER::RPT_INFO );

    if( srcFileName.Exists()
      && !wxCopyFile( srcFileName.GetFullPath(), destFileName.GetFullPath() ) )
    {
        tmp.Printf( _( "Failed to back up file \"%s\".\n" ), srcFileName.GetFullPath() );
        errorMsg += tmp;
    }

    // Back up the rescue symbol library document file if it exists.
    srcFileName.SetExt( "dcm" );
    destFileName.SetExt( srcFileName.GetExt() );

    tmp.Printf( _( "Backing up file \"%s\" to file \"%s\"." ),
                srcFileName.GetFullPath(), destFileName.GetFullPath() );
    aReporter.Report( tmp, REPORTER::RPT_INFO );

    if( srcFileName.Exists()
      && !wxCopyFile( srcFileName.GetFullPath(), destFileName.GetFullPath() ) )
    {
        tmp.Printf( _( "Failed to back up file \"%s\".\n" ), srcFileName.GetFullPath() );
        errorMsg += tmp;
    }

    if( !errorMsg.IsEmpty() )
    {
        wxMessageDialog dlg( this, _( "Some of the project files could not be backed up." ),
                             _( "Backup Error" ),
                             wxYES_NO | wxCENTRE | wxRESIZE_BORDER | wxICON_QUESTION );
        errorMsg.Trim();
        dlg.SetExtendedMessage( errorMsg );
        dlg.SetYesNoLabels( wxMessageDialog::ButtonLabel( _( "Continue with Rescue" ) ),
                            wxMessageDialog::ButtonLabel( _( "Abort Rescue" ) ) );

        if( dlg.ShowModal() == wxID_NO )
            return false;
    }

    return true;
}


void DIALOG_SYMBOL_REMAP::OnUpdateUIRemapButton( wxUpdateUIEvent& aEvent )
{
    aEvent.Enable( !m_remapped );
}
