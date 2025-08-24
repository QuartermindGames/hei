// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Quaternion math methods.
// Author:  Mark E. Sowden

#include <stdio.h>
#include <math.h>

#include "qmmath/public/qm_math_quaternion.h"

float qm_math_quaternion_compute_w( QmMathQuaternion *q )
{
	float t = 1.f - q->x * q->x - q->y * q->y - q->z * q->z;
	if ( t < 0 )
	{
		q->w = 0;
	}
	else
	{
		q->w = -sqrtf( t );
	}

	return q->w;
}

QmMathVector3f qm_math_quaternion_to_euler( const QmMathQuaternion src )
{
	QmMathVector3f v;

	// pitch
	float sinp = 2.0f * ( src.w * src.y - src.z * src.x );
	if ( fabsf( sinp ) >= 1.0f )
	{
		v.x = copysignf( QM_MATH_PI / 2.0f, sinp );
	}
	else
	{
		v.x = asinf( sinp );
	}

	// yaw
	float sinyCosp = 2.0f * ( src.w * src.z + src.x * src.y );
	float cosyCosp = 1.0f - 2.0f * ( src.y * src.y + src.z * src.z );
	v.y            = atan2f( sinyCosp, cosyCosp );

	// roll
	float sinrCosp = 2.0f * ( src.w * src.x + src.y * src.z );
	float cosrCosp = 1.0f - 2.0f * ( src.x * src.x + src.y * src.y );
	v.z            = atan2f( sinrCosp, cosrCosp );

	return v;
}

QmMathQuaternion qm_math_quaternion_from_euler( const QmMathVector3f src )
{
	// pitch
	float cp = cosf( src.x * 0.5f );
	float sp = sinf( src.x * 0.5f );

	// yaw
	float cy = cosf( src.y * 0.5f );
	float sy = sinf( src.y * 0.5f );

	// roll
	float cr = cosf( src.z * 0.5f );
	float sr = sinf( src.z * 0.5f );

	QmMathQuaternion q;
	q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy;
	q.z = cr * cp * sy - sr * sp * cy;
	q.w = cr * cp * cy + sr * sp * sy;

	return q;
}
