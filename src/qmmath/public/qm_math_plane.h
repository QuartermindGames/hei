// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#include "qm_math.h"
#include "qm_math_vector.h"

#if defined( __cplusplus )
extern "C"
{
#endif

	typedef struct QmMathPlane
	{
		QmMathVector3f normal;
		float          distance;
	} QmMathPlane;

	//TODO: the API is a *little* different in design here because this came from Ape's common library,
	//		so at some stage it needs to be refactored.

	QmMathPlane *qm_math_plane_setup( QmMathPlane *self, const QmMathVector3f *p0, const QmMathVector3f *p1, const QmMathVector3f *p2 );
	float        qm_math_plane_distance( const QmMathPlane *self, const QmMathVector3f *pos );
	void         qm_math_plane_basis_vectors( const QmMathPlane *self, QmMathVector3f *tangentDst, QmMathVector3f *bitangentDst );

	typedef enum QmMathPlaneProjection
	{
		COM_MATH_PLANE_PROJECTION_YZ,
		COM_MATH_PLANE_PROJECTION_XZ,
		COM_MATH_PLANE_PROJECTION_XY,
	} QmMathPlaneProjection;

	QmMathPlaneProjection qm_math_plane_compute_projection( const QmMathPlane *self );

	QmMathVector3f qm_math_plane_project_point( const QmMathPlane *self, const QmMathVector3f *point );

#if defined( __cplusplus )
};
#endif
