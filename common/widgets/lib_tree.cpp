/* -*- c++ -*-
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014 Henner Zeller <h.zeller@acm.org>
 * Copyright (C) 2014-2018 KiCad Developers, see AUTHORS.txt for contributors.
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

#include "lib_tree.h"
#include <wxdataviewctrl_helpers.h>

#include <wx/artprov.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/html/htmlwin.h>

#include <lib_table_base.h>


/**
 * Flag to enable logging of LIB_TREE code
 *
 * Use "KICAD_LIB_TREE" to enable.
 *
 * @ingroup trace_env_vars
 */
static const wxChar* trace_lib_tree = wxT( "KICAD_LIB_TREE" );


LIB_TREE::LIB_TREE( wxWindow* aParent, LIB_TABLE* aLibTable, LIB_TREE_MODEL_ADAPTER::PTR& aAdapter,
                    WIDGETS aWidgets, wxHtmlWindow* aDetails )
    : wxPanel( aParent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
               wxWANTS_CHARS | wxTAB_TRAVERSAL | wxNO_BORDER ),
      m_lib_table( aLibTable ),
      m_adapter( aAdapter ),
      m_query_ctrl( nullptr ),
      m_details_ctrl( nullptr ),
      m_menuActive( false ),
      m_filtering( false )
{
    // create space for context menu pointers, INVALID is the max value
    m_menus.resize( LIB_TREE_NODE::TYPE::INVALID + 1 );

    auto sizer = new wxBoxSizer( wxVERTICAL );

    // Search text control
    if( aWidgets & SEARCH )
    {
        auto search_sizer = new wxBoxSizer( wxHORIZONTAL );

        m_query_ctrl = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                       wxDefaultSize, wxTE_PROCESS_ENTER );

// Additional visual cue for GTK, which hides the placeholder text on focus
#ifdef __WXGTK__
        auto bitmap = new wxStaticBitmap( this, wxID_ANY, wxArtProvider::GetBitmap( wxART_FIND, wxART_FRAME_ICON ) );

        if( aWidgets & DETAILS )
            search_sizer->Add( bitmap, 0, wxALIGN_CENTER | wxALL, 5 );
        else
            search_sizer->Add( bitmap, 0, wxALIGN_CENTER | wxRIGHT, 5 );
#endif

        if( aWidgets & DETAILS )
            search_sizer->Add( m_query_ctrl, 1, wxLEFT | wxTOP | wxRIGHT | wxEXPAND, 5 );
        else
            search_sizer->Add( m_query_ctrl, 1, wxEXPAND, 5 );
        sizer->Add( search_sizer, 0, wxEXPAND, 5 );

        m_query_ctrl->Bind( wxEVT_TEXT, &LIB_TREE::onQueryText, this );
        m_query_ctrl->Bind( wxEVT_TEXT_ENTER, &LIB_TREE::onQueryEnter, this );
        m_query_ctrl->Bind( wxEVT_CHAR_HOOK, &LIB_TREE::onQueryCharHook, this );
    }

    // Tree control
    m_tree_ctrl = new wxDataViewCtrl( this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                      wxDV_SINGLE | wxDV_NO_HEADER );
    m_adapter->AttachTo( m_tree_ctrl );

    if( aWidgets & DETAILS )
        sizer->Add( m_tree_ctrl, 5, wxLEFT | wxTOP | wxRIGHT | wxEXPAND, 5 );
    else
        sizer->Add( m_tree_ctrl, 5, wxRIGHT | wxBOTTOM | wxEXPAND, 1 );

    // Description panel
    if( aWidgets & DETAILS )
    {
        if( !aDetails )
        {
            auto html_sz = ConvertDialogToPixels( wxPoint( 80, 80 ) );

            m_details_ctrl = new wxHtmlWindow(
                    this, wxID_ANY, wxDefaultPosition, wxSize( html_sz.x, html_sz.y ),
                    wxHW_SCROLLBAR_AUTO );

            sizer->Add( m_details_ctrl, 2, wxALL | wxEXPAND, 5 );
        }
        else
        {
            m_details_ctrl = aDetails;
        }

        m_details_ctrl->Bind( wxEVT_HTML_LINK_CLICKED, &LIB_TREE::onDetailsLink, this );
    }

    SetSizer( sizer );

    m_tree_ctrl->Bind( wxEVT_DATAVIEW_ITEM_ACTIVATED, &LIB_TREE::onTreeActivate, this );
    m_tree_ctrl->Bind( wxEVT_DATAVIEW_SELECTION_CHANGED, &LIB_TREE::onTreeSelect, this );
    m_tree_ctrl->Bind( wxEVT_COMMAND_DATAVIEW_ITEM_CONTEXT_MENU, &LIB_TREE::onContextMenu, this );

    Bind( COMPONENT_PRESELECTED, &LIB_TREE::onPreselect, this );

    // If wxTextCtrl::SetHint() is called before binding wxEVT_TEXT, the event
    // handler will intermittently fire.
    if( m_query_ctrl )
    {
        m_query_ctrl->SetHint( _( "Filter" ) );
        m_query_ctrl->SetFocus();
        m_query_ctrl->SetValue( wxEmptyString );    // SetValue() is required here to kick off
                                                    // initial sorting and pre-selection.
    }

    // There may be a part preselected in the model. Make sure it is displayed.
    postPreselectEvent();

    Layout();
    sizer->Fit( this );

#ifdef __WXGTK__
    // Scrollbars must be always enabled to prevent an infinite event loop
    // more details: http://trac.wxwidgets.org/ticket/18141
    if( m_details_ctrl )
        m_details_ctrl->ShowScrollbars( wxSHOW_SB_ALWAYS, wxSHOW_SB_ALWAYS );
#endif /* __WXGTK__ */
}


LIB_ID LIB_TREE::GetSelectedLibId( int* aUnit ) const
{
    auto sel = m_tree_ctrl->GetSelection();

    if( !sel )
    {
        LIB_ID emptyId;

        return emptyId;
    }

    if( aUnit )
        *aUnit = m_adapter->GetUnitFor( sel );

    return m_adapter->GetAliasFor( sel );
}


void LIB_TREE::SelectLibId( const LIB_ID& aLibId )
{
    selectIfValid( m_adapter->FindItem( aLibId ) );
}


void LIB_TREE::CenterLibId( const LIB_ID& aLibId )
{
    centerIfValid( m_adapter->FindItem( aLibId ) );
}


void LIB_TREE::Unselect()
{
    m_tree_ctrl->UnselectAll();
}

void LIB_TREE::ExpandLibId( const LIB_ID& aLibId )
{
    expandIfValid( m_adapter->FindItem( aLibId ) );
}


void LIB_TREE::Regenerate( bool aKeepState )
{
    STATE current;

    wxLogTrace( trace_lib_tree, "Regenerate, keep state: %d", aKeepState );

    // Store the state
    if( aKeepState )
        m_unfilteredState = getState();

    wxLogTrace( trace_lib_tree, "Unfiltered state: selection '%s'",
        m_unfilteredState.selection.GetUniStringLibId() );

    wxLogTrace( trace_lib_tree, "   Current selection '%s'",
        GetSelectedLibId().GetUniStringLibId() );

    wxString filter = m_query_ctrl->GetValue();

    wxLogTrace( trace_lib_tree, "   Updating adapter filter '%s'", filter);
    m_adapter->UpdateSearchString( filter );
    m_filtering = !filter.IsEmpty();
    wxLogTrace( trace_lib_tree, "  Filtering '%d'", m_filtering );

    postPreselectEvent();

    // Restore the state
    if( aKeepState )
        setState( m_unfilteredState );
}


void LIB_TREE::SetFocus()
{
    if( m_query_ctrl )
        m_query_ctrl->SetFocus();
    else
        m_tree_ctrl->SetFocus();
}


void LIB_TREE::toggleExpand( const wxDataViewItem& aTreeId )
{
    wxLogTrace( trace_lib_tree, "toggleExpand" );

    if( !aTreeId.IsOk() )
        return;

    wxLogTrace( trace_lib_tree, "...ok" );

    if( m_tree_ctrl->IsExpanded( aTreeId ) )
        m_tree_ctrl->Collapse( aTreeId );
    else
        m_tree_ctrl->Expand( aTreeId );
}


void LIB_TREE::selectIfValid( const wxDataViewItem& aTreeId )
{
    wxLogTrace( trace_lib_tree, "selectIfValid" );

    if( aTreeId.IsOk() )
    {
        wxLogTrace( trace_lib_tree, "...ok" );

        m_tree_ctrl->EnsureVisible( aTreeId );
        m_tree_ctrl->Select( aTreeId );
        postPreselectEvent();
    }
}


void LIB_TREE::centerIfValid( const wxDataViewItem& aTreeId )
{
    wxLogTrace( trace_lib_tree, "centerIfValid" );

    if( aTreeId.IsOk() )
    {
        wxLogTrace( trace_lib_tree, "...ok" );
        m_tree_ctrl->EnsureVisible( aTreeId );
    }
}


void LIB_TREE::expandIfValid( const wxDataViewItem& aTreeId )
{
    wxLogTrace( trace_lib_tree, "expandIfValid" );


    if( aTreeId.IsOk() && !m_tree_ctrl->IsExpanded( aTreeId ) )
    {
        wxLogTrace( trace_lib_tree, "Expand" );
        m_tree_ctrl->Expand( aTreeId );
    }
}


void LIB_TREE::postPreselectEvent()
{
    wxLogTrace( trace_lib_tree, "Post Preselect" );
    wxCommandEvent event( COMPONENT_PRESELECTED );
    wxPostEvent( this, event );
}


void LIB_TREE::postSelectEvent()
{
    wxLogTrace( trace_lib_tree, "Post Select" );
    wxCommandEvent event( COMPONENT_SELECTED );
    wxPostEvent( this, event );
}


LIB_TREE::STATE LIB_TREE::getState() const
{
    STATE state;
    wxDataViewItemArray items;
    m_adapter->GetChildren( wxDataViewItem( nullptr ), items );

    for( const auto& item : items )
    {
        if( m_tree_ctrl->IsExpanded( item ) )
            state.expanded.push_back( item );
    }

    state.selection = GetSelectedLibId();

    return state;
}


void LIB_TREE::setState( const STATE& aState )
{
    wxLogTrace( trace_lib_tree, "Setting state: selection '%s'",
        aState.selection.GetUniStringLibId() );

    m_tree_ctrl->Freeze();

    for( const auto& item : aState.expanded )
        m_tree_ctrl->Expand( item );

    // wxDataViewCtrl cannot be frozen when a selection
    // command is issued, otherwise it selects a random item (Windows)
    m_tree_ctrl->Thaw();

    if( !aState.selection.GetLibItemName().empty() || !aState.selection.GetLibNickname().empty() )
        SelectLibId( aState.selection );
}


void LIB_TREE::onQueryText( wxCommandEvent& aEvent )
{
    wxLogTrace( trace_lib_tree, "onQueryText" );
    Regenerate( false );

    // Required to avoid interaction with SetHint()
    // See documentation for wxTextEntry::SetHint
    aEvent.Skip();
}


void LIB_TREE::onQueryEnter( wxCommandEvent& aEvent )
{
    if( GetSelectedLibId().IsValid() )
        postSelectEvent();
}


void LIB_TREE::onQueryCharHook( wxKeyEvent& aKeyStroke )
{
    auto const sel = m_tree_ctrl->GetSelection();
    auto type = sel.IsOk() ? m_adapter->GetTypeFor( sel ) : LIB_TREE_NODE::INVALID;

    switch( aKeyStroke.GetKeyCode() )
    {
    case WXK_UP:
        selectIfValid( GetPrevItem( *m_tree_ctrl, sel ) );
        break;

    case WXK_DOWN:
        selectIfValid( GetNextItem( *m_tree_ctrl, sel ) );
        break;

    case WXK_ADD:
        if( type == LIB_TREE_NODE::LIB )
            m_tree_ctrl->Expand( sel );

        break;

    case WXK_SUBTRACT:
        if( type == LIB_TREE_NODE::LIB )
            m_tree_ctrl->Collapse( sel );

        break;

    case WXK_RETURN:
        if( type == LIB_TREE_NODE::LIB )
        {
            toggleExpand( sel );
            break;
        }
        /* fall through, so the selected component will be treated as the selected one */

    default:
        wxLogTrace( trace_lib_tree, "Skipping onQueryCharHook" );
        aKeyStroke.Skip(); // Any other key: pass on to search box directly.
        break;
    }
}


void LIB_TREE::onTreeSelect( wxDataViewEvent& aEvent )
{
    wxLogTrace( trace_lib_tree, "onTreeSelect" );
    postPreselectEvent();
}


void LIB_TREE::onTreeActivate( wxDataViewEvent& aEvent )
{
    wxLogTrace( trace_lib_tree, "Activated: '%s'",
        GetSelectedLibId().GetUniStringLibId() );

    if( !GetSelectedLibId().IsValid() )
    {
        // Expand library/part units subtree
        toggleExpand( m_tree_ctrl->GetSelection() );
    }
    else
    {
        postSelectEvent();
    }
}


void LIB_TREE::onDetailsLink( wxHtmlLinkEvent& aEvent )
{
    const wxHtmlLinkInfo& info = aEvent.GetLinkInfo();
    ::wxLaunchDefaultBrowser( info.GetHref() );
}


void LIB_TREE::onPreselect( wxCommandEvent& aEvent )
{
    wxLogTrace( trace_lib_tree, "onPreselect: '%s'",
        GetSelectedLibId().GetUniStringLibId() );
    if( m_details_ctrl )
    {
        int unit = 0;
        LIB_ID id = GetSelectedLibId( &unit );

        if( id.IsValid() )
            m_details_ctrl->SetPage( m_adapter->GenerateInfo( id, unit ) );
        else
            m_details_ctrl->SetPage( wxEmptyString );
    }

    aEvent.Skip();
}


void LIB_TREE::onContextMenu( wxDataViewEvent& aEvent )
{
    auto const sel = m_tree_ctrl->GetSelection();
    auto type = sel.IsOk() ? m_adapter->GetTypeFor( sel ) : LIB_TREE_NODE::INVALID;

    if( m_menus[type] )
    {
        m_menuActive = true;
        PopupMenu( m_menus[type].get() );
        m_menuActive = false;
    }
}


wxDEFINE_EVENT( COMPONENT_PRESELECTED, wxCommandEvent );
wxDEFINE_EVENT( COMPONENT_SELECTED, wxCommandEvent );
