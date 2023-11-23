/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_math_vector.h>

typedef struct PLPlane {
	PLVector3 normal;
	float offset;
} PLPlane;

static inline PLPlane PlMakePlane( const PLVector3 *point, const PLVector3 *normal ) {
	return ( PLPlane ){ *normal, -PlVector3DotProduct( *point, *normal ) };
}

static inline float PlSignedDistance( const PLPlane *plane, const PLVector3 *point ) {
	return PlVector3DotProduct( plane->normal, *point ) + plane->offset;
}

static inline PLVector3 PlProjectPointOnPlane( const PLPlane *plane, const PLVector3 *point ) {
	return PlScaleVector3(
	        PlSubtractVector3F( *point, PlSignedDistance( plane, point ) ),
	        plane->normal );
}
