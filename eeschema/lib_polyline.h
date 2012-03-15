/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2004-2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file lib_polyline.h
 */

#ifndef _LIB_POLYLINE_H_
#define _LIB_POLYLINE_H_

#include <lib_draw_item.h>


class LIB_POLYLINE : public LIB_ITEM
{
    int m_Width;                              // Line width
    std::vector<wxPoint> m_PolyPoints;        // list of points (>= 2)

    int m_ModifyIndex;                        // Index of the polyline point to modify

    /**
     * Draw the polyline.
     */
    void drawGraphic( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
                      int aColor, int aDrawMode, void* aData, const TRANSFORM& aTransform );

    /**
     * Calculate the polyline attributes relative to \a aPosition while editing.
     *
     * @param aPosition - Edit position in drawing units.
     */
    void calcEdit( const wxPoint& aPosition );

public:
    LIB_POLYLINE( LIB_COMPONENT * aParent );

    // Do not create a copy constructor.  The one generated by the compiler is adequate.

    ~LIB_POLYLINE() { }

    virtual wxString GetClass() const
    {
        return wxT( "LIB_POLYLINE" );
    }


    /**
     * Write polyline object out to a FILE in "*.lib" format.
     *
     * @param aFormatter A reference to an OUTPUTFORMATTER to write the component library
     *                   polyline to.
     * @return True if success writing else false.
     */
    virtual bool Save( OUTPUTFORMATTER& aFormatter );

    virtual bool Load( LINE_READER& aLineReader, wxString& aErrorMsg );

    void AddPoint( const wxPoint& aPoint );

    /**
     * Delete the segment at \a aPosition.
     */
    void DeleteSegment( const wxPoint aPosition );

    /**
     * @return the number of corners
     */
    unsigned GetCornerCount() const { return m_PolyPoints.size(); }

     /**
     * Test if the given point is within the bounds of this object.
     *
     * @param aPosition - A wxPoint to test
     * @return - true if a hit, else false
     */
    virtual bool HitTest( const wxPoint& aPosition );

    /**
     * @param aPosition = a wxPoint to test
     * @param aThreshold = max distance to a segment
     * @param aTransform = the transform matrix
     * @return true if the point \a aPosition is near a segment
     */
    virtual bool HitTest( wxPoint aPosition, int aThreshold, const TRANSFORM& aTransform );

    /**
     * Function GetBoundingBox
     * @return the boundary box for this, in library coordinates
     */
    virtual EDA_RECT GetBoundingBox() const;

    /**
     * Function GetPenSize
     * @return the size of the "pen" that be used to draw or plot this item
     */
    virtual int GetPenSize( ) const;

    virtual void DisplayInfo( EDA_DRAW_FRAME* aFrame );

    /**
     * See LIB_ITEM::BeginEdit().
     */
    void BeginEdit( int aEditMode, const wxPoint aStartPoint = wxPoint( 0, 0 ) );

    /**
     * See LIB_ITEM::ContinueEdit().
     */
    bool ContinueEdit( const wxPoint aNextPoint );

    /**
     * See LIB_ITEM::AbortEdit().
     */
    void EndEdit( const wxPoint& aPosition, bool aAbort = false );

    /**
     * @copydoc LIB_ITEM::SetOffset(const wxPoint&)
     */
    virtual void SetOffset( const wxPoint& aOffset );

    /**
     * @copydoc LIB_ITEM::Inside()
     */
    virtual bool Inside( EDA_RECT& aRect ) const;

    /**
     * @copydoc LIB_ITEM::Move()
     */
    virtual void Move( const wxPoint& aPosition );

    /**
     * @copydoc LIB_ITEM::GetPosition()
     */
    virtual wxPoint GetPosition() const { return m_PolyPoints[0]; }

    /**
     * @copydoc LIB_ITEM::MirrorHorizontal()
     */
    virtual void MirrorHorizontal( const wxPoint& aCenter );

    /**
     * @copydoc LIB_ITEM::MirrorVertical()
     */
    virtual void MirrorVertical( const wxPoint& aCenter );

    /**
     * @copydoc LIB_ITEM::Rotate(const wxPoint&,bool)
     */
    virtual void Rotate( const wxPoint& aCenter, bool aRotateCCW = true );

    /**
     * @copydoc LIB_ITEM::Plot()
     */
    virtual void Plot( PLOTTER* aPlotter, const wxPoint& aOffset, bool aFill,
                       const TRANSFORM& aTransform );

    /**
     * @copydoc LIB_ITEM::GetWidth()
     */
    virtual int GetWidth() const { return m_Width; }

    /**
     * @copydoc LIB_ITEM::SetWidth()
     */
    virtual void SetWidth( int aWidth ) { m_Width = aWidth; }

    /** @copydoc EDA_ITEM::GetSelectMenuText() */
    virtual wxString GetSelectMenuText() const;

    /** @copydoc EDA_ITEM::GetMenuImage() */
    virtual BITMAP_DEF GetMenuImage() const { return  add_polygon_xpm; }

private:
    virtual EDA_ITEM* doClone() const;

    /**
     * Function compare
     * provides the polyline segment draw object specific comparison.
     *
     * The sort order for each polyline segment point is as follows:
     *      - Line segment point horizontal (X) position.
     *      - Line segment point vertical (Y) position.
     *
     * @param aOther A reference to the other #LIB_ITEM to compare the polyline against.
     * @return An integer value less than 0 if the polyline is less than \a aOther, zero
     *         if the polyline is equal to \a aOther, or greater than 0 if the polyline
     *         is greater than \a aOther.
     */
    virtual int compare( const LIB_ITEM& aOther ) const;
};


#endif   // _LIB_POLYLINE_H_
