// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#ifdef M_PI
#	define QM_MATH_PI ( float ) M_PI
#else
#	define QM_MATH_PI 3.14159265359f
#endif
#define QM_MATH_180_DIV_PI ( 180.0f / QM_MATH_PI )
#define QM_MATH_PI_DIV_180 ( QM_MATH_PI / 180.f )

#define QM_MATH_TAU     6.28318530717f
#define QM_MATH_EPSILON 1.19209290e-7f

#define QM_MATH_CLAMP( MIN, VAL, MAX ) ( VAL ) < ( MIN ) ? ( MIN ) : ( ( VAL ) > ( MAX ) ? ( MAX ) : ( VAL ) )

#define QM_MATH_DEG2RAD( X ) ( ( X ) * ( QM_MATH_PI_DIV_180 ) )
#define QM_MATH_RAD2DEG( X ) ( ( X ) * ( QM_MATH_180_DIV_PI ) )

#define QM_MATH_FTOB( NUM ) ( unsigned char ) ( ( NUM ) * 255.f )
#define QM_MATH_BTOF( NUM ) ( ( NUM ) / ( float ) 255 )

#define QM_MATH_IS_POW2( NUM ) ( ( NUM ) != 0 && ( ( NUM ) & ~( NUM ) + 1 ) == ( NUM ) )

#define QM_MATH_ROUND_UP( NUM, MUL ) ( ( NUM ) + ( MUL ) - 1 & -( MUL ) )
