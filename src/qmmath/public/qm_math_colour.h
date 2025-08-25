// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#include "qm_math.h"
#include "qm_math_vector.h"

#if defined( __cplusplus )
extern "C"
{
#endif

	/////////////////////////////////////////////////////////////////////////////////////
	// Colour4f
	/////////////////////////////////////////////////////////////////////////////////////

	typedef struct QmMathColour4f
	{
		union
		{
			struct
			{
				float r;
				float g;
				float b;
				float a;
			};

			float v[ 4 ];
		};
	} QmMathColour4f;

#define QM_MATH_COLOUR4F( R, G, B, A ) \
	( QmMathColour4f ) { .r = ( R ), .g = ( G ), .b = ( B ), .a = ( A ) }
#define QM_MATH_COLOUR4F_RGB( R, G, B ) QM_MATH_COLOUR4F( R, G, B, 1.0f )
#define QM_MATH_COLOUR4F_R( R )         QM_MATH_COLOUR4F( R, 1.0f, 1.0f, 1.0f )
#define QM_MATH_COLOUR4F_G( G )         QM_MATH_COLOUR4F( 1.0f, G, 1.0f, 1.0f )
#define QM_MATH_COLOUR4F_B( B )         QM_MATH_COLOUR4F( 1.0f, 1.0f, B, 1.0f )
#define QM_MATH_COLOUR4F_A( A )         QM_MATH_COLOUR4F( 1.0f, 1.0f, 1.0f, A )

	static inline QmMathColour4f qm_math_colour4f( const float r, const float g, const float b, const float a )
	{
		return QM_MATH_COLOUR4F( r, g, b, a );
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// Colour4ub
	/////////////////////////////////////////////////////////////////////////////////////

	typedef struct QmMathColour4ub
	{
		union
		{
			struct
			{
				unsigned char r;
				unsigned char g;
				unsigned char b;
				unsigned char a;
			};

			unsigned char v[ 4 ];
		};
	} QmMathColour4ub;

#define QM_MATH_COLOUR4UB( R, G, B, A ) \
	( QmMathColour4ub ) { .r = ( R ), .g = ( G ), .b = ( B ), .a = ( A ) }
#define QM_MATH_COLOUR4UB_RGB( R, G, B ) QM_MATH_COLOUR4UB( R, G, B, 255 )
#define QM_MATH_COLOUR4UB_R( R )         QM_MATH_COLOUR4UB( R, 255, 255, 255 )
#define QM_MATH_COLOUR4UB_G( G )         QM_MATH_COLOUR4UB( 255, G, 255, 255 )
#define QM_MATH_COLOUR4UB_B( B )         QM_MATH_COLOUR4UB( 255, 255, B, 255 )
#define QM_MATH_COLOUR4UB_A( A )         QM_MATH_COLOUR4UB( 255, 255, 255, A )

	static inline QmMathColour4ub qm_math_colour4ub( const unsigned char r, const unsigned char g, const unsigned char b, const unsigned char a )
	{
		return QM_MATH_COLOUR4UB( r, g, b, a );
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// Conversion
	/////////////////////////////////////////////////////////////////////////////////////

	static inline QmMathColour4f qm_math_colour4ub_to_colour4f( const QmMathColour4ub src )
	{
		return qm_math_colour4f( QM_MATH_BTOF( src.r ), QM_MATH_BTOF( src.g ), QM_MATH_BTOF( src.b ), QM_MATH_BTOF( src.a ) );
	}

	static inline QmMathColour4ub qm_math_colour4f_to_colour4ub( const QmMathColour4f src )
	{
		return qm_math_colour4ub( QM_MATH_FTOB( src.r ), QM_MATH_FTOB( src.g ), QM_MATH_FTOB( src.b ), QM_MATH_FTOB( src.a ) );
	}

	/////////////////////////////////////////////////////////////////////////////////////
	// Clamp
	/////////////////////////////////////////////////////////////////////////////////////

	static inline QmMathColour4f qm_math_colour4f_clamp( const QmMathColour4f src, const float min, const float max )
	{
		return qm_math_colour4f( QM_MATH_CLAMP( min, src.r, max ),
		                         QM_MATH_CLAMP( min, src.g, max ),
		                         QM_MATH_CLAMP( min, src.b, max ),
		                         QM_MATH_CLAMP( min, src.a, max ) );
	}

	static inline QmMathColour4ub qm_math_colour4ub_clamp( const QmMathColour4ub src, const float min, const float max )
	{
		return qm_math_colour4ub( QM_MATH_CLAMP( min, src.r, max ),
		                          QM_MATH_CLAMP( min, src.g, max ),
		                          QM_MATH_CLAMP( min, src.b, max ),
		                          QM_MATH_CLAMP( min, src.a, max ) );
	}

#if defined( __cplusplus )
};
#endif
