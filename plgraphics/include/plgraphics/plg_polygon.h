// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plgraphics/plg.h>

PL_EXTERN_C

#define PLG_POLYGON_MAX_SIDES 32

#if !defined( PL_COMPILE_PLUGIN )

extern PLGPolygon *PlgCreatePolygon( PLGTexture *texture, PLVector2 textureOffset, PLVector2 textureScale, float textureRotation );
extern void PlgDestroyPolygon( PLGPolygon *polygon );

extern void PlgGeneratePolygonNormals( PLGPolygon *polygon );

extern void PlgAddPolygonVertex( PLGPolygon *polygon, const PLGVertex *vertex );
extern void PlgRemovePolygonVertex( PLGPolygon *polygon, unsigned int vertIndex );

extern unsigned int PlgGetNumOfPolygonVertices( const PLGPolygon *polygon );
extern PLGVertex *PlgGetPolygonVertex( PLGPolygon *polygon, unsigned int vertIndex );
extern PLGVertex *PlgGetPolygonVertices( PLGPolygon *polygon, unsigned int *numVertices );
extern PLGTexture *PlgGetPolygonTexture( PLGPolygon *polygon );
extern PLVector3 PlgGetPolygonFaceNormal( const PLGPolygon *polygon );
extern unsigned int PlgGetNumOfPolygonTriangles( const PLGPolygon *polygon );
extern unsigned int *PlgConvertPolygonToTriangles( const PLGPolygon *polygon, unsigned int *numTriangles );
extern PLGMesh *PlgConvertPolygonToMesh( const PLGPolygon *polygon );

#endif

PL_EXTERN_C_END
