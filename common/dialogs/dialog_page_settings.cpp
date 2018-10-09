/*
 * This program source code file is part of KICAD, a free EDA CAD application.
 *
 * Copyright (C) 1992-2018 Kicad Developers, see AUTHORS.txt for contributors.
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

/**
 * @file dialog_page_settings.cpp
 */

#include <fctsys.h>
#include <macros.h>              // DIM()
#include <common.h>
#include <project.h>
#include <confirm.h>
#include <gr_basic.h>
#include <base_struct.h>
#include <class_drawpanel.h>
#include <title_block.h>
#include <draw_frame.h>
#include <worksheet_shape_builder.h>
#include <base_screen.h>
#include <wildcards_and_files_ext.h>
#include <widgets/unit_binder.h>

#include <wx/valgen.h>
#include <wx/tokenzr.h>

#include <worksheet.h>
#include <dialog_page_settings.h>

#define MAX_PAGE_EXAMPLE_SIZE 200


// List of page formats.
// they are prefixed by "_HKI" (already in use for hotkeys) instead of "_",
// because we need both the translated and the not translated version.
// when displayed in dialog we should explicitly call wxGetTranslation()
// to show the translated version.
// See hotkeys_basic.h for more info
#define _HKI( x ) wxT( x )
static const wxString pageFmts[] =
{
    _HKI("A4 210x297mm"),
    _HKI("A3 297x420mm"),
    _HKI("A2 420x594mm"),
    _HKI("A1 594x841mm"),
    _HKI("A0 841x1189mm"),
    _HKI("A 8.5x11in"),
    _HKI("B 11x17in"),
    _HKI("C 17x22in"),
    _HKI("D 22x34in"),
    _HKI("E 34x44in"),
    _HKI("USLetter 8.5x11in"),      // USLetter without space is correct
    _HKI("USLegal 8.5x14in"),       // USLegal without space is correct
    _HKI("USLedger 11x17in"),       // USLedger without space is correct
    _HKI("User (Custom)"),          // size defined by user. The string must contain "Custom"
                                    // to be recognized in code
};


DIALOG_PAGES_SETTINGS::DIALOG_PAGES_SETTINGS( EDA_DRAW_FRAME* parent, wxSize aMaxUserSizeMils ) :
    DIALOG_PAGES_SETTINGS_BASE( parent ),
    m_initialized( false ),
    m_showMultiSheetOptions( parent->CanHaveMultipleSheets() )
{
    m_parent   = parent;
    m_screen   = m_parent->GetScreen();
    m_projectPath = Prj().GetProjectPath();
    m_page_bitmap = NULL;
    m_maxPageSizeMils = aMaxUserSizeMils;
    m_tb = m_parent->GetTitleBlock();
    m_customFmt = false;
    m_localPrjConfigChanged = false;
    m_pagelayout = NULL;

    m_iuPerMils = m_screen->MilsToIuScalar();
    const int min_page_size_iu = MIN_PAGE_SIZE * m_iuPerMils;

    m_customSizeX = std::make_unique<UNIT_BINDER>( parent, m_userSizeXLabel, m_userSizeXCtrl,
            m_userSizeXUnits, false,
            min_page_size_iu, aMaxUserSizeMils.x * m_iuPerMils );
    m_customSizeY = std::make_unique<UNIT_BINDER>( parent, m_userSizeYLabel, m_userSizeYCtrl,
            m_userSizeYUnits, false,
            min_page_size_iu, aMaxUserSizeMils.y * m_iuPerMils );

    m_PickDate->SetValue( wxDateTime::Now() );

    initDialog();

    GetSizer()->SetSizeHints( this );
    Centre();
}


DIALOG_PAGES_SETTINGS::~DIALOG_PAGES_SETTINGS()
{
    delete m_page_bitmap;
    delete m_pagelayout;
}


void DIALOG_PAGES_SETTINGS::initDialog()
{
    wxString    msg;

    // initialize page format choice box and page format list.
    // The first shows translated strings, the second contains not translated strings
    m_paperSizeComboBox->Clear();

    for( unsigned ii = 0; ii < DIM(pageFmts); ii++ )
    {
        m_pageFmt.Add( pageFmts[ii] );
        m_paperSizeComboBox->Append( wxGetTranslation( pageFmts[ii] ) );
    }

    // initialize the page layout descr filename
    SetWksFileName( BASE_SCREEN::m_PageLayoutDescrFileName );

    if( m_showMultiSheetOptions )
    {
        // Init display value for schematic sub-sheet number
        wxString format = m_TextSheetCount->GetLabel();
        msg.Printf( format, m_screen->m_NumberOfScreens );
        m_TextSheetCount->SetLabel( msg );

        format = m_TextSheetNumber->GetLabel();
        msg.Printf( format, m_screen->m_ScreenNumber );
        m_TextSheetNumber->SetLabel( msg );
    }
    else
    {
        m_TextSheetCount->Show( false );
        m_TextSheetNumber->Show( false );
    }

    m_pageInfo = m_parent->GetPageSettings();
    SetCurrentPageSizeSelection( m_pageInfo.GetType() );
    m_orientationComboBox->SetSelection( m_pageInfo.IsPortrait() );

    // only a click fires the "selection changed" event, so have to fabricate this check
    wxCommandEvent dummy;
    OnPaperSizeChoice( dummy );

    if( m_customFmt )
    {
        m_customSizeX->SetValue( m_pageInfo.GetWidthMils() * m_iuPerMils );
        m_customSizeY->SetValue( m_pageInfo.GetHeightMils() * m_iuPerMils );
    }
    else
    {
        m_customSizeX->SetValue( m_pageInfo.GetCustomWidthMils() * m_iuPerMils );
        m_customSizeY->SetValue( m_pageInfo.GetCustomHeightMils() * m_iuPerMils );
    }

    m_TextRevision->SetValue( m_tb.GetRevision() );
    m_TextDate->SetValue( m_tb.GetDate() );
    m_TextTitle->SetValue( m_tb.GetTitle() );
    m_TextCompany->SetValue( m_tb.GetCompany() );
    m_TextComment1->SetValue( m_tb.GetComment1() );
    m_TextComment2->SetValue( m_tb.GetComment2() );
    m_TextComment3->SetValue( m_tb.GetComment3() );
    m_TextComment4->SetValue( m_tb.GetComment4() );

    if( !m_showMultiSheetOptions )
    {
        // these options have meaning only if there are multiple sheets
        // (for example in Eeschema). Disable them otherwise.
        m_RevisionExport->Show( false );
        m_DateExport->Show( false );
        m_TitleExport->Show( false );
        m_CompanyExport->Show( false );
        m_Comment1Export->Show( false );
        m_Comment2Export->Show( false );
        m_Comment3Export->Show( false );
        m_Comment4Export->Show( false );
    }

    GetPageLayoutInfoFromDialog();
    UpdatePageLayoutExample();

    // Make the OK button the default.
    m_sdbSizerOK->SetDefault();
    m_initialized = true;
}


void DIALOG_PAGES_SETTINGS::OnOkClick( wxCommandEvent& event )
{
    if( SavePageSettings() )
    {
        m_screen->SetModify();
        m_parent->GetCanvas()->Refresh();

        if( LocalPrjConfigChanged() )
            m_parent->SaveProjectSettings( false );
    }

    event.Skip();
}


void DIALOG_PAGES_SETTINGS::OnPaperSizeChoice( wxCommandEvent& event )
{
    int idx = m_paperSizeComboBox->GetSelection();

    if( idx < 0 )
        idx = 0;

    const wxString paperType = m_pageFmt[idx];

    if( paperType.Contains( PAGE_INFO::Custom ) )
    {
        m_orientationComboBox->Enable( false );
        m_customSizeX->Enable( true );
        m_customSizeY->Enable( true );
        m_customFmt = true;
    }
    else
    {
        m_orientationComboBox->Enable( true );

#if 0
        // ForcePortrait() does not exist, but could be useful.
        // so I leave these lines, which could be seen as a todo feature
        if( paperType.ForcePortrait() )
        {
            m_orientationComboBox->SetStringSelection( _( "Portrait" ) );
            m_orientationComboBox->Enable( false );
        }
#endif
        m_customSizeX->Enable( false );
        m_customSizeY->Enable( false );
        m_customFmt = false;
    }

    GetPageLayoutInfoFromDialog();
    UpdatePageLayoutExample();
}


void DIALOG_PAGES_SETTINGS::OnUserPageSizeXTextUpdated( wxCommandEvent& event )
{
    if( m_initialized )
    {
        GetPageLayoutInfoFromDialog();
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnUserPageSizeYTextUpdated( wxCommandEvent& event )
{
    if( m_initialized )
    {
        GetPageLayoutInfoFromDialog();
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnPageOrientationChoice( wxCommandEvent& event )
{
    if( m_initialized )
    {
        GetPageLayoutInfoFromDialog();
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnRevisionTextUpdated( wxCommandEvent& event )
{
    if( m_initialized && m_TextRevision->IsModified() )
    {
        GetPageLayoutInfoFromDialog();
        m_tb.SetRevision( m_TextRevision->GetValue() );
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnDateTextUpdated( wxCommandEvent& event )
{
    if( m_initialized && m_TextDate->IsModified() )
    {
        GetPageLayoutInfoFromDialog();
        m_tb.SetDate( m_TextDate->GetValue() );
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnTitleTextUpdated( wxCommandEvent& event )
{
    if( m_initialized && m_TextTitle->IsModified() )
    {
        GetPageLayoutInfoFromDialog();
        m_tb.SetTitle( m_TextTitle->GetValue() );
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnCompanyTextUpdated( wxCommandEvent& event )
{
    if( m_initialized && m_TextCompany->IsModified() )
    {
        GetPageLayoutInfoFromDialog();
        m_tb.SetCompany( m_TextCompany->GetValue() );
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnComment1TextUpdated( wxCommandEvent& event )
{
    if( m_initialized && m_TextComment1->IsModified() )
    {
        GetPageLayoutInfoFromDialog();
        m_tb.SetComment1( m_TextComment1->GetValue() );
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnComment2TextUpdated( wxCommandEvent& event )
{
    if( m_initialized && m_TextComment2->IsModified() )
    {
        GetPageLayoutInfoFromDialog();
        m_tb.SetComment2( m_TextComment2->GetValue() );
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnComment3TextUpdated( wxCommandEvent& event )
{
    if( m_initialized && m_TextComment3->IsModified() )
    {
        GetPageLayoutInfoFromDialog();
        m_tb.SetComment3( m_TextComment3->GetValue() );
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnComment4TextUpdated( wxCommandEvent& event )
{
    if( m_initialized && m_TextComment4->IsModified() )
    {
        GetPageLayoutInfoFromDialog();
        m_tb.SetComment4( m_TextComment4->GetValue() );
        UpdatePageLayoutExample();
    }
}


void DIALOG_PAGES_SETTINGS::OnDateApplyClick( wxCommandEvent& event )
{
    wxDateTime datetime = m_PickDate->GetValue();
    wxString date =
    // We can choose different formats. Only one must be uncommented
    //
    //  datetime.Format( wxLocale::GetInfo( wxLOCALE_SHORT_DATE_FMT ) );
    //  datetime.Format( wxLocale::GetInfo( wxLOCALE_LONG_DATE_FMT ) );
    //  datetime.Format( wxT("%Y-%b-%d") );
        datetime.FormatISODate();

    m_TextDate->SetValue( date );
}


bool DIALOG_PAGES_SETTINGS::SavePageSettings()
{
    bool retSuccess = false;

    wxString fileName = GetWksFileName();

    if( fileName != BASE_SCREEN::m_PageLayoutDescrFileName )
    {
        wxString fullFileName = WORKSHEET_LAYOUT::MakeFullFileName( fileName, m_projectPath );

        if( !fullFileName.IsEmpty() && !wxFileExists( fullFileName ) )
        {
            wxString msg;
            msg.Printf( _( "Page layout description file \"%s\" not found." ),
                        GetChars( fullFileName ) );
            wxMessageBox( msg );
            return false;
        }

        BASE_SCREEN::m_PageLayoutDescrFileName = fileName;
        WORKSHEET_LAYOUT& pglayout = WORKSHEET_LAYOUT::GetTheInstance();
        pglayout.SetPageLayout( fullFileName );
        m_localPrjConfigChanged = true;
    }

    int idx = std::max( m_paperSizeComboBox->GetSelection(), 0 );
    const wxString paperType = m_pageFmt[idx];

    if( paperType.Contains( PAGE_INFO::Custom ) )
    {
        GetCustomSizeMilsFromDialog();

        retSuccess = m_pageInfo.SetType( PAGE_INFO::Custom );

        if( retSuccess )
        {
            if( !m_customSizeX->Validate( true ) || !m_customSizeY->Validate( true ) )
                return false;

            PAGE_INFO::SetCustomWidthMils( m_layout_size.x );
            PAGE_INFO::SetCustomHeightMils( m_layout_size.y );

            m_pageInfo.SetWidthMils( m_layout_size.x );
            m_pageInfo.SetHeightMils( m_layout_size.y );
        }
    }
    else
    {
        // search for longest common string first, e.g. A4 before A
        if( paperType.Contains( PAGE_INFO::USLetter ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::USLetter );
        else if( paperType.Contains( PAGE_INFO::USLegal ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::USLegal );
        else if( paperType.Contains( PAGE_INFO::USLedger ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::USLedger );
        else if( paperType.Contains( PAGE_INFO::GERBER ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::GERBER );
        else if( paperType.Contains( PAGE_INFO::A4 ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::A4 );
        else if( paperType.Contains( PAGE_INFO::A3 ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::A3 );
        else if( paperType.Contains( PAGE_INFO::A2 ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::A2 );
        else if( paperType.Contains( PAGE_INFO::A1 ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::A1 );
        else if( paperType.Contains( PAGE_INFO::A0 ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::A0 );
        else if( paperType.Contains( PAGE_INFO::A ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::A );
        else if( paperType.Contains( PAGE_INFO::B ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::B );
        else if( paperType.Contains( PAGE_INFO::C ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::C );
        else if( paperType.Contains( PAGE_INFO::D ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::D );
        else if( paperType.Contains( PAGE_INFO::E ) )
            retSuccess = m_pageInfo.SetType( PAGE_INFO::E );

        if( retSuccess )
        {
            int choice = m_orientationComboBox->GetSelection();
            m_pageInfo.SetPortrait( choice != 0 );
        }
    }

    if( !retSuccess )
    {
        wxASSERT_MSG( false, _( "the translation for paper size must preserve original spellings" ) );
        m_pageInfo.SetType( PAGE_INFO::A4 );
    }

    m_parent->SetPageSettings( m_pageInfo );

    m_tb.SetRevision( m_TextRevision->GetValue() );
    m_tb.SetDate(     m_TextDate->GetValue() );
    m_tb.SetCompany(  m_TextCompany->GetValue() );
    m_tb.SetTitle(    m_TextTitle->GetValue() );
    m_tb.SetComment1( m_TextComment1->GetValue() );
    m_tb.SetComment2( m_TextComment2->GetValue() );
    m_tb.SetComment3( m_TextComment3->GetValue() );
    m_tb.SetComment4( m_TextComment4->GetValue() );


    TITLE_BLOCK_EXPORT_OPTIONS tb_export_opts;

    tb_export_opts.SetShouldExport( TITLE_BLOCK::FIELD::REVISION,
        m_RevisionExport->IsChecked());

    tb_export_opts.SetShouldExport( TITLE_BLOCK::FIELD::DATE,
        m_DateExport->IsChecked());

    tb_export_opts.SetShouldExport( TITLE_BLOCK::FIELD::COMPANY,
        m_CompanyExport->IsChecked());

    tb_export_opts.SetShouldExport( TITLE_BLOCK::FIELD::TITLE,
        m_TitleExport->IsChecked());

    tb_export_opts.SetShouldExport( TITLE_BLOCK::FIELD::COMMENT, 0,
        m_Comment1Export->IsChecked());

    tb_export_opts.SetShouldExport( TITLE_BLOCK::FIELD::COMMENT, 1,
        m_Comment2Export->IsChecked());

    tb_export_opts.SetShouldExport( TITLE_BLOCK::FIELD::COMMENT, 2,
        m_Comment3Export->IsChecked());

    tb_export_opts.SetShouldExport( TITLE_BLOCK::FIELD::COMMENT, 3,
        m_Comment4Export->IsChecked());


    m_parent->SetTitleBlock( m_tb, tb_export_opts );

    return true;
}


void DIALOG_PAGES_SETTINGS::SetCurrentPageSizeSelection( const wxString& aPaperSize )
{
    // search all the not translated label list containing our paper type
    for( unsigned i = 0; i < m_pageFmt.GetCount(); ++i )
    {
        // parse each label looking for aPaperSize within it
        wxStringTokenizer st( m_pageFmt[i] );

        while( st.HasMoreTokens() )
        {
            if( st.GetNextToken() == aPaperSize )
            {
                m_paperSizeComboBox->SetSelection( i );
                return;
            }
        }
    }
}


void DIALOG_PAGES_SETTINGS::UpdatePageLayoutExample()
{
    int lyWidth, lyHeight;

    wxSize clamped_layout_size( Clamp( MIN_PAGE_SIZE, m_layout_size.x, m_maxPageSizeMils.x ),
                                Clamp( MIN_PAGE_SIZE, m_layout_size.y, m_maxPageSizeMils.y ) );

    double lyRatio = clamped_layout_size.x < clamped_layout_size.y ?
                        (double) clamped_layout_size.y / clamped_layout_size.x :
                        (double) clamped_layout_size.x / clamped_layout_size.y;

    if( clamped_layout_size.x < clamped_layout_size.y )
    {
        lyHeight = MAX_PAGE_EXAMPLE_SIZE;
        lyWidth = KiROUND( (double) lyHeight / lyRatio );
    }
    else
    {
        lyWidth = MAX_PAGE_EXAMPLE_SIZE;
        lyHeight = KiROUND( (double) lyWidth / lyRatio );
    }

    if( m_page_bitmap )
    {
        m_PageLayoutExampleBitmap->SetBitmap( wxNullBitmap );
        delete m_page_bitmap;
    }

    m_page_bitmap = new wxBitmap( lyWidth + 1, lyHeight + 1 );

    if( m_page_bitmap->IsOk() )
    {
        // Calculate layout preview scale.
        int appScale = m_screen->MilsToIuScalar();

        double scaleW = (double) lyWidth  / clamped_layout_size.x / appScale;
        double scaleH = (double) lyHeight / clamped_layout_size.y / appScale;

        // Prepare DC.
        wxSize example_size( lyWidth + 1, lyHeight + 1 );
        wxMemoryDC memDC;
        memDC.SelectObject( *m_page_bitmap );
        memDC.SetClippingRegion( wxPoint( 0, 0 ), example_size );
        memDC.Clear();
        memDC.SetUserScale( scaleW, scaleH );

        // Get logical page size and margins.
        PAGE_INFO pageDUMMY;

        // Get page type
        int idx = m_paperSizeComboBox->GetSelection();

        if( idx < 0 )
            idx = 0;

        wxString pageFmtName = m_pageFmt[idx].BeforeFirst( ' ' );
        bool portrait = clamped_layout_size.x < clamped_layout_size.y;
        pageDUMMY.SetType( pageFmtName, portrait );
        if( m_customFmt )
        {
            pageDUMMY.SetWidthMils( clamped_layout_size.x );
            pageDUMMY.SetHeightMils( clamped_layout_size.y );
        }

        // Draw layout preview.
        wxString emptyString;
        GRResetPenAndBrush( &memDC );

        WORKSHEET_LAYOUT::SetAltInstance( m_pagelayout );
        DrawPageLayout( &memDC, NULL, pageDUMMY,
                        emptyString, emptyString,
                        m_tb, m_screen->m_NumberOfScreens,
                        m_screen->m_ScreenNumber, 1, appScale, DARKGRAY, RED );

        memDC.SelectObject( wxNullBitmap );
        m_PageLayoutExampleBitmap->SetBitmap( *m_page_bitmap );
        WORKSHEET_LAYOUT::SetAltInstance( NULL );

        // Refresh the dialog.
        Layout();
        Refresh();
    }
}


void DIALOG_PAGES_SETTINGS::GetPageLayoutInfoFromDialog()
{
    int idx = std::max( m_paperSizeComboBox->GetSelection(), 0 );
    const wxString paperType = m_pageFmt[idx];

    // here we assume translators will keep original paper size spellings
    if( paperType.Contains( PAGE_INFO::Custom ) )
    {
        GetCustomSizeMilsFromDialog();

        if( m_layout_size.x && m_layout_size.y )
        {
            if( m_layout_size.x < m_layout_size.y )
                m_orientationComboBox->SetStringSelection( _( "Portrait" ) );
            else
                m_orientationComboBox->SetStringSelection( _( "Landscape" ) );
        }
    }
    else
    {
        PAGE_INFO       pageInfo;   // SetType() later to lookup size

        static const wxChar* papers[] = {
            // longest common string first, since sequential search below
            PAGE_INFO::A4,
            PAGE_INFO::A3,
            PAGE_INFO::A2,
            PAGE_INFO::A1,
            PAGE_INFO::A0,
            PAGE_INFO::A,
            PAGE_INFO::B,
            PAGE_INFO::C,
            PAGE_INFO::D,
            PAGE_INFO::E,
            PAGE_INFO::USLetter,
            PAGE_INFO::USLegal,
            PAGE_INFO::USLedger,
        };

        unsigned i;

        for( i=0;  i < DIM( papers );  ++i )
        {
            if( paperType.Contains( papers[i] ) )
            {
                pageInfo.SetType( papers[i] );
                break;
            }
        }

        wxASSERT( i != DIM(papers) );   // dialog UI match the above list?

        m_layout_size = pageInfo.GetSizeMils();

        // swap sizes to match orientation
        bool isPortrait = (bool) m_orientationComboBox->GetSelection();

        if( ( isPortrait  && m_layout_size.x >= m_layout_size.y ) ||
            ( !isPortrait && m_layout_size.x <  m_layout_size.y ) )
        {
            m_layout_size.Set( m_layout_size.y, m_layout_size.x );
        }
    }
}


void DIALOG_PAGES_SETTINGS::GetCustomSizeMilsFromDialog()
{
    double customSizeX = (double) m_customSizeX->GetValue() / m_iuPerMils;
    double customSizeY = (double) m_customSizeY->GetValue() / m_iuPerMils;

    // Prepare to painless double -> int conversion.
    customSizeX = Clamp( double( INT_MIN ), customSizeX, double( INT_MAX ) );
    customSizeY = Clamp( double( INT_MIN ), customSizeY, double( INT_MAX ) );
    m_layout_size = wxSize( KiROUND( customSizeX ), KiROUND( customSizeY ) );
}


void DIALOG_PAGES_SETTINGS::OnWksFileSelection( wxCommandEvent& event )
{
    wxFileName fn = GetWksFileName();
    wxString name = GetWksFileName();
    wxString path;

    if( fn.IsAbsolute() )
    {
        path = fn.GetPath();
        name = fn.GetFullName();
    }
    else
    {
        path = m_projectPath;
    }

    // Display a file picker dialog
    wxFileDialog fileDialog( this, _( "Select Page Layout Description File" ),
                             path, name, PageLayoutDescrFileWildcard(),
                             wxFD_DEFAULT_STYLE | wxFD_FILE_MUST_EXIST );

    if( fileDialog.ShowModal() != wxID_OK )
        return;

    wxString fileName = fileDialog.GetPath();

    // Try to remove the path, if the path is the current working dir,
    // or the dir of kicad.pro (template), and use a relative path
    wxString shortFileName = WORKSHEET_LAYOUT::MakeShortFileName( fileName, m_projectPath );

    // For Win/Linux/macOS compatibility, a relative path is a good idea
    if( shortFileName != GetWksFileName() && shortFileName != fileName )
    {
        wxString msg = wxString::Format( _(
                "The page layout description file name has changed.\n"
                "Do you want to use the relative path:\n"
                "\"%s\"\n"
                "instead of\n"
                "\"%s\"?" ), GetChars( shortFileName ), GetChars( fileName ) );

        if( !IsOK( this, msg ) )
            shortFileName = fileName;
    }

    SetWksFileName( shortFileName );

    if( m_pagelayout == NULL )
        m_pagelayout = new WORKSHEET_LAYOUT;

    m_pagelayout->SetPageLayout( fileName );

    GetPageLayoutInfoFromDialog();
    UpdatePageLayoutExample();
}
