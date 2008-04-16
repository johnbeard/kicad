/////////////////////////////////////////////////////////////////////////////
// Name:        annotate_dialog.h
// Purpose:
// Author:      jean-pierre Charras
// Modified by:
// Created:     16/04/2008 17:50:59
// RCS-ID:
// Copyright:   License GNU
// Licence:
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 16/04/2008 17:50:59

#ifndef _ANNOTATE_DIALOG_H_
#define _ANNOTATE_DIALOG_H_

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma interface "annotate_dialog.h"
#endif

/*!
 * Includes
 */

////@begin includes
////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
class wxBoxSizer;
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
#define ID_DIALOG 10000
#define ID_ENTIRE_SCHEMATIC 10002
#define ID_CURRENT_PAGE 10003
#define ID_RESET_ANNOTATION 10009
#define ID_SORT_BY_POSITION 10010
#define ID_SORT_BY_VALUE 10011
#define ID_CLEAR_ANNOTATION_CMP 10004
#define SYMBOL_WINEDA_ANNOTATEFRAME_STYLE wxDEFAULT_DIALOG_STYLE|MAYBE_RESIZE_BORDER
#define SYMBOL_WINEDA_ANNOTATEFRAME_TITLE _("EESchema Annotation")
#define SYMBOL_WINEDA_ANNOTATEFRAME_IDNAME ID_DIALOG
#define SYMBOL_WINEDA_ANNOTATEFRAME_SIZE wxSize(400, 300)
#define SYMBOL_WINEDA_ANNOTATEFRAME_POSITION wxDefaultPosition
////@end control identifiers


/*!
 * WinEDA_AnnotateFrame class declaration
 */

class WinEDA_AnnotateFrame: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( WinEDA_AnnotateFrame )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    WinEDA_AnnotateFrame();
    WinEDA_AnnotateFrame( WinEDA_SchematicFrame* parent, wxWindowID id = SYMBOL_WINEDA_ANNOTATEFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_ANNOTATEFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_ANNOTATEFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_ANNOTATEFRAME_SIZE, long style = SYMBOL_WINEDA_ANNOTATEFRAME_STYLE );

    /// Creation
    bool Create( wxWindow* parent, wxWindowID id = SYMBOL_WINEDA_ANNOTATEFRAME_IDNAME, const wxString& caption = SYMBOL_WINEDA_ANNOTATEFRAME_TITLE, const wxPoint& pos = SYMBOL_WINEDA_ANNOTATEFRAME_POSITION, const wxSize& size = SYMBOL_WINEDA_ANNOTATEFRAME_SIZE, long style = SYMBOL_WINEDA_ANNOTATEFRAME_STYLE );

    /// Destructor
    ~WinEDA_AnnotateFrame();

    /// Initialises member variables
    void Init();

    /// Creates the controls and sizers
    void CreateControls();

////@begin WinEDA_AnnotateFrame event handler declarations

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_CANCEL
    void OnCancelClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_CLEAR_ANNOTATION_CMP
    void OnClearAnnotationCmpClick( wxCommandEvent& event );

    /// wxEVT_COMMAND_BUTTON_CLICKED event handler for wxID_APPLY
    void OnApplyClick( wxCommandEvent& event );

////@end WinEDA_AnnotateFrame event handler declarations

////@begin WinEDA_AnnotateFrame member function declarations

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end WinEDA_AnnotateFrame member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();

    // User functions:
    bool GetLevel( void );
    bool GetResetItems( void );
    bool GetSortOrder( void );

////@begin WinEDA_AnnotateFrame member variables
    wxRadioButton* m_rbEntireSchematic;
    wxCheckBox* m_cbResetAnnotation;
    wxRadioButton* m_rbSortByPosition;
    wxRadioButton* rbSortByValue;
    wxBoxSizer* sizerDialogButtons;
    wxButton* m_btnClose;
    wxButton* m_btnClear;
    wxButton* m_btnApply;
////@end WinEDA_AnnotateFrame member variables

    WinEDA_SchematicFrame * m_Parent;

};

#endif
    // _ANNOTATE_DIALOG_H_
