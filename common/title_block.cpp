/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2018 Kicad Developers, see AUTHORS.txt for contributors.
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

#include <title_block.h>


void TITLE_BLOCK::ImportFrom( const TITLE_BLOCK& aOther,
    const TITLE_BLOCK_EXPORT_OPTIONS& aExportOptions )
{
    if( aExportOptions.ShouldExport( TITLE_BLOCK::FIELD::REVISION ) )
        SetRevision( aOther.GetRevision() );

    if( aExportOptions.ShouldExport( TITLE_BLOCK::FIELD::DATE ) )
        SetDate( aOther.GetDate() );

    if( aExportOptions.ShouldExport( TITLE_BLOCK::FIELD::TITLE ) )
        SetTitle( aOther.GetTitle() );

    if( aExportOptions.ShouldExport( TITLE_BLOCK::FIELD::COMPANY ) )
        SetCompany( aOther.GetCompany() );

    for( int i = 0; i < TITLE_BLOCK::max_comments; ++i )
    {
        if( aExportOptions.ShouldExport( TITLE_BLOCK::FIELD::COMMENT, i ) )
            SetComment( i, aOther.GetComment( i ) );
    }
}


void TITLE_BLOCK_EXPORT_OPTIONS::SetShouldExport( TITLE_BLOCK::FIELD aField,
    int aIndex, bool aExport )
{
    if( aExport )
        m_exports.insert( { aField, aIndex } );
    else
        m_exports.erase( { aField, aIndex } );
}


bool TITLE_BLOCK_EXPORT_OPTIONS::ShouldExport( TITLE_BLOCK::FIELD aField,
    int aIndex ) const
{
    return m_exports.count( { aField, aIndex } ) != 0;
}