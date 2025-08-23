/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <stdbool.h>

#include "qmmath/public/qm_math_vector.h"

typedef QmMathVector2f PLVector2;
typedef QmMathVector3f PLVector3;
typedef QmMathVector4f PLVector4;

#define PL_VEC2TO3( X ) PL_VECTOR3( X.x, X.y, 0.0f )
#define PL_VEC2TO4( X ) PL_VECTOR4( X.x, X.y, 0.0f, 0.0f )

#define PL_VEC3TO2( X ) PL_VECTOR2( X.x, X.y )
#define PL_VEC3TO4( X ) PL_VECTOR4( X.x, X.y, X.z, 0.0f )

#define PL_VEC4TO2( X ) PL_VECTOR2( X.x, X.y )
#define PL_VEC4TO3( X ) PL_VECTOR3( X.x, X.y, X.z )

/* these will give you the number of elements in each type
 * of vector. 												*/
#define PL_MVECNUM( x ) ( sizeof( x ) / sizeof( float ) )
#ifndef __cplusplus
PL_STATIC_ASSERT( ( PL_MVECNUM( PLVector2 ) == 2 ), "unexpected vector element num" );
PL_STATIC_ASSERT( PL_MVECNUM( PLVector3 ) == 3, "unexpected vector element num" );
PL_STATIC_ASSERT( PL_MVECNUM( PLVector4 ) == 4, "unexpected vector element num" );
#endif

#define PL_VECTOR_I( VECTOR, INDEX ) ( ( float * ) &( VECTOR ) )[ INDEX ]
/* todo: add bound checking to the below implementation??? Or just remove!? */
#define PL_VECTOR3_I( VECTOR, INDEX ) PL_VECTOR_I( VECTOR, INDEX )

/****************************************
 ****************************************/

extern PL_DLL const PLVector2 pl_vecOrigin2;
extern PL_DLL const PLVector3 pl_vecOrigin3;
extern PL_DLL const PLVector4 pl_vecOrigin4;

typedef struct PLMatrix4 PLMatrix4;

PL_DEPRECATED( PLVector3 PlTransformVector3( const PLVector3 *v, const PLMatrix4 *m ) );
PL_DEPRECATED( PLVector4 PlTransformVector4( const PLVector4 *v, const PLMatrix4 *m ) );
PL_DEPRECATED( PLVector2 PlAddVector2( PLVector2 v, PLVector2 v2 ) );
PL_DEPRECATED( PLVector3 PlAddVector3( PLVector3 v, PLVector3 v2 ) );
PL_DEPRECATED( PLVector3 PlAddVector3F( PLVector3 v, float f ) );
PL_DEPRECATED( PLVector4 PlAddVector4( PLVector4 v, PLVector4 v2 ) );
PL_DEPRECATED( PLVector2 PlDivideVector2( PLVector2 v, PLVector2 v2 ) );
PL_DEPRECATED( PLVector2 PlDivideVector2F( const PLVector2 *v, float f ) );
PL_DEPRECATED( PLVector3 PlDivideVector3( PLVector3 v, PLVector3 v2 ) );
PL_DEPRECATED( PLVector3 PlDivideVector3F( PLVector3 v, float v2 ) );
PL_DEPRECATED( PLVector4 PlDivideVector4F( PLVector4 v, float v2 ) );
PL_DEPRECATED( PLVector2 PlSubtractVector2( const PLVector2 *a, const PLVector2 *b ) );
PL_DEPRECATED( PLVector3 PlSubtractVector3( PLVector3 v, PLVector3 v2 ) );
PL_DEPRECATED( PLVector3 PlSubtractVector3F( PLVector3 v, float f ) );
PL_DEPRECATED( PLVector2 PlScaleVector2( const PLVector2 *v, const PLVector2 *scale ) );
PL_DEPRECATED( PLVector2 PlScaleVector2F( const PLVector2 *v, float scale ) );
PL_DEPRECATED( PLVector3 PlScaleVector3( PLVector3 v, PLVector3 v2 ) );
PL_DEPRECATED( PLVector3 PlScaleVector3F( PLVector3 v, float f ) );
PL_DEPRECATED( PLVector4 PlScaleVector4( const PLVector4 *v, const PLVector4 *scale ) );
PL_DEPRECATED( PLVector4 PlScaleVector4F( const PLVector4 *v, float scale ) );
PL_DEPRECATED( PLVector3 PlInverseVector3( PLVector3 v ) );
PL_DEPRECATED( float PlVector2DotProduct( const PLVector2 *a, const PLVector2 *b ) );
PL_DEPRECATED( float PlVector3DotProduct( PLVector3 v, PLVector3 v2 ) );
PL_DEPRECATED( float PlVector4DotProduct( const PLVector4 *a, const PLVector4 *b ) );
PL_DEPRECATED( float PlGetPlaneDotProduct( const PLVector4 *plane, const PLVector3 *vector ) );
PL_DEPRECATED( PLVector3 PlVector3CrossProduct( PLVector3 v, PLVector3 v2 ) );
PL_DEPRECATED( PLVector3 PlVector3Min( PLVector3 v, PLVector3 v2 ) );
PL_DEPRECATED( PLVector3 PlVector3Max( PLVector3 v, PLVector3 v2 ) );
PL_DEPRECATED( float PlGetVector2Length( const PLVector2 *v ) );
PL_DEPRECATED( float PlVector3Length( const PLVector3 v ) );

static inline float PlVector2Distance( const PLVector2 *a, const PLVector2 *b )
{
	return qm_math_vector2f_distance( *a, *b );
}

static inline float PlVector3Distance( const PLVector3 *a, const PLVector3 *b )
{
	return qm_math_vector3f_distance( *a, *b );
}

PL_DEPRECATED( PLVector2 plNormalizeVector2( const PLVector2 *v ) );
PL_DEPRECATED( PLVector3 PlNormalizeVector3( PLVector3 v ) );

PLVector4 PlNormalizePlane( PLVector4 plane );

PL_DEPRECATED( bool PlCompareVector2( const PLVector2 *v, const PLVector2 *v2 ) );
PL_DEPRECATED( bool PlCompareVector3( const PLVector3 *v, const PLVector3 *v2 ) );
PL_DEPRECATED( bool PlCompareVector4( const PLVector4 *v, const PLVector4 *v2 ) );
PL_DEPRECATED( PLVector2 PlClampVector2( const PLVector2 *v, float min, float max ) );
PL_DEPRECATED( PLVector3 PlClampVector3( const PLVector3 *v, float min, float max ) );
PL_DEPRECATED( PLVector4 PlClampVector4( const PLVector4 *v, float min, float max ) );
PL_DEPRECATED( const char *PlPrintVector2( const PLVector2 *v, PLVariableType format ) );
PL_DEPRECATED( const char *PlPrintVector3( const PLVector3 *v, PLVariableType format ) );
PL_DEPRECATED( const char *PlPrintVector4( const PLVector4 *v, PLVariableType format ) );

PLVector2 PlComputeLineNormal( const PLVector2 *x, const PLVector2 *y );

bool PlIsVectorNaN( float *v, uint8_t numElements );

inline static bool PlIsVector2NaN( const PLVector2 *v ) { return PlIsVectorNaN( ( float * ) v, 2 ); }
inline static bool PlIsVector3NaN( const PLVector3 *v ) { return PlIsVectorNaN( ( float * ) v, 3 ); }
inline static bool PlIsVector4NaN( const PLVector4 *v ) { return PlIsVectorNaN( ( float * ) v, 4 ); }
