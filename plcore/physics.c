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

#include <PL/platform_physics.h>

#include <float.h>

#if 0
PLCollisionAABB plGenerateAABB( const PLVertex *vertices, unsigned int numVertices, bool absolute ) {
	PLCollisionAABB bounds;
	if ( absolute ) {
		bounds.maxs = ( PLVector3 ){ vertices[ 0 ].position.x, vertices[ 0 ].position.y, vertices[ 0 ].position.z };
		bounds.mins = ( PLVector3 ){ vertices[ 0 ].position.x, vertices[ 0 ].position.y, vertices[ 0 ].position.z };

		for ( unsigned int i = 0; i < numVertices; ++i ) {
			if ( bounds.maxs.x < vertices[ i ].position.x ) { bounds.maxs.x = vertices[ i ].position.x; }
			if ( bounds.maxs.y < vertices[ i ].position.y ) { bounds.maxs.y = vertices[ i ].position.y; }
			if ( bounds.maxs.z < vertices[ i ].position.z ) { bounds.maxs.z = vertices[ i ].position.z; }
			if ( bounds.mins.x > vertices[ i ].position.x ) { bounds.mins.x = vertices[ i ].position.x; }
			if ( bounds.mins.y > vertices[ i ].position.y ) { bounds.mins.y = vertices[ i ].position.y; }
			if ( bounds.mins.z > vertices[ i ].position.z ) { bounds.mins.z = vertices[ i ].position.z; }
		}
	} else {
		/* this technically still doesn't, but it's better */
		float max = FLT_MIN;
		float min = FLT_MAX;
		for ( unsigned int i = 0; i < numVertices; ++i ) {
			if ( vertices[ i ].position.x > max ) { max = vertices[ i ].position.x; }
			if ( vertices[ i ].position.y > max ) { max = vertices[ i ].position.y; }
			if ( vertices[ i ].position.z > max ) { max = vertices[ i ].position.z; }
			if ( vertices[ i ].position.x < min ) { min = vertices[ i ].position.x; }
			if ( vertices[ i ].position.y < min ) { min = vertices[ i ].position.y; }
			if ( vertices[ i ].position.z < min ) { min = vertices[ i ].position.z; }
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
#endif

PLVector3 plGetAABBAbsOrigin( const PLCollisionAABB *bounds, PLVector3 origin ) {
	PLVector3 absOrigin = PLVector3( ( bounds->mins.x + bounds->maxs.x ) / 2, ( bounds->mins.y + bounds->maxs.y ) / 2, ( bounds->mins.z + bounds->maxs.z ) / 2 );
	return plAddVector3( origin, absOrigin );
}

bool plIsAABBIntersecting( const PLCollisionAABB *aBounds, const PLCollisionAABB *bBounds ) {
	PLVector3 aMax = plAddVector3( aBounds->maxs, aBounds->origin );
	PLVector3 aMin = plAddVector3( aBounds->mins, aBounds->origin );
	PLVector3 bMax = plAddVector3( bBounds->maxs, bBounds->origin );
	PLVector3 bMin = plAddVector3( bBounds->mins, bBounds->origin );

	return !(
	        aMax.x < bMin.x ||
	        aMax.y < bMin.y ||
	        aMax.z < bMin.z ||
	        aMin.x > bMax.x ||
	        aMin.y > bMax.y ||
	        aMin.z > bMax.z );
}

bool plIsPointIntersectingAABB( const PLCollisionAABB *bounds, PLVector3 point ) {
	PLVector3 max = plAddVector3( bounds->maxs, bounds->origin );
	PLVector3 min = plAddVector3( bounds->mins, bounds->origin );

	return !(
	        point.x > max.x ||
	        point.x < min.x ||
	        point.y > max.y ||
	        point.y < min.y ||
	        point.z > max.z ||
	        point.z < min.z );
}

bool plIsSphereIntersecting( const PLCollisionSphere *aSphere, const PLCollisionSphere *bSphere ) {
	PLVector3 difference = plSubtractVector3( aSphere->origin, bSphere->origin );
	float distance = plVector3Length( difference );
	float sum_radius = aSphere->radius + bSphere->radius;
	return distance < sum_radius;
}

PLCollision plIsSphereIntersectingPlane( const PLCollisionSphere *sphere, const PLCollisionPlane *plane ) {
	PLCollision collision;
	memset( &collision, 0, sizeof( PLCollision ) );

	PLVector3 pDist;
	pDist = plSubtractVector3( sphere->origin, plane->origin );

	/* distance from the sphere to the plane */
	float distance = plVector3DotProduct( plane->normal, pDist ) - ( sphere->radius * 2.0f );

	collision.contactNormal = plane->normal;
	collision.contactPoint = plScaleVector3f( plSubtractVector3( sphere->origin, plane->normal ), distance );
	collision.penetration = -distance;

	return collision;
}
