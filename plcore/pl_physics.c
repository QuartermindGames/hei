/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include <plcore/pl_physics.h>

#include <float.h>

PLCollisionAABB PlGenerateAabbFromCoords( const PLVector3 *vertices, unsigned int numVertices, bool absolute ) {
	PLCollisionAABB bounds;
	if ( absolute ) {
		bounds.maxs = ( PLVector3 ){ vertices[ 0 ].x, vertices[ 0 ].y, vertices[ 0 ].z };
		bounds.mins = ( PLVector3 ){ vertices[ 0 ].x, vertices[ 0 ].y, vertices[ 0 ].z };

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
		bounds.maxs = PLVector3( abs, abs, abs );
		bounds.mins = PLVector3( -abs, -abs, -abs );
	}

	/* abs origin is the middle of the bounding volume (wherever it is) and origin is the transformative point */
	bounds.absOrigin = PLVector3( ( bounds.mins.x + bounds.maxs.x ) / 2, ( bounds.mins.y + bounds.maxs.y ) / 2, ( bounds.mins.z + bounds.maxs.z ) / 2 );
	bounds.origin = pl_vecOrigin3;

	return bounds;
}

PLVector3 PlGetAabbAbsOrigin( const PLCollisionAABB *bounds, PLVector3 origin ) {
	PLVector3 absOrigin = PLVector3( ( bounds->mins.x + bounds->maxs.x ) / 2, ( bounds->mins.y + bounds->maxs.y ) / 2, ( bounds->mins.z + bounds->maxs.z ) / 2 );
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

bool PlIsSphereIntersecting( const PLCollisionSphere *aSphere, const PLCollisionSphere *bSphere ) {
	PLVector3 difference = PlSubtractVector3( aSphere->origin, bSphere->origin );
	float distance = PlVector3Length( difference );
	float sum_radius = aSphere->radius + bSphere->radius;
	return distance < sum_radius;
}

PLCollision PlIsSphereIntersectingPlane( const PLCollisionSphere *sphere, const PLCollisionPlane *plane ) {
	PLCollision collision;
	memset( &collision, 0, sizeof( PLCollision ) );

	PLVector3 pDist;
	pDist = PlSubtractVector3( sphere->origin, plane->origin );

	/* distance from the sphere to the plane */
	float distance = PlVector3DotProduct( plane->normal, pDist ) - ( sphere->radius * 2.0f );

	collision.contactNormal = plane->normal;
	collision.contactPoint = PlScaleVector3F( PlSubtractVector3( sphere->origin, plane->normal ), distance );
	collision.penetration = -distance;

	return collision;
}
