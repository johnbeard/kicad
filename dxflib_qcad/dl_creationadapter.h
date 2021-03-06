/****************************************************************************
** Copyright (C) 2001-2013 RibbonSoft, GmbH. All rights reserved.
**
** This file is part of the dxflib project.
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Licensees holding valid dxflib Professional Edition licenses may use
** this file in accordance with the dxflib Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.ribbonsoft.com for further details.
**
** Contact info@ribbonsoft.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef DL_CREATIONADAPTER_H
#define DL_CREATIONADAPTER_H

#include "dl_global.h"

#include "dl_creationinterface.h"

/**
 * An abstract adapter class for receiving DXF events when a DXF file is being read.
 * The methods in this class are empty. This class exists as convenience for creating
 * listener objects.
 *
 * @author Andrew Mustun
 */
class DXFLIB_EXPORT DL_CreationAdapter : public DL_CreationInterface
{
public:
    DL_CreationAdapter() {}
    virtual ~DL_CreationAdapter() {}

    virtual void processCodeValuePair( unsigned int, const std::string& ) override {}
    virtual void endSection() override {}
    virtual void addLayer( const DL_LayerData& ) override {}
    virtual void addLinetype( const DL_LinetypeData& ) override {}
    virtual void addLinetypeDash( double ) override {}
    virtual void addBlock( const DL_BlockData& ) override {}
    virtual void endBlock() override {}
    virtual void addTextStyle( const DL_StyleData& ) override {}
    virtual void addPoint( const DL_PointData& ) override {}
    virtual void addLine( const DL_LineData& ) override {}
    virtual void addXLine( const DL_XLineData& ) override {}
    virtual void addRay( const DL_RayData& ) override {}

    virtual void addArc( const DL_ArcData& ) override {}
    virtual void addCircle( const DL_CircleData& ) override {}
    virtual void addEllipse( const DL_EllipseData& ) override {}

    virtual void addPolyline( const DL_PolylineData& ) override {}
    virtual void addVertex( const DL_VertexData& ) override {}

    virtual void addSpline( const DL_SplineData& ) override {}
    virtual void addControlPoint( const DL_ControlPointData& ) override {}
    virtual void addFitPoint( const DL_FitPointData& ) override {}
    virtual void addKnot( const DL_KnotData& ) override {}

    virtual void addInsert( const DL_InsertData& ) override {}

    virtual void addMText( const DL_MTextData& ) override {}
    virtual void addMTextChunk( const std::string& ) override {}
    virtual void addText( const DL_TextData& ) override {}
    virtual void addArcAlignedText( const DL_ArcAlignedTextData& ) override {}
    virtual void addAttribute( const DL_AttributeData& ) override {}

    virtual void addDimAlign( const DL_DimensionData&,
            const DL_DimAlignedData& ) override {}
    virtual void addDimLinear( const DL_DimensionData&,
            const DL_DimLinearData& ) override {}
    virtual void addDimRadial( const DL_DimensionData&,
            const DL_DimRadialData& ) override {}
    virtual void addDimDiametric( const DL_DimensionData&,
            const DL_DimDiametricData& ) override {}
    virtual void addDimAngular( const DL_DimensionData&,
            const DL_DimAngularData& ) override {}
    virtual void addDimAngular3P( const DL_DimensionData&,
            const DL_DimAngular3PData& ) override {}
    virtual void addDimOrdinate( const DL_DimensionData&,
            const DL_DimOrdinateData& ) override {}
    virtual void addLeader( const DL_LeaderData& ) override {}
    virtual void addLeaderVertex( const DL_LeaderVertexData& ) override {}

    virtual void addHatch( const DL_HatchData& ) override {}

    virtual void addTrace( const DL_TraceData& ) override {}
    virtual void add3dFace( const DL_3dFaceData& ) override {}
    virtual void addSolid( const DL_SolidData& ) override {}

    virtual void addImage( const DL_ImageData& ) override {}
    virtual void linkImage( const DL_ImageDefData& ) override {}
    virtual void addHatchLoop( const DL_HatchLoopData& ) override {}
    virtual void addHatchEdge( const DL_HatchEdgeData& ) override {}

    virtual void addXRecord( const std::string& ) override {}
    virtual void addXRecordString( int, const std::string& ) override {}
    virtual void addXRecordReal( int, double ) override {}
    virtual void addXRecordInt( int, int ) override {}
    virtual void addXRecordBool( int, bool ) override {}

    virtual void addXDataApp( const std::string& ) override {}
    virtual void addXDataString( int, const std::string& ) override {}
    virtual void addXDataReal( int, double ) override {}
    virtual void addXDataInt( int, int ) override {}

    virtual void addDictionary( const DL_DictionaryData& ) override {}
    virtual void addDictionaryEntry( const DL_DictionaryEntryData& ) override {}

    virtual void endEntity() override {}

    virtual void addComment( const std::string& ) override {}

    virtual void setVariableVector( const std::string&, double, double, double, int ) override {}
    virtual void setVariableString( const std::string&, const std::string&, int ) override {}
    virtual void setVariableInt( const std::string&, int, int ) override {}
    virtual void setVariableDouble( const std::string&, double, int ) override {}
#ifdef DL_COMPAT
    virtual void setVariableVector( const char*, double, double, double, int ) {}
    virtual void setVariableString( const char*, const char*, int ) {}
    virtual void setVariableInt( const char*, int, int ) {}
    virtual void setVariableDouble( const char*, double, int ) {}
    virtual void processCodeValuePair( unsigned int, char* ) {}
    virtual void addComment( const char* ) {}
    virtual void addMTextChunk( const char* ) {}
#endif
    virtual void endSequence() override {}
};

#endif
