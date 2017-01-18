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

#include <dialog_multibutton.h>

#include <wx/event.h>


static const int BORDER_WIDTH = 5;


DIALOG_MULTIBUTTON::DIALOG_MULTIBUTTON( wxWindow* aParent,
        const wxString& aTitle ) :
    DIALOG_SHIM( aParent, wxID_ANY, aTitle,
            wxDefaultPosition, wxDefaultSize,
            wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER ),
    m_image( nullptr )
{
    /* Sizers go something like this:
     *
     *  +---------------------------------+
     *  | +-------+---------------------+ |
     *  | |  Img  |      Body item      | |
     *  | |       |      Body item      | |
     *  | +-------+---------------------+ |
     *  | +-----------------------------+ |
     *  | |        Button Button Button | |
     *  | +-----------------------------+ |
     *  +---------------------------------+
     */

    // overall sizer for the whole dialog
    wxBoxSizer* mainBoxSizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( mainBoxSizer );

    // L/R size for hint image/body items
    m_topBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    mainBoxSizer->Add( m_topBoxSizer, 1, wxALIGN_TOP | wxALL, 0 );

    // Vertical sizer for body items
    m_itemBoxSizer = new wxBoxSizer( wxVERTICAL );
    m_topBoxSizer->Add( m_itemBoxSizer, 1, wxALIGN_TOP | wxALL, BORDER_WIDTH );

    // L/R sizer for the dialog buttons
    m_buttonBoxSizer = new wxBoxSizer( wxHORIZONTAL );
    mainBoxSizer->Add( m_buttonBoxSizer, 0, wxALIGN_RIGHT | wxALL, BORDER_WIDTH );

    // Relayout on resize (for example, so text items will wrap)
    Bind( wxEVT_SIZE, [ = ] ( wxSizeEvent& event ) {
        Layout();
        event.Skip();
    });
}

void DIALOG_MULTIBUTTON::SetHintImage( const wxArtID& aImageId )
{
    wxBitmap bmp = wxArtProvider::GetBitmap( aImageId, wxART_MESSAGE_BOX, { 48, 48 } );

    // create or update the image
    if( !m_image )
    {
        m_image = new wxStaticBitmap( this, wxID_ANY, bmp );
        // image goes on the left of the top sizer
        m_topBoxSizer->Insert( 0, m_image, 0, wxALIGN_TOP | wxALL, BORDER_WIDTH * 2 );
    }
    else
    {
        m_image->SetBitmap( bmp );
    }
}


void DIALOG_MULTIBUTTON::AddText( const wxString aMessage )
{
    auto text = new wxStaticText( this, wxID_ANY, aMessage );

    m_itemBoxSizer->Add( text, 0, wxGROW | wxALL, BORDER_WIDTH );
}


void DIALOG_MULTIBUTTON::AddButton( int aId, const wxString& aLabel,
        bool aIsDefault )
{
    std::unique_ptr<wxButton> button( new wxButton( this, aId, aLabel ) );

    AddButton( std::move( button ) );
}


void DIALOG_MULTIBUTTON::AddButton( std::unique_ptr<wxButton> aButton,
        bool aIsDefault )
{
    if( aIsDefault )
    {
        aButton->SetDefault();
    }

    const auto endId = aButton->GetId();

    aButton->Bind( wxEVT_BUTTON, [ = ] ( wxCommandEvent& evt ) {
        EndModal( endId );
    } );

    m_buttonBoxSizer->Add( aButton.release(), 0, wxGROW | wxALL, BORDER_WIDTH );
}


void DIALOG_MULTIBUTTON::AddItem( std::unique_ptr<wxWindow> aItem )
{
    m_itemBoxSizer->Add( aItem.release(), 0, wxGROW | wxALL, BORDER_WIDTH );
}


void DIALOG_MULTIBUTTON::AdjustSize()
{
    FinishDialogSettings();
}
