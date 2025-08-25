/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_physics.h>

#include <float.h>

/* todo: should be COMPUTE not GENERATE!!! */

QmMathVector3f PlGetAabbAbsOrigin( const PLCollisionAABB *bounds, QmMathVector3f origin )
{
	QmMathVector3f absOrigin = qm_math_vector3f( ( bounds->mins.x + bounds->maxs.x ) / 2,
	                                        ( bounds->mins.y + bounds->maxs.y ) / 2,
	                                        ( bounds->mins.z + bounds->maxs.z ) / 2 );
	return qm_math_vector3f_add( origin, absOrigin );
}

bool PlIsAabbIntersecting( const PLCollisionAABB *aBounds, const PLCollisionAABB *bBounds ) {
	QmMathVector3f aMax = qm_math_vector3f_add( aBounds->maxs, aBounds->origin );
	QmMathVector3f aMin = qm_math_vector3f_add( aBounds->mins, aBounds->origin );
	QmMathVector3f bMax = qm_math_vector3f_add( bBounds->maxs, bBounds->origin );
	QmMathVector3f bMin = qm_math_vector3f_add( bBounds->mins, bBounds->origin );

	return !(
	        aMax.x < bMin.x ||
	        aMax.y < bMin.y ||
	        aMax.z < bMin.z ||
	        aMin.x > bMax.x ||
	        aMin.y > bMax.y ||
	        aMin.z > bMax.z );
}

bool PlIsPointIntersectingAabb( const PLCollisionAABB *bounds, QmMathVector3f point ) {
	QmMathVector3f max = qm_math_vector3f_add( bounds->maxs, bounds->origin );
	QmMathVector3f min = qm_math_vector3f_add( bounds->mins, bounds->origin );

	return !(
	        point.x > max.x ||
	        point.x < min.x ||
	        point.y > max.y ||
	        point.y < min.y ||
	        point.z > max.z ||
	        point.z < min.z );
}

static QmMathVector3f FindClosestPointOnAabb( const PLCollisionAABB *bounds, const QmMathVector3f *point )
{
	return qm_math_vector3f(
	        PL_MAX( bounds->mins.x, PL_MIN( bounds->maxs.x, point->x ) ),
	        PL_MAX( bounds->mins.y, PL_MIN( bounds->maxs.y, point->y ) ),
	        PL_MAX( bounds->mins.z, PL_MIN( bounds->maxs.z, point->z ) ) );
}

bool PlIsSphereIntersectingAabb( const PLCollisionSphere *sphere, const PLCollisionAABB *bounds ) {
	QmMathVector3f point = FindClosestPointOnAabb( bounds, &sphere->origin );
	float distance = qm_math_vector3f_length( qm_math_vector3f_sub( point, sphere->origin ) );
	return ( distance <= sphere->radius );
}

bool PlIsSphereIntersecting( const PLCollisionSphere *aSphere, const PLCollisionSphere *bSphere ) {
	QmMathVector3f difference = qm_math_vector3f_sub( aSphere->origin, bSphere->origin );
	float distance = qm_math_vector3f_length( difference );
	float sum_radius = aSphere->radius + bSphere->radius;
	return distance < sum_radius;
}

PLCollision PlIsSphereIntersectingPlane( const PLCollisionSphere *sphere, const PLCollisionPlane *plane ) {
	PLCollision collision;
	PL_ZERO_( collision );

	QmMathVector3f pDist = qm_math_vector3f_sub( sphere->origin, plane->origin );

	/* distance from the sphere to the plane */
	float distance = qm_math_vector3f_dot_product( plane->normal, pDist ) - ( sphere->radius * 2.0f );

	collision.contactNormal = plane->normal;
	collision.contactPoint = qm_math_vector3f_scale_float( qm_math_vector3f_sub( sphere->origin, plane->normal ), distance );
	collision.penetration = -distance;

	return collision;
}

bool PlIsAabbIntersectingPlane( const PLCollisionAABB *aabb, const PLCollisionPlane *plane, PLCollision *result ) {
	float distance = qm_math_vector3f_dot_product( plane->normal, aabb->origin ) -
	                 qm_math_vector3f_dot_product( plane->normal, plane->origin );

	QmMathVector3f halfDimension;
	halfDimension.x = ( aabb->maxs.x - aabb->mins.x ) / 2.0f;
	halfDimension.y = ( aabb->maxs.y - aabb->mins.y ) / 2.0f;
	halfDimension.z = ( aabb->maxs.z - aabb->mins.z ) / 2.0f;

	float extents = fabsf( halfDimension.x * plane->normal.x ) +
	                fabsf( halfDimension.y * plane->normal.y ) +
	                fabsf( halfDimension.z * plane->normal.z );

	if ( distance > extents ) {
		*result = ( PLCollision ) {};
		result->penetration = extents - distance;
		result->contactNormal = plane->normal;
		result->contactPoint = qm_math_vector3f_scale_float( qm_math_vector3f_add( aabb->origin, plane->normal ), result->penetration );
		return true;
	}

	return false;
}

/* https://github.com/erich666/GraphicsGems/blob/master/gems/RayBox.c */
bool PlIsRayIntersectingAabb( const PLCollisionAABB *bounds, const PLCollisionRay *ray, QmMathVector3f *hitPoint ) {
	QmMathVector3f max = qm_math_vector3f_add( bounds->maxs, bounds->origin );
	QmMathVector3f min = qm_math_vector3f_add( bounds->mins, bounds->origin );

	/* Find candidate planes */

	static const unsigned char right = 0;
	static const unsigned char left = 1;
	static const unsigned char middle = 2;

	QmMathVector3f candidatePlane;
	unsigned char hitQuadrant[ 3 ];
	bool isInside = true;

	for ( unsigned int i = 0; i < 3; ++i ) {
		if ( PL_VECTOR3_I( ray->origin, i ) < PL_VECTOR3_I( min, i ) ) {
			hitQuadrant[ i ] = left;
			PL_VECTOR3_I( candidatePlane, i ) = PL_VECTOR3_I( min, i );
			isInside = false;
		} else if ( PL_VECTOR3_I( ray->origin, i ) > PL_VECTOR3_I( max, i ) ) {
			hitQuadrant[ i ] = right;
			PL_VECTOR3_I( candidatePlane, i ) = PL_VECTOR3_I( max, i );
			isInside = false;
		} else {
			hitQuadrant[ i ] = middle;
		}
	}

	/* Ray origin inside bounding box */
	if ( isInside ) {
		*hitPoint = ray->origin;
		return true;
	}

	/* Calculate T distances to candidate planes */
	QmMathVector3f maxT;
	for ( unsigned int i = 0; i < 3; ++i ) {
		if ( hitQuadrant[ i ] != middle && PL_VECTOR3_I( ray->direction, i ) != 0.0f ) {
			PL_VECTOR3_I( maxT, i ) = ( PL_VECTOR3_I( candidatePlane, i ) - PL_VECTOR3_I( ray->origin, i ) ) / PL_VECTOR3_I( ray->direction, i );
		} else {
			PL_VECTOR3_I( maxT, i ) = -1.0f;
		}
	}

	/* Get largest of the maxT's for final choice of intersection */
	unsigned int whichPlane = 0;
	for ( unsigned int i = 1; i < 3; ++i ) {
		if ( PL_VECTOR3_I( maxT, whichPlane ) < PL_VECTOR3_I( maxT, i ) ) {
			whichPlane = i;
		}
	}

	/* Check final candidate actually inside box */

	if ( PL_VECTOR3_I( maxT, whichPlane ) < 0.0f ) {
		return false;
	}

	for ( unsigned int i = 0; i < 3; ++i ) {
		if ( whichPlane != i ) {
			PL_VECTOR3_I( hitPoint, i ) = PL_VECTOR3_I( ray->origin, i ) + PL_VECTOR3_I( maxT, whichPlane ) * PL_VECTOR3_I( ray->direction, i );
			if ( PL_VECTOR3_I( hitPoint, i ) < PL_VECTOR3_I( min, i ) || PL_VECTOR3_I( hitPoint, i ) > PL_VECTOR3_I( max, i ) ) {
				return false;
			}
		} else {
			PL_VECTOR3_I( hitPoint, i ) = PL_VECTOR3_I( candidatePlane, i );
		}
	}

	return true;
}
