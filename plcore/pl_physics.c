/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_physics.h>

#include <float.h>

float PlComputeSphereFromCoords( const PLVector3 *vertices, unsigned int numVertices ) {
	float maxRadius = 0.0f;
	for ( unsigned int i = 0; i < numVertices; ++i ) {
		float radius = PlVector3Length( vertices[ i ] );
		if ( radius > maxRadius ) {
			maxRadius = radius;
		}
	}

	return maxRadius;
}

/* todo: should be COMPUTE not GENERATE!!! */
PLCollisionAABB PlGenerateAabbFromCoords( const PLVector3 *vertices, unsigned int numVertices, bool absolute ) {
	PLCollisionAABB bounds;
	if ( absolute ) {
		bounds.maxs = ( PLVector3 ) { vertices[ 0 ].x, vertices[ 0 ].y, vertices[ 0 ].z };
		bounds.mins = ( PLVector3 ) { vertices[ 0 ].x, vertices[ 0 ].y, vertices[ 0 ].z };

		for ( unsigned int i = 0; i < numVertices; ++i ) {
			if ( bounds.maxs.x < vertices[ i ].x ) { bounds.maxs.x = vertices[ i ].x; }
			if ( bounds.maxs.y < vertices[ i ].y ) { bounds.maxs.y = vertices[ i ].y; }
			if ( bounds.maxs.z < vertices[ i ].z ) { bounds.maxs.z = vertices[ i ].z; }
			if ( bounds.mins.x > vertices[ i ].x ) { bounds.mins.x = vertices[ i ].x; }
			if ( bounds.mins.y > vertices[ i ].y ) { bounds.mins.y = vertices[ i ].y; }
			if ( bounds.mins.z > vertices[ i ].z ) { bounds.mins.z = vertices[ i ].z; }
		}
	} else {
		/* this technically still doesn't, but it's better */
		float max = FLT_MIN;
		float min = FLT_MAX;
		for ( unsigned int i = 0; i < numVertices; ++i ) {
			if ( vertices[ i ].x > max ) { max = vertices[ i ].x; }
			if ( vertices[ i ].y > max ) { max = vertices[ i ].y; }
			if ( vertices[ i ].z > max ) { max = vertices[ i ].z; }
			if ( vertices[ i ].x < min ) { min = vertices[ i ].x; }
			if ( vertices[ i ].y < min ) { min = vertices[ i ].y; }
			if ( vertices[ i ].z < min ) { min = vertices[ i ].z; }
		}

		if ( min < 0 ) { min *= -1; }
		if ( max < 0 ) { max *= -1; }

		float abs = min > max ? min : max;
		bounds.maxs = PL_VECTOR3( abs, abs, abs );
		bounds.mins = PL_VECTOR3( -abs, -abs, -abs );
	}

	/* abs origin is the middle of the bounding volume (wherever it is) and origin is the transformative point */
	bounds.absOrigin = PlGetAabbAbsOrigin( &bounds, pl_vecOrigin3 );
	bounds.origin = pl_vecOrigin3;

	return bounds;
}

PLVector3 PlGetAabbAbsOrigin( const PLCollisionAABB *bounds, PLVector3 origin ) {
	PLVector3 absOrigin = PL_VECTOR3( ( bounds->mins.x + bounds->maxs.x ) / 2,
	                                  ( bounds->mins.y + bounds->maxs.y ) / 2,
	                                  ( bounds->mins.z + bounds->maxs.z ) / 2 );
	return PlAddVector3( origin, absOrigin );
}

bool PlIsAabbIntersecting( const PLCollisionAABB *aBounds, const PLCollisionAABB *bBounds ) {
	PLVector3 aMax = PlAddVector3( aBounds->maxs, aBounds->origin );
	PLVector3 aMin = PlAddVector3( aBounds->mins, aBounds->origin );
	PLVector3 bMax = PlAddVector3( bBounds->maxs, bBounds->origin );
	PLVector3 bMin = PlAddVector3( bBounds->mins, bBounds->origin );

	return !(
	        aMax.x < bMin.x ||
	        aMax.y < bMin.y ||
	        aMax.z < bMin.z ||
	        aMin.x > bMax.x ||
	        aMin.y > bMax.y ||
	        aMin.z > bMax.z );
}

bool PlIsPointIntersectingAabb( const PLCollisionAABB *bounds, PLVector3 point ) {
	PLVector3 max = PlAddVector3( bounds->maxs, bounds->origin );
	PLVector3 min = PlAddVector3( bounds->mins, bounds->origin );

	return !(
	        point.x > max.x ||
	        point.x < min.x ||
	        point.y > max.y ||
	        point.y < min.y ||
	        point.z > max.z ||
	        point.z < min.z );
}

static PLVector3 FindClosestPointOnAabb( const PLCollisionAABB *bounds, const PLVector3 *point ) {
	return ( PLVector3 ) {
	        PL_MAX( bounds->mins.x, PL_MIN( bounds->maxs.x, point->x ) ),
	        PL_MAX( bounds->mins.y, PL_MIN( bounds->maxs.y, point->y ) ),
	        PL_MAX( bounds->mins.z, PL_MIN( bounds->maxs.z, point->z ) ) };
}

bool PlIsSphereIntersectingAabb( const PLCollisionSphere *sphere, const PLCollisionAABB *bounds ) {
	PLVector3 point = FindClosestPointOnAabb( bounds, &sphere->origin );
	float distance = PlVector3Length( PlSubtractVector3( point, sphere->origin ) );
	return ( distance <= sphere->radius );
}

bool PlIsSphereIntersecting( const PLCollisionSphere *aSphere, const PLCollisionSphere *bSphere ) {
	PLVector3 difference = PlSubtractVector3( aSphere->origin, bSphere->origin );
	float distance = PlVector3Length( difference );
	float sum_radius = aSphere->radius + bSphere->radius;
	return distance < sum_radius;
}

PLCollision PlIsSphereIntersectingPlane( const PLCollisionSphere *sphere, const PLCollisionPlane *plane ) {
	PLCollision collision;
	PL_ZERO_( collision );

	PLVector3 pDist = PlSubtractVector3( sphere->origin, plane->origin );

	/* distance from the sphere to the plane */
	float distance = PlVector3DotProduct( plane->normal, pDist ) - ( sphere->radius * 2.0f );

	collision.contactNormal = plane->normal;
	collision.contactPoint = PlScaleVector3F( PlSubtractVector3( sphere->origin, plane->normal ), distance );
	collision.penetration = -distance;

	return collision;
}

bool PlIsAabbIntersectingPlane( const PLCollisionAABB *aabb, const PLCollisionPlane *plane, PLCollision *result ) {
	float distance = PlVector3DotProduct( plane->normal, aabb->origin ) -
	                 PlVector3DotProduct( plane->normal, plane->origin );

	PLVector3 halfDimension;
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
		result->contactPoint = PlScaleVector3F( PlAddVector3( aabb->origin, plane->normal ), result->penetration );
		return true;
	}

	return false;
}

/* https://github.com/erich666/GraphicsGems/blob/master/gems/RayBox.c */
bool PlIsRayIntersectingAabb( const PLCollisionAABB *bounds, const PLCollisionRay *ray, PLVector3 *hitPoint ) {
	PLVector3 max = PlAddVector3( bounds->maxs, bounds->origin );
	PLVector3 min = PlAddVector3( bounds->mins, bounds->origin );

	/* Find candidate planes */

	static const unsigned char right = 0;
	static const unsigned char left = 1;
	static const unsigned char middle = 2;

	PLVector3 candidatePlane;
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
	PLVector3 maxT;
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
