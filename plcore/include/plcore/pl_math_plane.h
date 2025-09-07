/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_math_vector.h>

typedef struct PLPlane {
	QmMathVector3f normal;
	float offset;
} PLPlane;

static inline PLPlane PlMakePlane( const QmMathVector3f *point, const QmMathVector3f *normal ) {
	return ( PLPlane ){ *normal, -PlVector3DotProduct( *point, *normal ) };
}

static inline float PlSignedDistance( const PLPlane *plane, const QmMathVector3f *point ) {
	return PlVector3DotProduct( plane->normal, *point ) + plane->offset;
}

static inline QmMathVector3f PlProjectPointOnPlane( const PLPlane *plane, const QmMathVector3f *point ) {
	return PlScaleVector3(
	        PlSubtractVector3F( *point, PlSignedDistance( plane, point ) ),
	        plane->normal );
}
