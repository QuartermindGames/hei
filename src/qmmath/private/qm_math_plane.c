// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Plane math methods.
// Author:  Mark E. Sowden

#include <math.h>

#include "qmmath/public/qm_math_plane.h"

QmMathPlane *qm_math_plane_setup( QmMathPlane *self, const QmMathVector3f *p0, const QmMathVector3f *p1, const QmMathVector3f *p2 )
{
	QmMathVector3f s0 = qm_math_vector3f_sub( *p1, *p0 );
	QmMathVector3f s1 = qm_math_vector3f_sub( *p2, *p0 );

	self->normal = qm_math_vector3f_cross_product( s0, s1 );
	self->normal = qm_math_vector3f_normalize( self->normal );

	self->distance = -qm_math_vector3f_dot_product( self->normal, *p0 );

	return self;
}

float qm_math_plane_distance( const QmMathPlane *self, const QmMathVector3f *pos )
{
	return qm_math_vector3f_dot_product( self->normal, *pos ) + self->distance;
}

void qm_math_plane_basis_vectors( const QmMathPlane *self, QmMathVector3f *tangentDst, QmMathVector3f *bitangentDst )
{
	if ( fabsf( self->normal.x ) > fabsf( self->normal.y ) )
	{
		*tangentDst = qm_math_vector3f( self->normal.z, 0.0f, -self->normal.x );
	}
	else
	{
		*tangentDst = qm_math_vector3f( 0.0f, self->normal.z, -self->normal.y );
	}
	*tangentDst = qm_math_vector3f_normalize( *tangentDst );

	*bitangentDst = qm_math_vector3f_cross_product( self->normal, *tangentDst );
	*bitangentDst = qm_math_vector3f_normalize( *bitangentDst );
}

QmMathPlaneProjection qm_math_plane_compute_projection( const QmMathPlane *self )
{
	float nx = fabsf( self->normal.x );
	float ny = fabsf( self->normal.y );
	float nz = fabsf( self->normal.z );

	if ( ny > nx && ny < nz )
	{
		return COM_MATH_PLANE_PROJECTION_XZ;
	}
	if ( nz > nx && nz > ny )
	{
		return COM_MATH_PLANE_PROJECTION_XY;
	}

	return COM_MATH_PLANE_PROJECTION_YZ;
}

QmMathVector3f qm_math_plane_project_point( const QmMathPlane *self, const QmMathVector3f *point )
{
	float dist = qm_math_plane_distance( self, point );
	return qm_math_vector3f_add( *point, qm_math_vector3f_scale_float( self->normal, -dist ) );
}
