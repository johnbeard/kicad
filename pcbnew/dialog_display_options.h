/////////////////////////////////////////////////////////////////////////////
// Name:        dialog_display_options.h
// Purpose:     
// Author:      jean-pierre Charras
// Modified by: 
// Created:     25/02/2006 18:43:49
// RCS-ID:      
// Copyright:   License GNU
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 25/02/2006 18:43:49

#ifndef _DIALOG_DISPLAY_OPTIONS_H_
#define _DIALOG_DISPLAY_OPTIONS_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "dialog_display_options.h"
#endif

/*!
 * Includes
 */

////@begin includes
#include "wx/valgen.h"
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_STYLE wxCAPTION|wxSYSTEM_MENU|wxCLOSE_BOX|MAYBE_RESIZE_BORDER
#define SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_TITLE _("Display Options")
#define SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_IDNAME ID_DIALOG
#define SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_SIZE wxSize(400, 300)
#define SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_POSITION wxDefaultPosition
#define ID_RADIOBOX_OPT_TRACK 10001
#define ID_RADIOBOX_SHOWCLR 10002
#define ID_RADIOBOX_SHOW_VIAS 10003
#define ID_RADIOBOX_MODTXT 10004
#define ID_RADIOBOX_MOD_EDGES 10005
#define ID_RADIOBOX_SHOWPADS 10007
#define ID_CHECKBOX_PAD_CLR 10008
#define ID_CHECKBOX_PADNUM 10009
#define ID_CHECKBOX_PADNC 10010
#define ID_RADIOBOX_SHOW_OTHERS 10011
#define ID_RADIOBOX_PAGE_LIMITS 10006
////@end control identifiers

/*!
 * Compatibility
 */

#ifndef wxCLOSE_BOX
#define wxCLOSE_BOX 0x1000
#endif

/*!
 * WinEDA_DisplayOptionsDialog class declaration
 */

class WinEDA_DisplayOptionsDialog: public wxDialog
{    
    DECLARE_DYNAMIC_CLASS( WinEDA_DisplayOptionsDialog )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WinEDA_DisplayOptionsDialog( );
    WinEDA_DisplayOptionsDialog( WinEDA_BasePcbFrame* parent, wxWindowID id = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_IDNAME, const wxString& caption = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_TITLE, const wxPoint& pos = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_POSITION, const wxSize& size = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_SIZE, long style = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_IDNAME, const wxString& caption = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_TITLE, const wxPoint& pos = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_POSITION, const wxSize& size = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_SIZE, long style = SYMBOL_WINEDA_DISPLAYOPTIONSDIALOG_STYLE );

    /// Creates the controls and sizers
    void CreateControls();

////@begin WinEDA_DisplayOptionsDialog event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_OK
    void OnOkClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

////@end WinEDA_DisplayOptionsDialog event handler declarations

////@begin WinEDA_DisplayOptionsDialog member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WinEDA_DisplayOptionsDialog member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
	void AcceptPcbOptions(wxCommandEvent& event);

////@begin WinEDA_DisplayOptionsDialog member variables
    wxRadioBox* m_OptDisplayTracks;
    wxRadioBox* m_OptDisplayTracksClearance;
    wxRadioBox* m_OptDisplayViaHole;
    wxRadioBox* m_OptDisplayModTexts;
    wxRadioBox* m_OptDisplayModEdges;
    wxRadioBox* m_OptDisplayPads;
    wxCheckBox* m_OptDisplayPadClearence;
    wxCheckBox* m_OptDisplayPadNumber;
    wxCheckBox* m_OptDisplayPadNoConn;
    wxRadioBox* m_OptDisplayDrawings;
    wxRadioBox* m_Show_Page_Limits;
////@end WinEDA_DisplayOptionsDialog member variables
	WinEDA_BasePcbFrame * m_Parent;
};

#endif
    // _DIALOG_DISPLAY_OPTIONS_H_
