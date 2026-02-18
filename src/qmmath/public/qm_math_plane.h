// Copyright © 2017-2026 Quartermind Games, Mark E. Sowden <markelswo@gmail.com>

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
		QM_MATH_PLANE_PROJECTION_YZ,
		QM_MATH_PLANE_PROJECTION_XZ,
		QM_MATH_PLANE_PROJECTION_XY,
	} QmMathPlaneProjection;

	QmMathPlaneProjection qm_math_plane_compute_projection( const QmMathPlane *self );

	QmMathVector3f qm_math_plane_project_point( const QmMathPlane *self, const QmMathVector3f *point );

	static constexpr QmMathVector3f QM_MATH_PROJECT_YZ[ 2 ] = { QM_MATH_VECTOR3F( 0.0f, 1.0f, 0.0f ), QM_MATH_VECTOR3F( 0.0f, 0.0f, 1.0f ) };
	static constexpr QmMathVector3f QM_MATH_PROJECT_XZ[ 2 ] = { QM_MATH_VECTOR3F( 1.0f, 0.0f, 0.0f ), QM_MATH_VECTOR3F( 0.0f, 0.0f, 1.0f ) };
	static constexpr QmMathVector3f QM_MATH_PROJECT_XY[ 2 ] = { QM_MATH_VECTOR3F( 1.0f, 0.0f, 0.0f ), QM_MATH_VECTOR3F( 0.0f, 1.0f, 0.0f ) };

	static constexpr QmMathVector3f QM_MATH_PROJECTION_AXIS[ 3 ][ 2 ] = {
	        {QM_MATH_PROJECT_YZ[ 0 ], QM_MATH_PROJECT_YZ[ 1 ]},
	        {QM_MATH_PROJECT_XZ[ 0 ], QM_MATH_PROJECT_XZ[ 1 ]},
	        {QM_MATH_PROJECT_XY[ 0 ], QM_MATH_PROJECT_XY[ 1 ]},
	};

	static constexpr QmMathVector3f QM_MATH_PROJECTION_NORMAL[ 3 ] = {
	        [QM_MATH_PLANE_PROJECTION_YZ] = QM_MATH_VECTOR3F( 1.0f, 0.0f, 0.0f ),
	        [QM_MATH_PLANE_PROJECTION_XZ] = QM_MATH_VECTOR3F( 0.0f, 1.0f, 0.0f ),
	        [QM_MATH_PLANE_PROJECTION_XY] = QM_MATH_VECTOR3F( 0.0f, 0.0f, 1.0f ),
	};

#if defined( __cplusplus )
};
#endif
