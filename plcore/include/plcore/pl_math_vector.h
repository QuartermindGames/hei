/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <stdbool.h>

#include "../../src/qmmath/public/qm_math_vector.h"

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
PL_STATIC_ASSERT( ( PL_MVECNUM( QmMathVector2f ) == 2 ), "unexpected vector element num" );
PL_STATIC_ASSERT( PL_MVECNUM( QmMathVector3f ) == 3, "unexpected vector element num" );
PL_STATIC_ASSERT( PL_MVECNUM( QmMathVector4f ) == 4, "unexpected vector element num" );
#endif

#define PL_VECTOR_I( VECTOR, INDEX ) ( ( float * ) &( VECTOR ) )[ INDEX ]
/* todo: add bound checking to the below implementation??? Or just remove!? */
#define PL_VECTOR3_I( VECTOR, INDEX ) PL_VECTOR_I( VECTOR, INDEX )

/****************************************
 ****************************************/

extern PL_DLL const QmMathVector2f pl_vecOrigin2;
extern PL_DLL const QmMathVector3f pl_vecOrigin3;
extern PL_DLL const QmMathVector4f pl_vecOrigin4;

typedef struct PLMatrix4 PLMatrix4;

PL_DEPRECATED( QmMathVector3f PlTransformVector3( const QmMathVector3f *v, const PLMatrix4 *m ) );
PL_DEPRECATED( QmMathVector4f PlTransformVector4( const QmMathVector4f *v, const PLMatrix4 *m ) );

PL_DEPRECATED( float PlGetPlaneDotProduct( const QmMathVector4f *plane, const QmMathVector3f *vector ) );
QmMathVector4f PlNormalizePlane( QmMathVector4f plane );

QmMathVector2f PlComputeLineNormal( const QmMathVector2f *x, const QmMathVector2f *y );

bool PlIsVectorNaN( float *v, uint8_t numElements );

inline static bool PlIsVector2NaN( const QmMathVector2f *v ) { return PlIsVectorNaN( ( float * ) v, 2 ); }
inline static bool PlIsVector3NaN( const QmMathVector3f *v ) { return PlIsVectorNaN( ( float * ) v, 3 ); }
inline static bool PlIsVector4NaN( const QmMathVector4f *v ) { return PlIsVectorNaN( ( float * ) v, 4 ); }
