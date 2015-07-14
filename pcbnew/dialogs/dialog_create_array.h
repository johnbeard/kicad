/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 John Beard, john.j.beard@gmail.com
 * Copyright (C) 1992-2015 KiCad Developers, see AUTHORS.txt for contributors.
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

#ifndef __DIALOG_CREATE_ARRAY__
#define __DIALOG_CREATE_ARRAY__

// Include the wxFormBuider header base:
#include <dialog_create_array_base.h>

#include <boost/bimap.hpp>

class CONFIG_SAVE_RESTORE_WINDOW
{
protected:
    /*!
     * Struct RECORD_T
     *
     * Base class for a save/restore window - this provides the "valid" flag
     * and any other common methods.
     */
    struct RECORD_T
    {
        RECORD_T() :
            m_configValid( false )
        {}

        void SetValid()
        {
            m_configValid = true;
        }

        bool IsConfigValid() const
        {
            return m_configValid;
        }

        bool m_configValid;
    };
private:

    enum CONFIG_CTRL_TYPE_T
    {
        CFG_CTRL_TEXT,
        CFG_CTRL_CHECKBOX,
        CFG_CTRL_RADIOBOX,
        CFG_CTRL_CHOICE,
        CFG_CTRL_TAB,
        CFG_CTRL_SPINBOX,
    };

    struct CONFIG_CTRL_T
    {
        wxControl* control;
        CONFIG_CTRL_TYPE_T type;
        void* dest;
    };

    std::vector<CONFIG_CTRL_T> ctrls;
    RECORD_T& m_record;

protected:

    CONFIG_SAVE_RESTORE_WINDOW( RECORD_T& aRecord ) :
        m_record( aRecord )
    {}

    void Add( wxRadioBox* ctrl, int& dest )
    {
        CONFIG_CTRL_T ctrlInfo = { ctrl, CFG_CTRL_RADIOBOX, (void*) &dest };

        ctrls.push_back( ctrlInfo );
    }

    void Add( wxCheckBox* ctrl, bool& dest )
    {
        CONFIG_CTRL_T ctrlInfo = { ctrl, CFG_CTRL_CHECKBOX, (void*) &dest };

        ctrls.push_back( ctrlInfo );
    }

    void Add( wxTextCtrl* ctrl, wxString& dest )
    {
        CONFIG_CTRL_T ctrlInfo = { ctrl, CFG_CTRL_TEXT, (void*) &dest };

        ctrls.push_back( ctrlInfo );
    }

    void Add( wxChoice* ctrl, int& dest )
    {
        CONFIG_CTRL_T ctrlInfo = { ctrl, CFG_CTRL_CHOICE, (void*) &dest };

        ctrls.push_back( ctrlInfo );
    }

    void Add( wxNotebook* ctrl, int& dest )
    {
        CONFIG_CTRL_T ctrlInfo = { ctrl, CFG_CTRL_TAB, (void*) &dest };

        ctrls.push_back( ctrlInfo );
    }

    void Add( wxSpinCtrl* ctrl, int& dest )
    {
        CONFIG_CTRL_T ctrlInfo = { ctrl, CFG_CTRL_SPINBOX, (void*) &dest };

        ctrls.push_back( ctrlInfo );
    }

    void ReadConfigFromControls()
    {
        for( std::vector<CONFIG_CTRL_T>::const_iterator iter = ctrls.begin(), iend = ctrls.end();
             iter != iend; ++iter )
        {
            switch( iter->type )
            {
            case CFG_CTRL_CHECKBOX:
                *(bool*) iter->dest = static_cast<wxCheckBox*>( iter->control )->GetValue();
                break;

            case CFG_CTRL_TEXT:
                *(wxString*) iter->dest = static_cast<wxTextCtrl*>( iter->control )->GetValue();
                break;

            case CFG_CTRL_CHOICE:
                *(int*) iter->dest = static_cast<wxChoice*>( iter->control )->GetSelection();
                break;

            case CFG_CTRL_RADIOBOX:
                *(int*) iter->dest = static_cast<wxRadioBox*>( iter->control )->GetSelection();
                break;

            case CFG_CTRL_TAB:
                *(int*) iter->dest = static_cast<wxNotebook*>( iter->control )->GetSelection();
                break;

            case CFG_CTRL_SPINBOX:
                *(int*) iter->dest = static_cast<wxSpinCtrl*>( iter->control )->GetValue();
                break;

            default:
                wxASSERT_MSG( false, wxString(
                                "Unhandled control type for config store: " ) << iter->type );
            }
        }

        m_record.SetValid();
    }

    void RestoreConfigToControls()
    {
        if( !m_record.IsConfigValid() )
            return;

        for( std::vector<CONFIG_CTRL_T>::const_iterator iter = ctrls.begin(), iend = ctrls.end();
             iter != iend; ++iter )
        {
            switch( iter->type )
            {
            case CFG_CTRL_CHECKBOX:
                static_cast<wxCheckBox*>( iter->control )->SetValue( *(bool*) iter->dest );
                break;

            case CFG_CTRL_TEXT:
                static_cast<wxTextCtrl*>( iter->control )->SetValue( *(wxString*) iter->dest );
                break;

            case CFG_CTRL_CHOICE:
                static_cast<wxChoice*>( iter->control )->SetSelection( *(int*) iter->dest );
                break;

            case CFG_CTRL_RADIOBOX:
                static_cast<wxRadioBox*>( iter->control )->SetSelection( *(int*) iter->dest );
                break;

            case CFG_CTRL_TAB:
                static_cast<wxNotebook*>( iter->control )->SetSelection( *(int*) iter->dest );
                break;

            case CFG_CTRL_SPINBOX:
                static_cast<wxSpinCtrl*>( iter->control )->SetValue( *(int*) iter->dest );
                break;

            default:
                wxASSERT_MSG( false, wxString(
                                "Unhandled control type for config restore: " ) << iter->type );
            }
        }
    }
};

class DIALOG_CREATE_ARRAY : public DIALOG_CREATE_ARRAY_BASE,
    public CONFIG_SAVE_RESTORE_WINDOW
{
public:

    enum ARRAY_TYPE_T
    {
        ARRAY_GRID,         ///< A grid (x*y) array
        ARRAY_CIRCULAR,     ///< A circular array
    };

    // NOTE: do not change order relative to charSetDescriptions
    enum ARRAY_NUMBERING_TYPE_T
    {
        NUMBERING_NUMERIC = 0,      ///< Arabic numerals: 0,1,2,3,4,5,6,7,8,9,10,11...
        NUMBERING_HEX,
        NUMBERING_ALPHA_NO_IOSQXZ,  /*!< Alphabet, excluding IOSQXZ
                                     *
                                     * Per ASME Y14.35M-1997 sec. 5.2 (previously MIL-STD-100 sec. 406.5)
                                     * as these can be confused with numerals and are often not used
                                     * for pin numbering on BGAs, etc
                                     */
        NUMBERING_ALPHA_FULL,       ///< Full 26-character alphabet
        NUMBERING_TYPE_Max          ///< Invalid maximum value, insert above here
    };

    /**
     * Struct ARRAY_OPTIONS
     *
     * This is the base class which describes some grid and provides the
     * laying out of array items according to some given parameters, which
     * will mostly depend on the array type, with a few common ones.
     */
    struct ARRAY_OPTIONS
    {
        ARRAY_OPTIONS( ARRAY_TYPE_T aType ) :
            m_type( aType ),
            m_shouldRenumber( false )
        {}

        virtual ~ARRAY_OPTIONS() {};

        ARRAY_TYPE_T m_type;
        bool m_shouldRenumber;

        /*!
         * Function GetArrayPositions
         * Returns the set of points that represent the array
         * in order, if that is important
         *
         * TODO: Can/should this be done with some sort of iterator?
         */
        virtual void TransformItem( int n, BOARD_ITEM* item,
                const wxPoint& rotPoint ) const = 0;
        virtual int         GetArraySize() const = 0;
        virtual wxString GetItemNumber( int n ) const = 0;
        virtual wxString InterpolateNumberIntoString( int n, const wxString& pattern ) const;

        bool        ShouldRenumberItems() const
        {
            return m_shouldRenumber;
        }

        /*!
         * Struct AXIS_T
         * Describes a single axis of an array. Arrays can have one or more
         * axes (grids have two, circle have one, for example)
         */
        struct AXIS_T
        {
            ARRAY_NUMBERING_TYPE_T m_type;
            int m_offset;
            int m_step;

            AXIS_T() :
                m_type( NUMBERING_NUMERIC ),
                m_offset( 0 ),
                m_step ( 1 )
            {}

            /*!
             * Function GetCoordinateNumberForIndex
             *
             * Get the coordinate number for a give axis index. This applies
             * the offset and stepping internally, and returns a string in the
             * correct alphabet
             *
             * @param n the index of the coordinate to get a number for
             * @return wxString of the coordinate
             */
            wxString GetCoordinateNumberForIndex( int n ) const;
        };
    };


    /*!
     * Struct ARRAY_GRID_OPTIONS
     * Parameters and layout for 2D grid arrays
     */
    struct ARRAY_GRID_OPTIONS : public ARRAY_OPTIONS
    {
        ARRAY_GRID_OPTIONS() :
            ARRAY_OPTIONS( ARRAY_GRID ),
            m_nx( 0 ), m_ny( 0 ),
            m_horizontalThenVertical( true ),
            m_reverseNumberingAlternate( false ),
            m_stagger( 0 ),
            m_stagger_rows( true ),
            m_2dArrayNumbering( false )
        {}

        long    m_nx, m_ny;
        bool    m_horizontalThenVertical, m_reverseNumberingAlternate;
        wxPoint m_delta;
        wxPoint m_offset;
        long    m_stagger;
        bool    m_stagger_rows;
        bool    m_2dArrayNumbering;
        AXIS_T  m_priAxis, m_secAxis;

        void        TransformItem( int n, BOARD_ITEM* item, const wxPoint& rotPoint ) const;    // override virtual
        int         GetArraySize() const;                                                       // override virtual
        wxString    GetItemNumber( int n ) const;                                               // override virtual

private:
        wxPoint getGridCoords( int n ) const;
    };

    /*!
     * Struct ARRAY_CIRCULAR_OPTIONS
     *
     * Parameters and layout for circular arrays
     */
    struct ARRAY_CIRCULAR_OPTIONS : public ARRAY_OPTIONS
    {
        ARRAY_CIRCULAR_OPTIONS() :
            ARRAY_OPTIONS( ARRAY_CIRCULAR ),
            m_nPts( 0 ),
            m_angle( 0.0f ),
            m_relativeCentre( false ),
            m_rotateItems( false )
        {}

        long m_nPts;
        double m_angle;
        wxPoint m_centre;
        bool m_relativeCentre;
        bool m_rotateItems;
        AXIS_T m_axis;

        void        TransformItem( int n, BOARD_ITEM* item, const wxPoint& rotPoint ) const;    // override virtual
        int         GetArraySize() const;                                                       // override virtual
        wxString    GetItemNumber( int n ) const;                                               // override virtual
    };

    // Constructor and destructor
    DIALOG_CREATE_ARRAY( PCB_BASE_FRAME* aParent, wxPoint aOrigPos, ARRAY_OPTIONS** settings );
    virtual ~DIALOG_CREATE_ARRAY() {};

private:

    /**
     * The settings object returned to the caller by this dialog.
     * We update the caller's object and never have ownership
     */
    ARRAY_OPTIONS** m_settings;

    /*!
     * The position of the original item(s), used for finding radius, etc
     */
    const wxPoint m_originalItemPosition;

    // Event callbacks
    void    OnParameterChanged( wxCommandEvent& event );
    void    OnOkClick( wxCommandEvent& event );

    // Internal callback handlers
    void setControlEnablement();

    // Calculate derived properties of a circular grid (eg radius)
    void calculateCircularArrayProperties();

    /*!
     * Fill an axis description from a set of controls
     * @return true if the values made sense
     */
    bool fillAxisFromControls( ARRAY_OPTIONS::AXIS_T& axis,
            wxChoice* alphabetSelector,
            wxTextCtrl* offsetEntry,
            wxSpinCtrl* stepEntry );

    /*!
     * Persistent settings for the create array dialog - these are
     * save on close and reloaded on next dialog open
     */
    struct CREATE_ARRAY_DIALOG_ENTRIES: public CONFIG_SAVE_RESTORE_WINDOW::RECORD_T
    {
        CREATE_ARRAY_DIALOG_ENTRIES() :
            m_gridStaggerType( 0 ),
            m_gridNumberingAxis( 0 ),
            m_gridNumberingReverseAlternate( false ),
            m_grid2dArrayNumbering( 0 ),
            m_gridPriAxisNumScheme( 0 ),
            m_gridSecAxisNumScheme( 0 ),
            m_gridPriAxisNumberingStep( 1 ),
            m_gridSecAxisNumberingStep( 1 ),
            m_circRotate( false ),
            m_circRelativeCentre( false ),
            m_circNumberingStep( 1 ),
            m_arrayTypeTab( 0 )
        {}

        wxString m_gridNx, m_gridNy,
                 m_gridDx, m_gridDy,
                 m_gridOffsetX, m_gridOffsetY,
                 m_gridStagger;

        int      m_gridStaggerType, m_gridNumberingAxis;
        bool     m_gridNumberingReverseAlternate;
        int      m_grid2dArrayNumbering;
        int      m_gridPriAxisNumScheme, m_gridSecAxisNumScheme;
        wxString m_gridPriNumberingOffset, m_gridSecNumberingOffset;
        int      m_gridPriAxisNumberingStep, m_gridSecAxisNumberingStep;

        wxString m_circCentreX, m_circCentreY,
                 m_circAngle, m_circCount, m_circNumberingOffset;
        bool     m_circRotate, m_circRelativeCentre;
        int      m_circNumberingStep;

        int      m_arrayTypeTab;
    };

    /*!
     * The static dialog state, which is used to save and restore the
     * dialog settings
     */
    static CREATE_ARRAY_DIALOG_ENTRIES m_options;
};

#endif      // __DIALOG_CREATE_ARRAY__
