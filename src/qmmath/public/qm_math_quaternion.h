// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#include "qm_math.h"
#include "qm_math_vector.h"

#if defined( __cplusplus )
extern "C"
{
#endif

	typedef QmMathVector4f QmMathQuaternion;

	float qm_math_quaternion_compute_w( QmMathQuaternion *q );

	QmMathQuaternion qm_math_quaternion_from_euler( QmMathVector3f src );

#if defined( __cplusplus )
};
#endif
