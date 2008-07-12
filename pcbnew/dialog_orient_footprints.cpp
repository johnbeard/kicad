/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_orient_footprints.cpp
// Purpose:     
// Author:      j-p Charras
// Modified by: 
// Created:     11/07/2008 13:32:12
// RCS-ID:      
// Copyright:   j-p Charras   
// Licence:     GPL
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 11/07/2008 13:32:12

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "fctsys.h"

#include "common.h"
#include "pcbnew.h"

#include "dialog_orient_footprints.h"

////@begin XPM images
////@end XPM images


int s_NewOrientation = 0;

/*!
 * dialog_orient_footprints type definition
 */

IMPLEMENT_DYNAMIC_CLASS( dialog_orient_footprints, wxDialog )


/*!
 * dialog_orient_footprints event table definition
 */

BEGIN_EVENT_TABLE( dialog_orient_footprints, wxDialog )

////@begin dialog_orient_footprints event table entries
    EVT_CLOSE( dialog_orient_footprints::OnCloseWindow )

    EVT_BUTTON( wxID_OK, dialog_orient_footprints::OnOkClick )

    EVT_BUTTON( wxID_CANCEL, dialog_orient_footprints::OnCancelClick )

////@end dialog_orient_footprints event table entries

END_EVENT_TABLE()


/*!
 * dialog_orient_footprints constructors
 */

dialog_orient_footprints::dialog_orient_footprints()
{
    Init();
}

dialog_orient_footprints::dialog_orient_footprints( WinEDA_PcbFrame* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
    m_Parent = parent;
    Init();
    Create(parent, id, caption, pos, size, style);
}


/*!
 * dialog_orient_footprints creator
 */

bool dialog_orient_footprints::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin dialog_orient_footprints creation
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);
    wxDialog::Create( parent, id, caption, pos, size, style );

    CreateControls();
    if (GetSizer())
    {
        GetSizer()->SetSizeHints(this);
    }
    Centre();
////@end dialog_orient_footprints creation
    return true;
}


/*!
 * dialog_orient_footprints destructor
 */

dialog_orient_footprints::~dialog_orient_footprints()
{
////@begin dialog_orient_footprints destruction
////@end dialog_orient_footprints destruction
}


/*!
 * Member initialisation
 */

void dialog_orient_footprints::Init()
{
////@begin dialog_orient_footprints member initialisation
    m_OrientationCtrl = NULL;
    m_FilterPattern = NULL;
    m_ApplyToLocked = NULL;
    m_CloseButton = NULL;
////@end dialog_orient_footprints member initialisation
}


/*!
 * Control creation for dialog_orient_footprints
 */

void dialog_orient_footprints::CreateControls()
{    
////@begin dialog_orient_footprints content construction
    // Generated by DialogBlocks, 11/07/2008 16:49:18 (unregistered)

    dialog_orient_footprints* itemDialog1 = this;

    wxBoxSizer* itemBoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    itemDialog1->SetSizer(itemBoxSizer2);

    wxBoxSizer* itemBoxSizer3 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer3, 1, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxStaticText* itemStaticText4 = new wxStaticText( itemDialog1, wxID_STATIC, _("Orientation:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText4, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);

    m_OrientationCtrl = new wxTextCtrl( itemDialog1, ID_TEXTCTRLROT_VALUE, _T(""), wxDefaultPosition, wxDefaultSize, 0 );
    if (dialog_orient_footprints::ShowToolTips())
        m_OrientationCtrl->SetToolTip(_("New orientation (0.1 degree resolution)"));
    itemBoxSizer3->Add(m_OrientationCtrl, 0, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    wxStaticText* itemStaticText6 = new wxStaticText( itemDialog1, wxID_STATIC, _("Filter:"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer3->Add(itemStaticText6, 0, wxGROW|wxLEFT|wxRIGHT|wxTOP, 5);

    m_FilterPattern = new wxTextCtrl( itemDialog1, ID_TEXTCTRL_FOOTPRINTS_FILTER, _("*"), wxDefaultPosition, wxDefaultSize, 0 );
    if (dialog_orient_footprints::ShowToolTips())
        m_FilterPattern->SetToolTip(_("Filter to select footprints by reference"));
    itemBoxSizer3->Add(m_FilterPattern, 1, wxGROW|wxLEFT|wxRIGHT|wxBOTTOM, 5);

    itemBoxSizer3->Add(5, 5, 0, wxGROW|wxALL, 5);

    m_ApplyToLocked = new wxCheckBox( itemDialog1, ID_CHECKBOX_APPLY_TO_LOCKED, _("Include Locked Footprints"), wxDefaultPosition, wxDefaultSize, 0 );
    m_ApplyToLocked->SetValue(false);
    if (dialog_orient_footprints::ShowToolTips())
        m_ApplyToLocked->SetToolTip(_("Force locked footprints to be modified"));
    itemBoxSizer3->Add(m_ApplyToLocked, 0, wxGROW|wxALL, 5);

    wxBoxSizer* itemBoxSizer10 = new wxBoxSizer(wxVERTICAL);
    itemBoxSizer2->Add(itemBoxSizer10, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

    wxButton* itemButton11 = new wxButton( itemDialog1, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
    itemBoxSizer10->Add(itemButton11, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    m_CloseButton = new wxButton( itemDialog1, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_CloseButton->SetDefault();
    itemBoxSizer10->Add(m_CloseButton, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

////@end dialog_orient_footprints content construction
    
    wxString txt;
    txt.Printf(wxT("%g"), (double) s_NewOrientation/10);
    m_OrientationCtrl->SetValue(txt);
    
    m_CloseButton->SetFocus( );
}


/*!
 * Should we show tooltips?
 */

bool dialog_orient_footprints::ShowToolTips()
{
    return true;
}

/*!
 * Get bitmap resources
 */

wxBitmap dialog_orient_footprints::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin dialog_orient_footprints bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end dialog_orient_footprints bitmap retrieval
}

/*!
 * Get icon resources
 */

wxIcon dialog_orient_footprints::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin dialog_orient_footprints icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end dialog_orient_footprints icon retrieval
}


/***********************************************/
void WinEDA_PcbFrame::OnOrientFootprints( void )
/***********************************************/
/** function OnOrientFootprints
 * install the dialog box for the comman Orient Footprints
 */
{
    dialog_orient_footprints dialogbox(this);
    dialogbox.ShowModal();
    dialogbox.Destroy();
}


/*******************************************************************/
void WinEDA_PcbFrame::ReOrientModules( const wxString& ModuleMask,
                                       int Orient, bool include_fixe )
/*******************************************************************/
/** function ReOrientModules
 * Set the orientation of footprints
 * @param ModuleMask = mask (wildcard allowed) selection
 * @param Orient = new orientation
 * @param include_fixe = true to orient locked footprints
 */
{
    MODULE*  Module;
    wxString line;
    bool redraw = false;

    line.Printf( _( "Ok to set footprints orientation to %g degrees ?" ), (double)Orient / 10 );
    if( !IsOK( this, line ) )
        return;

    Module = m_Pcb->m_Modules;
    for( ; Module != NULL; Module = (MODULE*) Module->Pnext )
    {
        if( Module->IsLocked() && !include_fixe )
            continue;

        if( WildCompareString( ModuleMask, Module->m_Reference->m_Text, FALSE ) )
        {
            GetScreen()->SetModify();
            redraw = true;
            Rotate_Module( NULL, Module, Orient, FALSE );
        }
    }
    
    if ( redraw )
        DrawPanel->Refresh();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
 */

void dialog_orient_footprints::OnOkClick( wxCommandEvent& event )
{
    double d_orient;
    wxString text = m_OrientationCtrl->GetValue();
    
    if ( ! text.ToDouble(&d_orient) )
    {
        DisplayError(this, _("Bad value for footprints orientation"));
        return;
    }

    s_NewOrientation = (int) round(d_orient * 10);
    if ( s_NewOrientation > 3600 )
        s_NewOrientation = 3600;
    if ( s_NewOrientation < -3600 )
        s_NewOrientation = -3600;

    text = m_FilterPattern->GetValue();
    m_Parent->ReOrientModules( text, s_NewOrientation, m_ApplyToLocked->IsChecked() );
    Close();
}


/*!
 * wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
 */

void dialog_orient_footprints::OnCancelClick( wxCommandEvent& event )
{
    Close();
}


/*!
 * wxEVT_CLOSE_WINDOW event handler for ID_DIALOG_ORIENT_FOOTPRINTS
 */

void dialog_orient_footprints::OnCloseWindow( wxCloseEvent& event )
{
	EndModal(1);
}

