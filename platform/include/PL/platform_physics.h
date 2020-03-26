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

#pragma once

#include <PL/platform.h>
#include <PL/platform_math.h>

PL_EXTERN_C

typedef struct PLCollisionRay { PLVector3 origin, direction; } PLCollisionRay;
typedef struct PLCollisionAABB { PLVector3 origin, mins, maxs; } PLCollisionAABB;
typedef struct PLCollisionSphere { PLVector3 origin; float radius; } PLCollisionSphere;

#define PLAABB PLCollisionAABB /* for legacy crap, until everything is updated */

#ifndef __cplusplus
#	define PLCollisionRay( ORIGIN, DIRECTION ) ( PLCollisionRay ){ ( ORIGIN ), ( DIRECTION ) }
#	define PLCollisionAABB( MINS, MAXS )       ( PLCollisionAABB ){ ( MINS ), ( MAXS ) }
#	define PLCollisionSphere( ORIGIN, RADIUS ) ( PLCollisionSphere ){ ( ORIGIN ), ( RADIUS ) }
#endif

PL_INLINE static bool plIntersectAABB( const PLVector3 *aPosition, const PLCollisionAABB *aBounds, const PLVector3 *bPosition, const PLCollisionAABB *bBounds ) {
	PLVector3 aMax = plAddVector3( aBounds->maxs, *aPosition );
	PLVector3 aMin = plAddVector3( aBounds->mins, *aPosition );
	PLVector3 bMax = plAddVector3( bBounds->maxs, *bPosition );
	PLVector3 bMin = plAddVector3( bBounds->mins, *bPosition );

	return !(
		aMax.x < bMin.x ||
		aMax.y < bMin.y ||
		aMax.z < bMin.z ||
		aMin.x > bMax.x ||
		aMin.y > bMax.y ||
		aMin.z > bMax.z
		);
}

PL_INLINE static bool plIntersectPoint( const PLCollisionAABB *bounds, PLVector3 point ) {
	PLVector3 max = plAddVector3( bounds->maxs, bounds->origin );
	PLVector3 min = plAddVector3( bounds->mins, bounds->origin );

	return !(
		point.x > max.x ||
		point.x < min.x ||
		point.y > max.y ||
		point.y < min.y ||
		point.z > max.z ||
		point.z < min.z
		);
}

/**
 * Checks whether or not AABB is intersecting with the given line.
 */
PL_INLINE static bool plIsAABBIntersectingLine( const PLCollisionAABB *aBounds, const PLVector3 *lineStart, const PLVector3 *lineEnd, PLVector3 *intersection ) {

}

PL_INLINE static bool plIsSphereIntersecting( const PLCollisionSphere *aSphere, const PLCollisionSphere *bSphere ) {
	PLVector3 difference = plSubtractVector3( aSphere->origin, bSphere->origin );
	float distance = plVector3Length( &difference );
	float sum_radius = aSphere->radius + bSphere->radius;
	return distance < sum_radius;
}

/* https://github.com/erich666/GraphicsGems/blob/master/gemsii/intersect/intsph.c */
PL_INLINE static bool plIsRayIntersectingSphere( const PLCollisionSphere *sphere, const PLCollisionRay *ray, float *enterDistance, float *leaveDistance ) {
	PLVector3 d = plSubtractVector3( ray->origin, sphere->origin );
	float u = plVector3DotProduct( d, d ) - sphere->radius * sphere->radius;
	float bsq = plVector3DotProduct( d, ray->direction );
	float disc = bsq * bsq - u;

	if( disc >= 0.0f ) {
		float root = sqrtf( disc );
		*enterDistance = -bsq - root;
		*leaveDistance = -bsq + root;
		return true;
	}

	return false;
}

PL_EXTERN_C_END

/************************************************************/
