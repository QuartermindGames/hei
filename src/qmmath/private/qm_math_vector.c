// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Vector math methods.
// Author:  Mark E. Sowden

#include <stdio.h>
#include <math.h>

#include "qmmath/public/qm_math_vector.h"

/////////////////////////////////////////////////////////////////////////////////////
// Print
/////////////////////////////////////////////////////////////////////////////////////

const char *qm_math_vector2f_print( const QmMathVector2f src, char *dst, const unsigned int dstSize )
{
	snprintf( dst, dstSize, "%f %f", src.x, src.y );
	return dst;
}

const char *qm_math_vector3f_print( const QmMathVector3f src, char *dst, const unsigned int dstSize )
{
	snprintf( dst, dstSize, "%f %f %f", src.x, src.y, src.z );
	return dst;
}

const char *qm_math_vector4f_print( const QmMathVector4f src, char *dst, const unsigned int dstSize )
{
	snprintf( dst, dstSize, "%f %f %f %f", src.x, src.y, src.z, src.z );
	return dst;
}

/////////////////////////////////////////////////////////////////////////////////////
// Length
/////////////////////////////////////////////////////////////////////////////////////

float qm_math_vector2f_length( const QmMathVector2f src )
{
	return sqrtf( qm_math_vector2f_dot_product( src, src ) );
}

float qm_math_vector3f_length( const QmMathVector3f src )
{
	return sqrtf( qm_math_vector3f_dot_product( src, src ) );
}
