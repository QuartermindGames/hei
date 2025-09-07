/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_math.h>

PL_DLL const QmMathVector2f pl_vecOrigin2 = {};
PL_DLL const QmMathVector3f pl_vecOrigin3 = {};
PL_DLL const QmMathVector4f pl_vecOrigin4 = {};

/****************************************
 * Transform
 ****************************************/

QmMathVector3f PlTransformVector3( const QmMathVector3f *v, const PLMatrix4 *m )
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

QmMathVector4f PlTransformVector4( const QmMathVector4f *v, const PLMatrix4 *m )
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

/**
 * Function that works similarly to D3DXPlaneDotCoord.
 */
float PlGetPlaneDotProduct( const QmMathVector4f *plane, const QmMathVector3f *vector )
{
	return plane->x * vector->x + plane->y * vector->y + plane->z * vector->z + plane->w * 1.0f;
}

QmMathVector3f PlVector3Max( QmMathVector3f v, QmMathVector3f v2 )
{
	return qm_math_vector3f(
	        v.x > v2.x ? v.x : v2.x,
	        v.y > v2.y ? v.y : v2.y,
	        v.z > v2.z ? v.z : v2.z );
}

/**
 * Function that works similarly to D3DXPlaneNormalize.
 */
QmMathVector4f PlNormalizePlane( QmMathVector4f plane )
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

/****************************************
 * Special
 ****************************************/

QmMathVector2f PlComputeLineNormal( const QmMathVector2f *x, const QmMathVector2f *y )
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
