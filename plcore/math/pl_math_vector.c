/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_math.h>

PL_DLL const PLVector2 pl_vecOrigin2 = {};
PL_DLL const PLVector3 pl_vecOrigin3 = {};
PL_DLL const PLVector4 pl_vecOrigin4 = {};

/****************************************
 * Transform
 ****************************************/

PLVector3 PlTransformVector3( const PLVector3 *v, const PLMatrix4 *m )
{
	return qm_math_vector3f(
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
	                ( m->pl_m4pos( 2, 3 ) * 1.0 ) );
}

PLVector4 PlTransformVector4( const PLVector4 *v, const PLMatrix4 *m )
{
	return qm_math_vector4f(
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


PLVector2 PlAddVector2( PLVector2 v, PLVector2 v2 ) { return qm_math_vector2f_add( v, v2 ); }
PLVector3 PlAddVector3( PLVector3 v, PLVector3 v2 ) { return qm_math_vector3f_add( v, v2 ); }
PLVector3 PlAddVector3F( PLVector3 v, float f ) { return qm_math_vector3f_add_float( v, f ); }
PLVector4 PlAddVector4( PLVector4 v, PLVector4 v2 ) { return qm_math_vector4f_add( v, v2 ); }

PLVector2 PlDivideVector2( PLVector2 v, PLVector2 v2 ) { return qm_math_vector2f_div( v, v2 ); }
PLVector2 PlDivideVector2F( const PLVector2 *v, float f ) { return qm_math_vector2f_div_float( *v, f ); }
PLVector3 PlDivideVector3( PLVector3 v, PLVector3 v2 ) { return qm_math_vector3f_div( v, v2 ); }
PLVector3 PlDivideVector3F( PLVector3 v, float v2 ) { return qm_math_vector3f_div_float( v, v2 ); }
PLVector4 PlDivideVector4F( PLVector4 v, float v2 ) { return qm_math_vector4f_div_float( v, v2 ); }

PLVector2 PlSubtractVector2( const PLVector2 *a, const PLVector2 *b ) { return qm_math_vector2f_sub( *a, *b ); }
PLVector3 PlSubtractVector3( PLVector3 v, PLVector3 v2 ) { return qm_math_vector3f_sub( v, v2 ); }
PLVector3 PlSubtractVector3F( PLVector3 v, float f ) { return qm_math_vector3f_sub( v, qm_math_vector3f( f, f, f ) ); }

PLVector2 PlScaleVector2( const PLVector2 *v, const PLVector2 *scale ) { return qm_math_vector2f_scale( *v, *scale ); }
PLVector2 PlScaleVector2F( const PLVector2 *v, float scale ) { return qm_math_vector2f_scale_float( *v, scale ); }
PLVector3 PlScaleVector3( PLVector3 v, PLVector3 v2 ) { return qm_math_vector3f_scale( v, v2 ); }
PLVector3 PlScaleVector3F( PLVector3 v, float f ) { return qm_math_vector3f_scale_float( v, f ); }
PLVector4 PlScaleVector4( const PLVector4 *v, const PLVector4 *scale ) { return qm_math_vector4f_scale( *v, *scale ); }
PLVector4 PlScaleVector4F( const PLVector4 *v, float scale ) { return qm_math_vector4f_scale_float( *v, scale ); }

/****************************************
 * Inverse
 ****************************************/

PLVector3 PlInverseVector3( PLVector3 v )
{
	v.x = -v.x;
	v.y = -v.y;
	v.z = -v.z;
	return v;
}

float PlVector2DotProduct( const PLVector2 *a, const PLVector2 *b ) { return qm_math_vector2f_dot_product( *a, *b ); }
float PlVector3DotProduct( PLVector3 v, PLVector3 v2 ) { return qm_math_vector3f_dot_product( v, v2 ); }
float PlVector4DotProduct( const PLVector4 *a, const PLVector4 *b ) { return qm_math_vector4f_dot_product( *a, *b ); }

/**
 * Function that works similarly to D3DXPlaneDotCoord.
 */
float PlGetPlaneDotProduct( const PLVector4 *plane, const PLVector3 *vector )
{
	return plane->x * vector->x + plane->y * vector->y + plane->z * vector->z + plane->w * 1.0f;
}

PLVector3 PlVector3CrossProduct( PLVector3 v, PLVector3 v2 ) { return qm_math_vector3f_cross_product( v, v2 ); }
PLVector3 PlVector3Min( PLVector3 v, PLVector3 v2 ) { return qm_math_vector3f_min( v, v2 ); }

PLVector3 PlVector3Max( PLVector3 v, PLVector3 v2 )
{
	return qm_math_vector3f(
	        v.x > v2.x ? v.x : v2.x,
	        v.y > v2.y ? v.y : v2.y,
	        v.z > v2.z ? v.z : v2.z );
}

float PlGetVector2Length( const PLVector2 *v ) { return qm_math_vector2f_length( *v ); }
float PlVector3Length( const PLVector3 v ) { return qm_math_vector3f_length( v ); }

PLVector2 plNormalizeVector2( const PLVector2 *v ) { return qm_math_vector2f_normalize( *v ); }
PLVector3 PlNormalizeVector3( PLVector3 v ) { return qm_math_vector3f_normalize( v ); }

/**
 * Function that works similarly to D3DXPlaneNormalize.
 */
PLVector4 PlNormalizePlane( PLVector4 plane )
{
	const float l = qm_math_vector3f_length( qm_math_vector3f( plane.x, plane.y, plane.z ) );
	if ( l != 0.f )
	{
		plane.x /= l;
		plane.y /= l;
		plane.z /= l;
		plane.w /= l;
	}

	return plane;
}

bool PlCompareVector2( const PLVector2 *v, const PLVector2 *v2 ) { return qm_math_vector2f_compare( *v, *v2 ); }
bool PlCompareVector3( const PLVector3 *v, const PLVector3 *v2 ) { return qm_math_vector3f_compare( *v, *v2 ); }
bool PlCompareVector4( const PLVector4 *v, const PLVector4 *v2 ) { return qm_math_vector4f_compare( *v, *v2 ); }

PLVector2 PlClampVector2( const PLVector2 *v, float min, float max ) { return qm_math_vector2f_clamp( *v, min, max ); }
PLVector3 PlClampVector3( const PLVector3 *v, float min, float max ) { return qm_math_vector3f_clamp( *v, min, max ); }
PLVector4 PlClampVector4( const PLVector4 *v, float min, float max ) { return qm_math_vector4f_clamp( *v, min, max ); }

const char *PlPrintVector2( const PLVector2 *v, PLVariableType format )
{
	static char s[ 64 ];
	return qm_math_vector2f_print( *v, s, sizeof( s ) );
}

const char *PlPrintVector3( const PLVector3 *v, PLVariableType format )
{
	static char s[ 64 ];
	return qm_math_vector3f_print( *v, s, sizeof( s ) );
}

const char *PlPrintVector4( const PLVector4 *v, PLVariableType format )
{
	static char s[ 64 ];
	return qm_math_vector4f_print( *v, s, sizeof( s ) );
}

/****************************************
 * Special
 ****************************************/

PLVector2 PlComputeLineNormal( const PLVector2 *x, const PLVector2 *y )
{
	return qm_math_vector2f_normalize( qm_math_vector2f( x->y - y->y, y->x - x->x ) );
}

bool PlIsVectorNaN( float *v, uint8_t numElements )
{
	for ( uint8_t i = 0; i < numElements; ++i )
	{
		if ( !isnan( v[ i ] ) )
		{
			continue;
		}

		return true;
	}

	return false;
}
