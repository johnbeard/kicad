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

#ifndef COMMON_DIALOGS_DIALOG_MULTIBUTTON_H_
#define COMMON_DIALOGS_DIALOG_MULTIBUTTON_H_

#include "dialog_shim.h"

#include <memory>

#include <wx/sizer.h>
#include <wx/string.h>
#include <wx/artprov.h>

/**
 * Class DIALOG_MULTIBUTTON
 *
 * A simple dialog class that allows you you add a set of items
 * and buttons to a wxDialog and display it modally, then get the
 * button IDs when the modal dialog ends.
 *
 * This is designed for multiple-choice questions to the user, but
 * can be used for any easily-laid-out interaction.
 *
 * Anything that requires the dialog be smarter about the layout than
 * just throwing the items into one sizer probably means that this
 * class is a bad fit.
 */
class DIALOG_MULTIBUTTON : public DIALOG_SHIM
{
public:

    /**
     * Construct an empty dialog with the given title.
     *
     * You shoulf then add items like text and buttons bofore showing
     * to the user
     */
    DIALOG_MULTIBUTTON( wxWindow* aParent, const wxString& aTitle );

    /**
     * Add a block of text to the dialog.
     */
    void AddText( const wxString aMessage );

    /*!
     * Construct and add a button to the dialog, show along the bottom.
     * A click on this button will end the modal dialog
     *
     * If you use a stock ID and empty label, a stock button will be
     * created and used
     *
     * @param aId the button ID, returned on modal end from that button
     * @param aLabel the button label
     * @param aIsDefault set this button as the default for the dialog
     */
    void AddButton( int aId, const wxString& aLabel,
            bool aIsDefault = false );

    /**
     * Add an existing button to the dialog's bottom row. Clicking
     * the button will call EndModal with the button's ID.
     *
     * @param aButton a button to take ownership of and add to the dialog
     * @param aIsDefault set this button as the default for the dialog
     */
    void AddButton( std::unique_ptr<wxButton> aButton,
                    bool aIsDefault = false );

    /**
     * Set the hint image of the dialog (defualt is no image)
     *
     * @param aImageId the wxARTID ID of the image you want.
     *        wxART_ERROR, wxART_WARNING, wxART_INFOMRATION are likely
     *        common values.
     */
    void SetHintImage( const wxArtID& aImageId );

    /**
     * Add an existing control to the dialog.
     * Ownership will be taken, but you can retain a pointer if you want
     * to retrieve stare before Destroy/destruction. You can also
     * bind the item events before passing it in.
     */
    void AddItem( std::unique_ptr<wxWindow> aItem );

    /**
     * Adjust the size of the dialog to the contained items and
     * centre in the parent window.
     *
     * Normally called once all items have been added
     */
    void AdjustSize();

private:
    wxBoxSizer* m_itemBoxSizer;
    wxBoxSizer* m_buttonBoxSizer;
    wxBoxSizer* m_topBoxSizer;
    wxStaticBitmap* m_image;
};


#endif /* COMMON_DIALOGS_DIALOG_MULTIBUTTON_H_ */
