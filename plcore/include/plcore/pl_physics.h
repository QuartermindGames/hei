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

#include <plcore/pl.h>
#include <plcore/pl_math.h>

/**
 * References:
 *	http://www.3dkingdoms.com/weekly/weekly.php?a=21
 */

PL_EXTERN_C

typedef struct PLCollisionRay {
	PLVector3 origin, direction;
} PLCollisionRay;

typedef struct PLCollisionAABB {
	PLVector3 origin, mins, maxs, absOrigin;
} PLCollisionAABB;

typedef struct PLCollisionSphere {
	PLVector3 origin;
	float radius;
} PLCollisionSphere;

typedef struct PLCollisionPlane {
	PLVector3 origin, normal;
} PLCollisionPlane;

typedef struct PLCollision {
	PLVector3 contactNormal;
	PLVector3 contactPoint;
	float penetration;
} PLCollision;

#ifndef __cplusplus
#	define PlSetupCollisionRay( ORIGIN, DIRECTION ) \
		( PLCollisionRay ) { ( ORIGIN ), ( DIRECTION ) }
#	define PL_COLLISION_AABB( ORIGIN, MINS, MAXS ) \
		( PLCollisionAABB ) { ( ORIGIN ), ( MINS ), ( MAXS ) }
#	define PlSetupCollisionSphere( ORIGIN, RADIUS ) \
		( PLCollisionSphere ) { ( ORIGIN ), ( RADIUS ) }
#	define PlSetupCollisionPlane( ORIGIN, NORMAL ) \
		( PLCollisionPlane ) { ( ORIGIN ), ( NORMAL ) }
#endif

float PlComputeSphereFromCoords( const PLVector3 *vertices, unsigned int numVertices );

PLCollisionAABB PlGenerateAabbFromCoords( const PLVector3 *vertices, unsigned int numVertices, bool absolute );
PLVector3 PlGetAabbAbsOrigin( const PLCollisionAABB *bounds, PLVector3 origin );

bool PlIsAabbIntersecting( const PLCollisionAABB *aBounds, const PLCollisionAABB *bBounds );
bool PlIsPointIntersectingAabb( const PLCollisionAABB *bounds, PLVector3 point );
bool PlIsSphereIntersectingAabb( const PLCollisionSphere *sphere, const PLCollisionAABB *bounds );

/**
 * Get which side of a line a point is on.
 */
inline static float PlTestPointLinePosition( const PLVector2 *position, const PLVector2 *lineStart, const PLVector2 *lineEnd ) {
	return ( lineEnd->x - lineStart->x ) * ( position->y - lineStart->y ) - ( lineEnd->y - lineStart->y ) * ( position->x - lineStart->x );
}

inline static bool PlIsPointIntersectingLine( const PLVector2 *position, const PLVector2 *lineStart, const PLVector2 *lineEnd, const PLVector2 *lineNormal, float *intersection ) {
	*intersection = 0.0f;

#if 0
	PLVector2 xCoord = ( lineEnd->x > lineStart->x ) ? PLVector2( lineEnd->x, lineStart->x ) : PLVector2( lineStart->x, lineEnd->x );
	PLVector2 yCoord = ( lineEnd->y > lineStart->y ) ? PLVector2( lineEnd->y, lineStart->y ) : PLVector2( lineStart->y, lineEnd->y );
	if( !( position->x > xCoord.x && position->x < xCoord.y ) && !( position->y > yCoord.x && position->y < yCoord.y ) ) {
		return false;
	}
#endif

	//if( ( position->x > xCoord.y || position->x < xCoord.x ) && ( position->y > yCoord.y || position->y < yCoord.x ) ) {
	//	return false;
	//}

	/* figure out which side of the line we're on */
	float lineSide = PlTestPointLinePosition( position, lineStart, lineEnd );
	if ( lineSide <= -1000.0f ) {
		return false;
	}

	*intersection = lineSide;

	return ( *intersection <= 1000.0f );
}

inline static bool PlTestLineIntersection( float dst1, float dst2, const PLVector3 *lineStart, const PLVector3 *lineEnd, PLVector3 *hit ) {
	if ( ( dst1 * dst2 ) >= 0.0f || dst1 == dst2 ) {
		return false;
	}

	for ( unsigned int i = 0; i < 3; ++i ) {
		PlVector3Index( hit, i ) = PlVector3Index( lineStart, i ) + ( PlVector3Index( lineEnd, i ) - PlVector3Index( lineStart, i ) ) * ( -dst1 / ( dst2 - dst1 ) );
	}

	return true;
}

/**
 * Checks whether or not AABB is intersecting with the given line.
 * Currently only works in 2D space (X & Z).
 */
inline static bool PlIsAabbIntersectingLine( const PLCollisionAABB *bounds, const PLVector2 *lineStart, const PLVector2 *lineEnd, const PLVector2 *lineNormal ) {
#if 0
	PLVector2 origin = PLVector2( bounds->origin.x, bounds->origin.z );

	PLVector2 a = plAddVector2( PLVector2( bounds->mins.x, bounds->mins.z ), origin );
	PLVector2 b = plAddVector2( PLVector2( bounds->maxs.x, bounds->mins.z ), origin );
	//PLVector2 c = plAddVector2( PLVector2( bounds->mins.x, bounds->maxs.z ), origin );
	//PLVector2 d = plAddVector2( PLVector2( bounds->maxs.x, bounds->maxs.z ), origin );

	float aR, bR, cR, dR;
	plIsPointIntersectingLine( &a, lineStart, lineEnd, lineNormal, &aR );
	plIsPointIntersectingLine( &b, lineStart, lineEnd, lineNormal, &bR );
	//plIsPointIntersectingLine( &c, lineStart, lineEnd, lineNormal, &cR );
	//plIsPointIntersectingLine( &d, lineStart, lineEnd, lineNormal, &dR );

#	if 0
	if( x == 0.0f || y == 0.0f ) {
		return PLVector2( x, y );
	}
#	endif
#endif

	return false;
}

PLCollision PlIsAabbIntersectingPlane( const PLCollisionAABB *aabb, const PLCollisionPlane *plane );

bool PlIsSphereIntersecting( const PLCollisionSphere *aSphere, const PLCollisionSphere *bSphere );
PLCollision PlIsSphereIntersectingPlane( const PLCollisionSphere *sphere, const PLCollisionPlane *plane );

/* https://github.com/erich666/GraphicsGems/blob/master/gemsii/intersect/intsph.c */
inline static bool PlIsRayIntersectingSphere( const PLCollisionSphere *sphere, const PLCollisionRay *ray, float *enterDistance, float *leaveDistance ) {
	PLVector3 d = PlSubtractVector3( ray->origin, sphere->origin );
	float u = PlVector3DotProduct( d, d ) - sphere->radius * sphere->radius;
	float bsq = PlVector3DotProduct( d, ray->direction );
	float disc = bsq * bsq - u;

	if ( disc >= 0.0f ) {
		float root = sqrtf( disc );
		*enterDistance = -bsq - root;
		*leaveDistance = -bsq + root;
		return true;
	}

	return false;
}

bool PlIsRayIntersectingAabb( const PLCollisionAABB *bounds, const PLCollisionRay *ray, PLVector3 *hitPoint );

PL_EXTERN_C_END

/************************************************************/
