/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_math.h>

PL_DLL const PLVector2 pl_vecOrigin2 = { 0.0f, 0.0f };
PL_DLL const PLVector3 pl_vecOrigin3 = { 0.0f, 0.0f, 0.0f };
PL_DLL const PLVector4 pl_vecOrigin4 = { 0.0f, 0.0f, 0.0f, 0.0f };

/****************************************
 * Transform
 ****************************************/

PLVector3 PlTransformVector3( const PLVector3 *v, const PLMatrix4 *m ) {
	return ( PLVector3 ){
	        ( m->pl_m4pos( 0, 0 ) * v->x ) +
	                ( m->pl_m4pos( 0, 1 ) * v->y ) +
	                ( m->pl_m4pos( 0, 2 ) * v->z ) +
	                ( m->pl_m4pos( 0, 3 ) * 1.0 ),
	        ( m->pl_m4pos( 1, 0 ) * v->x ) +
	                ( m->pl_m4pos( 1, 1 ) * v->y ) +
	                ( m->pl_m4pos( 1, 2 ) * v->z ) +
	                ( m->pl_m4pos( 1, 3 ) * 1.0 ),
	        ( m->pl_m4pos( 2, 0 ) * v->x ) +
	                ( m->pl_m4pos( 2, 1 ) * v->y ) +
	                ( m->pl_m4pos( 2, 2 ) * v->z ) +
	                ( m->pl_m4pos( 2, 3 ) * 1.0 ),
	};
}

PLVector4 PlTransformVector4( const PLVector4 *v, const PLMatrix4 *m ) {
	return PL_VECTOR4(
	        /* x */
	        ( m->pl_m4pos( 0, 0 ) * v->x ) +
	                ( m->pl_m4pos( 0, 1 ) * v->y ) +
	                ( m->pl_m4pos( 0, 2 ) * v->z ) +
	                ( m->pl_m4pos( 0, 3 ) * v->w ),
	        /* y */
	        ( m->pl_m4pos( 1, 0 ) * v->x ) +
	                ( m->pl_m4pos( 1, 1 ) * v->y ) +
	                ( m->pl_m4pos( 1, 2 ) * v->z ) +
	                ( m->pl_m4pos( 1, 3 ) * v->w ),
	        /* z */
	        ( m->pl_m4pos( 2, 0 ) * v->x ) +
	                ( m->pl_m4pos( 2, 1 ) * v->y ) +
	                ( m->pl_m4pos( 2, 2 ) * v->z ) +
	                ( m->pl_m4pos( 2, 3 ) * v->w ),
	        /* w */
	        ( m->pl_m4pos( 3, 0 ) * v->x ) +
	                ( m->pl_m4pos( 3, 1 ) * v->y ) +
	                ( m->pl_m4pos( 3, 2 ) * v->z ) +
	                ( m->pl_m4pos( 3, 3 ) * v->w ) );
}

/****************************************
 * Addition
 ****************************************/

PLVector2 PlAddVector2( PLVector2 v, PLVector2 v2 ) {
	v.x += v2.x;
	v.y += v2.y;
	return v;
}

PLVector3 PlAddVector3( PLVector3 v, PLVector3 v2 ) {
	v.x += v2.x;
	v.y += v2.y;
	v.z += v2.z;
	return v;
}

PLVector3 PlAddVector3F( PLVector3 v, float f ) {
	v.x += f;
	v.y += f;
	v.z += f;
	return v;
}

PLVector4 PlAddVector4( PLVector4 v, PLVector4 v2 ) {
	v.x += v2.x;
	v.y += v2.y;
	v.z += v2.z;
	v.w += v2.w;
	return v;
}

/****************************************
 * Division
 ****************************************/

PLVector2 PlDivideVector2( PLVector2 v, PLVector2 v2 ) {
	v.x /= v2.x;
	v.y /= v2.y;
	return v;
}

PLVector2 PlDivideVector2F( const PLVector2 *v, float f ) {
	return PL_VECTOR2( v->x / f, v->y / f );
}

PLVector3 PlDivideVector3( PLVector3 v, PLVector3 v2 ) {
	v.x /= v2.x;
	v.y /= v2.y;
	v.z /= v2.z;
	return v;
}

PLVector3 PlDivideVector3F( PLVector3 v, float v2 ) {
	v.x /= v2;
	v.y /= v2;
	v.z /= v2;
	return v;
}

PLVector4 PlDivideVector4F( PLVector4 v, float v2 ) {
	v.x /= v2;
	v.y /= v2;
	v.z /= v2;
	v.w /= v2;
	return v;
}

/****************************************
 * Subtract
 ****************************************/

PLVector2 PlSubtractVector2( const PLVector2 *a, const PLVector2 *b ) {
	return PL_VECTOR2( a->x - b->x, a->y - b->y );
}

PLVector3 PlSubtractVector3( PLVector3 v, PLVector3 v2 ) {
	v.x -= v2.x;
	v.y -= v2.y;
	v.z -= v2.z;
	return v;
}

PLVector3 PlSubtractVector3F( PLVector3 v, float f ) {
	return PlSubtractVector3( v, PL_VECTOR3( f, f, f ) );
}

/****************************************
 * Scale
 ****************************************/

PLVector2 PlScaleVector2( const PLVector2 *v, const PLVector2 *scale ) {
	return PL_VECTOR2( v->x * scale->x, v->y * scale->y );
}

PLVector2 PlScaleVector2F( const PLVector2 *v, float scale ) {
	return PL_VECTOR2( v->x * scale, v->y * scale );
}

PLVector3 PlScaleVector3( PLVector3 v, PLVector3 v2 ) {
	v.x *= v2.x;
	v.y *= v2.y;
	v.z *= v2.z;
	return v;
}

PLVector3 PlScaleVector3F( PLVector3 v, float f ) {
	v.x *= f;
	v.y *= f;
	v.z *= f;
	return v;
}

/****************************************
 * Inverse
 ****************************************/

PLVector3 PlInverseVector3( PLVector3 v ) {
	v.x = -v.x;
	v.y = -v.y;
	v.z = -v.z;
	return v;
}

/****************************************
 * Dot Product
 ****************************************/

float PlVector2DotProduct( const PLVector2 *a, const PLVector2 *b ) {
	return a->x * b->x + a->y * b->y;
}

float PlVector3DotProduct( PLVector3 v, PLVector3 v2 ) {
	return ( v.x * v2.x + v.y * v2.y + v.z * v2.z );
}

/**
 * Function that works similarly to D3DXPlaneDotCoord.
 */
float PlGetPlaneDotProduct( const PLVector4 *plane, const PLVector3 *vector ) {
	return plane->x * vector->x + plane->y * vector->y + plane->z * vector->z + plane->w * 1.0f;
}

/****************************************
 * Cross Product
 ****************************************/

PLVector3 PlVector3CrossProduct( PLVector3 v, PLVector3 v2 ) {
	return PL_VECTOR3(
	        v.y * v2.z - v.z * v2.y,
	        v.z * v2.x - v.x * v2.z,
	        v.x * v2.y - v.y * v2.x );
}

/****************************************
 * Min
 ****************************************/

PLVector3 PlVector3Min( PLVector3 v, PLVector3 v2 ) {
	return PL_VECTOR3(
	        v.x < v2.x ? v.x : v2.x,
	        v.y < v2.y ? v.y : v2.y,
	        v.z < v2.z ? v.z : v2.z );
}

/****************************************
 * Max
 ****************************************/

PLVector3 PlVector3Max( PLVector3 v, PLVector3 v2 ) {
	return PL_VECTOR3(
	        v.x > v2.x ? v.x : v2.x,
	        v.y > v2.y ? v.y : v2.y,
	        v.z > v2.z ? v.z : v2.z );
}

/****************************************
 * Length
 ****************************************/

float PlGetVector2Length( const PLVector2 *v ) {
	return sqrtf( PlVector2DotProduct( v, v ) );
}

float PlVector3Length( const PLVector3 v ) {
	return sqrtf( PlVector3DotProduct( v, v ) );
}

/****************************************
 * Normalize
 ****************************************/

PLVector2 plNormalizeVector2( const PLVector2 *v ) {
	float length = PlGetVector2Length( v );
	return PL_VECTOR2( v->x / length, v->y / length );
}

PLVector3 PlNormalizeVector3( PLVector3 v ) {
	float length = PlVector3Length( v );
	if ( length != 0 ) {
		v.x /= length;
		v.y /= length;
		v.z /= length;
	}
	return v;
}

/**
 * Function that works similarly to D3DXPlaneNormalize.
 */
PLVector4 PlNormalizePlane( PLVector4 plane ) {
	float l = PlVector3Length( PL_VECTOR3( plane.x, plane.y, plane.z ) );

	plane.x /= l;
	plane.y /= l;
	plane.z /= l;
	plane.w /= l;

	return plane;
}

/****************************************
 * Compare
 ****************************************/

bool PlCompareVector2( const PLVector2 *v, const PLVector2 *v2 ) {
	return ( ( v->x == v2->x ) && ( v->y == v2->y ) );
}

bool PlCompareVector3( const PLVector3 *v, const PLVector3 *v2 ) {
	return ( ( v->x == v2->x ) && ( v->y == v2->y ) && ( v->z == v2->z ) );
}

bool PlCompareVector4( const PLVector4 *v, const PLVector4 *v2 ) {
	for ( unsigned int i = 0; i < 4; ++i ) {
		if ( PL_VECTOR_I( *v, i ) == PL_VECTOR_I( *v2, i ) ) {
			continue;
		}

		return false;
	}

	return true;
}

/****************************************
 * Clamp
 ****************************************/

PLVector2 PlClampVector2( const PLVector2 *v, float min, float max ) {
	return PL_VECTOR2(
	        PlClamp( min, v->x, max ),
	        PlClamp( min, v->y, max ) );
}

PLVector3 PlClampVector3( const PLVector3 *v, float min, float max ) {
	return PL_VECTOR3(
	        PlClamp( min, v->x, max ),
	        PlClamp( min, v->y, max ),
	        PlClamp( min, v->z, max ) );
}

PLVector4 PlClampVector4( const PLVector4 *v, float min, float max ) {
	return PL_VECTOR4(
	        PlClamp( min, v->x, max ),
	        PlClamp( min, v->y, max ),
	        PlClamp( min, v->z, max ),
	        PlClamp( min, v->w, max ) );
}

/****************************************
 * Print
 ****************************************/

const char *PlPrintVector2( const PLVector2 *v, PLVariableType format ) {
	static char s[ 64 ];
	if ( format == PL_VAR_I32 )
		snprintf( s, sizeof( s ), "%i %i", ( int ) v->x, ( int ) v->y );
	else
		snprintf( s, sizeof( s ), "%f %f", v->x, v->y );
	return s;
}

const char *PlPrintVector3( const PLVector3 *v, PLVariableType format ) {
	static char s[ 64 ];
	if ( format == PL_VAR_I32 )
		snprintf( s, sizeof( s ), "%i %i %i", ( int ) v->x, ( int ) v->y, ( int ) v->z );
	else
		snprintf( s, sizeof( s ), "%f %f %f", v->x, v->y, v->z );
	return s;
}

const char *PlPrintVector4( const PLVector4 *v, PLVariableType format ) {
	static char s[ 64 ];
	if ( format == PL_VAR_I32 )
		snprintf( s, sizeof( s ), "%i %i %i %i", ( int ) v->x, ( int ) v->y, ( int ) v->z, ( int ) v->w );
	else
		snprintf( s, sizeof( s ), "%f %f %f %f", v->x, v->y, v->z, v->w );
	return s;
}

/****************************************
 * Special
 ****************************************/

PLVector2 PlComputeLineNormal( const PLVector2 *x, const PLVector2 *y ) {
	PLVector2 v = PL_VECTOR2( x->y - y->y, y->x - x->x );
	return plNormalizeVector2( &v );
}

bool PlIsVectorNaN( float *v, uint8_t numElements ) {
	for ( uint8_t i = 0; i < numElements; ++i ) {
		if ( !isnan( v[ i ] ) ) {
			continue;
		}

		return true;
	}

	return false;
}
