/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <plgraphics/plg.h>

PL_EXTERN_C

#define PLG_POLYGON_MAX_SIDES 32

#if !defined( PL_COMPILE_PLUGIN )

//TODO: all of this is going to go!

typedef struct PLGPolygon PLGPolygon;

extern void PlgGeneratePolygonNormals( PLGPolygon *polygon );

extern void PlgRemovePolygonVertex( PLGPolygon *polygon, unsigned int vertIndex );

extern unsigned int PlgGetNumOfPolygonTriangles( const PLGPolygon *polygon );
extern unsigned int *PlgConvertPolygonToTriangles( const PLGPolygon *polygon, unsigned int *numTriangles );

#endif

PL_EXTERN_C_END
