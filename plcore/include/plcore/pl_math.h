/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <stdbool.h>

#include "../../src/qmmath/public/qm_math_vector.h"
#include "../../src/qmmath/public/qm_math_colour.h"

#include <plcore/pl.h>

PL_EXTERN_C

// Base Defines

#ifdef M_PI
#	define PL_PI ( float ) M_PI
#else
#	define PL_PI 3.14159265359f
#endif
#define PL_180_DIV_PI ( 180.0f / PL_PI )
#define PL_PI_DIV_180 ( PL_PI / 180.f )

#define PL_TAU     6.28318530717f
#define PL_EPSILON 1.19209290e-7f

enum {
	// Colours
	PL_RED = 0,
	PL_GREEN,
	PL_BLUE,
	PL_ALPHA
};

#define PlFloatToByte( a ) ( uint8_t ) ( roundf( ( a ) * 255.f ) )
#define PlByteToFloat( a ) ( ( a ) / ( float ) 255 )

#define PlClamp( min, val, max ) ( val ) < ( min ) ? ( min ) : ( ( val ) > ( max ) ? ( max ) : ( val ) )

static inline bool PlIsPowerOfTwo( unsigned int num ) {
	return ( bool ) ( ( num != 0 ) && ( ( num & ( ~num + 1 ) ) == num ) );
}

#define PL_DEG2RAD( X ) ( ( X ) * ( PL_PI_DIV_180 ) )
#define PL_RAD2DEG( X ) ( ( X ) * ( PL_180_DIV_PI ) )

/* https://stackoverflow.com/a/9194117 */
static inline int PlRoundUp( int num, int multiple ) {
	return ( num + multiple - 1 ) & -multiple;
}

typedef struct PLRectangleI32 {
	int x, y, w, h;
} PLRectangleI32;
static inline PLRectangleI32 PlCreateRectangleI32( int x, int y, int w, int h ) {
	PLRectangleI32 rectangle;
	rectangle.x = x;
	rectangle.y = y;
	rectangle.w = w;
	rectangle.h = h;

	return rectangle;
}

typedef struct PLRectangleF32 {
	float x, y, w, h;
} PLRectangleF32;
static inline PLRectangleF32 PlCreateRectangleF32( float x, float y, float w, float h ) {
	PLRectangleF32 rectangle;
	rectangle.x = x;
	rectangle.y = y;
	rectangle.w = w;
	rectangle.h = h;

	return rectangle;
}

#include <plcore/pl_math_vector.h>

/////////////////////////////////////////////////////////////////////////////////////
// Colour

#define PL_COLOUR_INDEX( COLOUR, INDEX )  ( ( uint8_t * ) &( COLOUR ) )[ INDEX ]
#define PlColourF32Index( COLOUR, INDEX ) ( ( float * ) &( COLOUR ) )[ INDEX ]

QmMathColour4ub PlColourF32ToU8( const QmMathColour4f *in );
QmMathColour4f PlColourU8ToF32( const QmMathColour4ub *in );

QmMathColour4ub PlAddColour( const QmMathColour4ub *c, const QmMathColour4ub *c2 );
QmMathColour4f PlAddColourF32( const QmMathColour4f *c, const QmMathColour4f *c2 );

static inline void PlSetColour4B( QmMathColour4ub *c, uint8_t r, uint8_t g, uint8_t b, uint8_t a ) {
	c->r = r;
	c->g = g;
	c->b = b;
	c->a = a;
}

static inline void PlSetColour4F( QmMathColour4ub *c, float r, float g, float b, float a ) {
	c->r = PlFloatToByte( r );
	c->g = PlFloatToByte( g );
	c->b = PlFloatToByte( b );
	c->a = PlFloatToByte( a );
}

static inline void PlClearColour( QmMathColour4ub *c ) {
	PlSetColour4B( c, 0, 0, 0, 0 );
}

static inline bool PlColour4fCompare( const QmMathColour4f *a, const QmMathColour4f *b ) {
	return ( ( a->r == b->r ) && ( a->g == b->g ) && ( a->b == b->b ) && ( a->a == b->a ) );
}

static inline bool PlCompareColour( QmMathColour4ub c, QmMathColour4ub c2 ) {
	return ( ( c.r == c2.r ) && ( c.g == c2.g ) && ( c.b == c2.b ) && ( c.a == c2.a ) );
}

static inline void PlMultiplyColour( QmMathColour4ub *c, QmMathColour4ub c2 ) {
	c->r *= c2.r;
	c->g *= c2.g;
	c->b *= c2.b;
	c->a *= c2.a;
}

static inline void PlMultiplyColourf( QmMathColour4ub *c, float a ) {
	uint8_t a2 = PlFloatToByte( a );
	c->r *= a2;
	c->g *= a2;
	c->b *= a2;
	c->a *= a2;
}

static inline void PlDivideColour( QmMathColour4ub *c, QmMathColour4ub c2 ) {
	c->r /= c2.r;
	c->g /= c2.g;
	c->b /= c2.b;
	c->a /= c2.a;
}

static inline void PlDivideColourF( QmMathColour4ub *c, float a ) {
	uint8_t a2 = PlFloatToByte( a );
	c->r /= a2;
	c->g /= a2;
	c->b /= a2;
	c->a /= a2;
}

static inline const char *PlPrintColour( QmMathColour4ub c ) {
	static char s[ 16 ] = { '\0' };
	snprintf( s, 16, "%i %i %i %i", c.r, c.g, c.b, c.a );
	return s;
}

static inline QmMathColour4f PlVector4ToColourF32( const QmMathVector4f *v ) {
	QmMathColour4f colour;
	memcpy( &colour, v, sizeof( QmMathColour4f ) );
	return colour;
}

#define PL_COLOURF32RGB( R, G, B ) \
	( QmMathColour4f ){ .r = ( R ), .g = ( G ), .b = ( B ), .a = 1.0f }

#define PL_COLOURU8_TO_F32( C ) QM_MATH_COLOUR4F( PlByteToFloat( ( C ).r ), PlByteToFloat( ( C ).g ), PlByteToFloat( ( C ).b ), PlByteToFloat( ( C ).a ) )
#define PL_COLOURF32_TO_U8( C ) QM_MATH_COLOUR4UB( PlFloatToByte( ( C ).r ), PlFloatToByte( ( C ).g ), PlFloatToByte( ( C ).b ), PlFloatToByte( ( C ).a ) )

/* pinks */
#define PL_COLOUR_PINK                   QM_MATH_COLOUR4UB_RGB( 255, 192, 203 )
#define PL_COLOUR_LIGHT_PINK             QM_MATH_COLOUR4UB_RGB( 255, 182, 193 )
#define PL_COLOUR_HOT_PINK               QM_MATH_COLOUR4UB_RGB( 255, 105, 180 )
#define PL_COLOUR_DEEP_PINK              QM_MATH_COLOUR4UB_RGB( 255, 20, 147 )
#define PL_COLOUR_PALE_VIOLET_RED        QM_MATH_COLOUR4UB_RGB( 219, 112, 147 )
#define PL_COLOUR_MEDIUM_VIOLET_RED      QM_MATH_COLOUR4UB_RGB( 199, 21, 133 )
/* reds */
#define PL_COLOUR_LIGHT_SALMON           QM_MATH_COLOUR4UB_RGB( 255, 160, 122 )
#define PL_COLOUR_SALMON                 QM_MATH_COLOUR4UB_RGB( 250, 128, 114 )
#define PL_COLOUR_DARK_SALMON            QM_MATH_COLOUR4UB_RGB( 233, 150, 122 )
#define PL_COLOUR_LIGHT_CORAL            QM_MATH_COLOUR4UB_RGB( 240, 128, 128 )
#define PL_COLOUR_INDIAN_RED             QM_MATH_COLOUR4UB_RGB( 205, 92, 92 )
#define PL_COLOUR_CRIMSON                QM_MATH_COLOUR4UB_RGB( 220, 20, 60 )
#define PL_COLOUR_FIRE_BRICK             QM_MATH_COLOUR4UB_RGB( 178, 34, 34 )
#define PL_COLOUR_DARK_RED               QM_MATH_COLOUR4UB_RGB( 139, 0, 0 )
#define PL_COLOUR_RED                    QM_MATH_COLOUR4UB_RGB( 255, 0, 0 )
/* oranges */
#define PL_COLOUR_ORANGE_RED             QM_MATH_COLOUR4UB_RGB( 255, 69, 0 )
#define PL_COLOUR_TOMATO                 QM_MATH_COLOUR4UB_RGB( 255, 99, 71 )
#define PL_COLOUR_CORAL                  QM_MATH_COLOUR4UB_RGB( 255, 127, 80 )
#define PL_COLOUR_DARK_ORANGE            QM_MATH_COLOUR4UB_RGB( 255, 140, 0 )
#define PL_COLOUR_ORANGE                 QM_MATH_COLOUR4UB_RGB( 255, 165, 0 )
/* yellows */
#define PL_COLOUR_YELLOW                 QM_MATH_COLOUR4UB_RGB( 255, 255, 0 )
#define PL_COLOUR_LIGHT_YELLOW           QM_MATH_COLOUR4UB_RGB( 255, 255, 224 )
#define PL_COLOUR_LEMON_CHIFFON          QM_MATH_COLOUR4UB_RGB( 255, 250, 205 )
#define PL_COLOUR_LIGHT_GOLDENROD_YELLOW QM_MATH_COLOUR4UB_RGB( 250, 250, 210 )
#define PL_COLOUR_PAPAYA_WHIP            QM_MATH_COLOUR4UB_RGB( 255, 239, 213 )
#define PL_COLOUR_MOCCASIN               QM_MATH_COLOUR4UB_RGB( 255, 228, 181 )
#define PL_COLOUR_PEACH_PUFF             QM_MATH_COLOUR4UB_RGB( 255, 218, 185 )
#define PL_COLOUR_PALE_GOLDENROD         QM_MATH_COLOUR4UB_RGB( 238, 232, 170 )
#define PL_COLOUR_KHAKI                  QM_MATH_COLOUR4UB_RGB( 240, 230, 140 )
#define PL_COLOUR_DARK_KHAKI             QM_MATH_COLOUR4UB_RGB( 189, 183, 107 )
#define PL_COLOUR_GOLD                   QM_MATH_COLOUR4UB_RGB( 255, 215, 0 )
/* browns */
#define PL_COLOUR_CORNSILK               QM_MATH_COLOUR4UB_RGB( 255, 248, 220 )
#define PL_COLOUR_BLANCHED_ALMOND        QM_MATH_COLOUR4UB_RGB( 255, 235, 205 )
#define PL_COLOUR_BISQUE                 QM_MATH_COLOUR4UB_RGB( 255, 228, 196 )
#define PL_COLOUR_NAVAJO_WHITE           QM_MATH_COLOUR4UB_RGB( 255, 222, 173 )
#define PL_COLOUR_WHEAT                  QM_MATH_COLOUR4UB_RGB( 245, 222, 179 )
#define PL_COLOUR_BURLY_WOOD             QM_MATH_COLOUR4UB_RGB( 222, 184, 135 )
#define PL_COLOUR_TAN                    QM_MATH_COLOUR4UB_RGB( 210, 180, 140 )
#define PL_COLOUR_ROSY_BROWN             QM_MATH_COLOUR4UB_RGB( 188, 143, 143 )
#define PL_COLOUR_SANDY_BROWN            QM_MATH_COLOUR4UB_RGB( 244, 164, 96 )
#define PL_COLOUR_GOLDENROD              QM_MATH_COLOUR4UB_RGB( 218, 165, 32 )
#define PL_COLOUR_DARK_GOLDENROD         QM_MATH_COLOUR4UB_RGB( 184, 134, 11 )
#define PL_COLOUR_PERU                   QM_MATH_COLOUR4UB_RGB( 205, 133, 63 )
#define PL_COLOUR_CHOCOLATE              QM_MATH_COLOUR4UB_RGB( 210, 105, 30 )
#define PL_COLOUR_SADDLE_BROWN           QM_MATH_COLOUR4UB_RGB( 139, 69, 19 )
#define PL_COLOUR_SIENNA                 QM_MATH_COLOUR4UB_RGB( 160, 82, 45 )
#define PL_COLOUR_BROWN                  QM_MATH_COLOUR4UB_RGB( 165, 42, 42 )
#define PL_COLOUR_MAROON                 QM_MATH_COLOUR4UB_RGB( 128, 0, 0 )
/* greens */
#define PL_COLOUR_DARK_OLIVE_GREEN       QM_MATH_COLOUR4UB_RGB( 85, 107, 47 )
#define PL_COLOUR_OLIVE                  QM_MATH_COLOUR4UB_RGB( 128, 128, 0 )
#define PL_COLOUR_OLIVE_DRAB             QM_MATH_COLOUR4UB_RGB( 107, 142, 35 )
#define PL_COLOUR_YELLOW_GREEN           QM_MATH_COLOUR4UB_RGB( 154, 205, 50 )
#define PL_COLOUR_LIME_GREEN             QM_MATH_COLOUR4UB_RGB( 50, 205, 50 )
#define PL_COLOUR_LIME                   QM_MATH_COLOUR4UB_RGB( 0, 255, 0 )
#define PL_COLOUR_LAWN_GREEN             QM_MATH_COLOUR4UB_RGB( 124, 252, 0 )
#define PL_COLOUR_CHARTREUSE             QM_MATH_COLOUR4UB_RGB( 127, 255, 0 )
#define PL_COLOUR_GREEN_YELLOW           QM_MATH_COLOUR4UB_RGB( 173, 255, 47 )
#define PL_COLOUR_SPRING_GREEN           QM_MATH_COLOUR4UB_RGB( 0, 255, 127 )
#define PL_COLOUR_MEDIUM_SPRING_GREEN    QM_MATH_COLOUR4UB_RGB( 0, 250, 154 )
#define PL_COLOUR_LIGHT_GREEN            QM_MATH_COLOUR4UB_RGB( 144, 238, 144 )
#define PL_COLOUR_PALE_GREEN             QM_MATH_COLOUR4UB_RGB( 152, 251, 152 )
#define PL_COLOUR_DARK_SEA_GREEN         QM_MATH_COLOUR4UB_RGB( 143, 188, 143 )
#define PL_COLOUR_MEDIUM_AQUAMARINE      QM_MATH_COLOUR4UB_RGB( 102, 205, 170 )
#define PL_COLOUR_MEDIUM_SEA_GREEN       QM_MATH_COLOUR4UB_RGB( 60, 179, 113 )
#define PL_COLOUR_SEA_GREEN              QM_MATH_COLOUR4UB_RGB( 46, 139, 87 )
#define PL_COLOUR_FOREST_GREEN           QM_MATH_COLOUR4UB_RGB( 34, 139, 34 )
#define PL_COLOUR_GREEN                  QM_MATH_COLOUR4UB_RGB( 0, 128, 0 )
#define PL_COLOUR_DARK_GREEN             QM_MATH_COLOUR4UB_RGB( 0, 100, 0 )
/* cyans */
#define PL_COLOUR_AQUA                   QM_MATH_COLOUR4UB_RGB( 0, 255, 255 )
#define PL_COLOUR_CYAN                   PL_COLOUR_AQUA
#define PL_COLOUR_LIGHT_CYAN             QM_MATH_COLOUR4UB_RGB( 224, 255, 255 )
#define PL_COLOUR_PALE_TURQUOISE         QM_MATH_COLOUR4UB_RGB( 175, 238, 238 )
#define PL_COLOUR_AQUAMARINE             QM_MATH_COLOUR4UB_RGB( 127, 255, 212 )
#define PL_COLOUR_TURQUOISE              QM_MATH_COLOUR4UB_RGB( 64, 224, 208 )
#define PL_COLOUR_MEDIUM_TURQUOISE       QM_MATH_COLOUR4UB_RGB( 72, 209, 204 )
#define PL_COLOUR_DARK_TURQUOISE         QM_MATH_COLOUR4UB_RGB( 0, 206, 209 )
#define PL_COLOUR_LIGHT_SEA_GREEN        QM_MATH_COLOUR4UB_RGB( 32, 178, 170 )
#define PL_COLOUR_CADET_BLUE             QM_MATH_COLOUR4UB_RGB( 95, 158, 160 )
#define PL_COLOUR_DARK_CYAN              QM_MATH_COLOUR4UB_RGB( 0, 139, 139 )
#define PL_COLOUR_TEAL                   QM_MATH_COLOUR4UB_RGB( 0, 128, 128 )
/* blues */
#define PL_COLOUR_LIGHT_STEEL_BLUE       QM_MATH_COLOUR4UB_RGB( 176, 196, 222 )
#define PL_COLOUR_POWDER_BLUE            QM_MATH_COLOUR4UB_RGB( 176, 224, 230 )
#define PL_COLOUR_LIGHT_BLUE             QM_MATH_COLOUR4UB_RGB( 173, 216, 230 )
#define PL_COLOUR_SKY_BLUE               QM_MATH_COLOUR4UB_RGB( 135, 206, 235 )
#define PL_COLOUR_LIGHT_SKY_BLUE         QM_MATH_COLOUR4UB_RGB( 135, 206, 250 )
#define PL_COLOUR_DEEP_SKY_BLUE          QM_MATH_COLOUR4UB_RGB( 0, 191, 255 )
#define PL_COLOUR_DODGER_BLUE            QM_MATH_COLOUR4UB_RGB( 30, 144, 255 )
#define PL_COLOUR_CORNFLOWER_BLUE        QM_MATH_COLOUR4UB_RGB( 100, 149, 237 )
#define PL_COLOUR_STEEL_BLUE             QM_MATH_COLOUR4UB_RGB( 70, 130, 180 )
#define PL_COLOUR_ROYAL_BLUE             QM_MATH_COLOUR4UB_RGB( 65, 105, 225 )
#define PL_COLOUR_BLUE                   QM_MATH_COLOUR4UB_RGB( 0, 0, 255 )
#define PL_COLOUR_MEDIUM_BLUE            QM_MATH_COLOUR4UB_RGB( 0, 0, 205 )
#define PL_COLOUR_DARK_BLUE              QM_MATH_COLOUR4UB_RGB( 0, 0, 139 )
#define PL_COLOUR_NAVY                   QM_MATH_COLOUR4UB_RGB( 0, 0, 128 )
#define PL_COLOUR_MIDNIGHT_BLUE          QM_MATH_COLOUR4UB_RGB( 25, 25, 112 )
/* purples */
#define PL_COLOUR_LAVENDER               QM_MATH_COLOUR4UB_RGB( 230, 230, 250 )
#define PL_COLOUR_THISTLE                QM_MATH_COLOUR4UB_RGB( 216, 191, 216 )
#define PL_COLOUR_PLUM                   QM_MATH_COLOUR4UB_RGB( 221, 160, 221 )
#define PL_COLOUR_VIOLET                 QM_MATH_COLOUR4UB_RGB( 238, 130, 238 )
#define PL_COLOUR_ORCHID                 QM_MATH_COLOUR4UB_RGB( 218, 112, 214 )
#define PL_COLOUR_FUCHSIA                QM_MATH_COLOUR4UB_RGB( 255, 0, 255 )
#define PL_COLOUR_MAGENTA                PL_COLOUR_FUCHSIA
#define PL_COLOUR_MEDIUM_ORCHID          QM_MATH_COLOUR4UB_RGB( 186, 85, 211 )
#define PL_COLOUR_MEDIUM_PURPLE          QM_MATH_COLOUR4UB_RGB( 147, 112, 219 )
#define PL_COLOUR_BLUE_VIOLET            QM_MATH_COLOUR4UB_RGB( 138, 42, 226 )
#define PL_COLOUR_DARK_VIOLET            QM_MATH_COLOUR4UB_RGB( 148, 0, 211 )
#define PL_COLOUR_DARK_ORCHID            QM_MATH_COLOUR4UB_RGB( 153, 50, 204 )
#define PL_COLOUR_DARK_MAGNENTA          QM_MATH_COLOUR4UB_RGB( 139, 0, 139 )
#define PL_COLOUR_PURPLE                 QM_MATH_COLOUR4UB_RGB( 128, 0, 128 )
#define PL_COLOUR_INDIGO                 QM_MATH_COLOUR4UB_RGB( 75, 0, 130 )
#define PL_COLOUR_DARK_SLATE_BLUE        QM_MATH_COLOUR4UB_RGB( 72, 61, 139 )
#define PL_COLOUR_SLATE_BLUE             QM_MATH_COLOUR4UB_RGB( 106, 90, 205 )
#define PL_COLOUR_MEDIUM_SLATE_BLUE      QM_MATH_COLOUR4UB_RGB( 123, 104, 238 )
/* whites */
#define PL_COLOUR_WHITE                  QM_MATH_COLOUR4UB_RGB( 255, 255, 255 )
#define PL_COLOUR_SNOW                   QM_MATH_COLOUR4UB_RGB( 255, 250, 250 )
#define PL_COLOUR_HONEYDEW               QM_MATH_COLOUR4UB_RGB( 240, 255, 240 )
#define PL_COLOUR_MINT_CREAM             QM_MATH_COLOUR4UB_RGB( 245, 255, 250 )
#define PL_COLOUR_AZURE                  QM_MATH_COLOUR4UB_RGB( 240, 255, 255 )
#define PL_COLOUR_ALICE_BLUE             QM_MATH_COLOUR4UB_RGB( 240, 248, 255 )
#define PL_COLOUR_GHOST_WHITE            QM_MATH_COLOUR4UB_RGB( 248, 248, 255 )
#define PL_COLOUR_WHITE_SMOKE            QM_MATH_COLOUR4UB_RGB( 245, 245, 245 )
#define PL_COLOUR_SEASHELL               QM_MATH_COLOUR4UB_RGB( 255, 245, 238 )
#define PL_COLOUR_BEIGE                  QM_MATH_COLOUR4UB_RGB( 245, 245, 220 )
#define PL_COLOUR_OLD_LACE               QM_MATH_COLOUR4UB_RGB( 253, 245, 230 )
#define PL_COLOUR_FLORAL_WHITE           QM_MATH_COLOUR4UB_RGB( 255, 250, 240 )
#define PL_COLOUR_IVORY                  QM_MATH_COLOUR4UB_RGB( 255, 255, 240 )
#define PL_COLOUR_ANTIQUE_WHITE          QM_MATH_COLOUR4UB_RGB( 250, 235, 215 )
#define PL_COLOUR_LINEN                  QM_MATH_COLOUR4UB_RGB( 250, 240, 230 )
#define PL_COLOUR_LAVENDER_BLUSH         QM_MATH_COLOUR4UB_RGB( 255, 240, 245 )
#define PL_COLOUR_MISTY_ROSE             QM_MATH_COLOUR4UB_RGB( 255, 228, 225 )
/* blacks */
#define PL_COLOUR_GAINSBORO              QM_MATH_COLOUR4UB_RGB( 220, 220, 220 )
#define PL_COLOUR_LIGHT_GRAY             QM_MATH_COLOUR4UB_RGB( 211, 211, 211 )
#define PL_COLOUR_SILVER                 QM_MATH_COLOUR4UB_RGB( 192, 192, 192 )
#define PL_COLOUR_DARK_GRAY              QM_MATH_COLOUR4UB_RGB( 169, 169, 169 )
#define PL_COLOUR_GRAY                   QM_MATH_COLOUR4UB_RGB( 128, 128, 128 )
#define PL_COLOUR_DIM_GRAY               QM_MATH_COLOUR4UB_RGB( 105, 105, 105 )
#define PL_COLOUR_LIGHT_SLATE_GRAY       QM_MATH_COLOUR4UB_RGB( 119, 135, 153 )
#define PL_COLOUR_SLATE_GRAY             QM_MATH_COLOUR4UB_RGB( 112, 128, 144 )
#define PL_COLOUR_DARK_SLATE_GRAY        QM_MATH_COLOUR4UB_RGB( 47, 79, 79 )
#define PL_COLOUR_BLACK                  QM_MATH_COLOUR4UB_RGB( 0, 0, 0 )

#define PL_COLOURF32_WHITE PL_COLOURF32RGB( 1.0f, 1.0f, 1.0f )
#define PL_COLOURF32_BLACK PL_COLOURF32RGB( 0.0f, 0.0f, 0.0f )

/////////////////////////////////////////////////////////////////////////////////////
// Primitives

// Cube

// Sphere

typedef struct PLSphere {
	QmMathVector3f position;
	float radius;

	QmMathColour4ub colour;
} PLSphere;

// Quad

typedef struct PLQuad {
	float x, y, w, h;
} PLQuad;

#define PL_QUAD( X, Y, W, H ) \
	( PLQuad ) { .x = ( X ), .y = ( Y ), .w = ( W ), .h = ( H ) }

/////////////////////////////////////////////////////////////////////////////////////
// Randomisation

int *PlSeedRandom( int seed );
int *PlSeedPerlin( const int *hashTable );

double PlGeneratePerlinNoise( int *seed, double x, double y, double z );

// Linear congruential generator

#define PL_RANDOM_LCG_VB6_MUL 0x43FD43FD
#define PL_RANDOM_LCG_VB6_INC 0xC39EC3
#define PL_RANDOM_LCG_VB6_MOD 0xFFFFFF

static inline int PlGenerateRandomIntegerLCG( int *seed, int mul, int inc, int mod ) {
	return ( *seed = ( ( *seed * mul + inc ) & mod ) );
}

#define PlGenerateRandomIntegerLCG_VB6( SEED ) PlGenerateRandomIntegerLCG( SEED, PL_RANDOM_LCG_VB6_MUL, PL_RANDOM_LCG_VB6_INC, PL_RANDOM_LCG_VB6_MOD )

/////////////////////////////////////////////////////////////////////////////////////
// Interpolation
// http://paulbourke.net/miscellaneous/interpolation/

static inline float PlLinearInterpolate( float y1, float y2, float mu ) {
	return ( y1 * ( 1 - mu ) + y2 * mu );
}

static inline QmMathVector3f PlLinearInterpolateV3f( QmMathVector3f a, QmMathVector3f b, float mu )
{
	return ( QmMathVector3f ) {
	        .x = PlLinearInterpolate( a.x, b.x, mu ),
	        .y = PlLinearInterpolate( a.y, b.y, mu ),
	        .z = PlLinearInterpolate( a.z, b.z, mu ),
	};
}

static inline float PlCosineInterpolate( float y1, float y2, float mu )
{
	float mu2 = ( 1 - cosf( mu * ( float ) PL_PI ) ) / 2;
	return ( y1 * ( 1 - mu2 ) + y2 * mu2 );
}

static inline QmMathVector3f PlCosineInterpolateV3f( QmMathVector3f a, QmMathVector3f b, float mu )
{
	return ( QmMathVector3f ) {
	        .x = PlCosineInterpolate( a.x, b.x, mu ),
	        .y = PlCosineInterpolate( a.y, b.y, mu ),
	        .z = PlCosineInterpolate( a.z, b.z, mu ),
	};
}

// http://probesys.blogspot.co.uk/2011/10/useful-math-functions.html

static inline float PlOutPow( float x, float p ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	int sign = ( int ) p % 2 == 0 ? -1 : 1;
	return ( sign * ( powf( x - 1.0f, p ) + sign ) );
}

static inline float PlLinear( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}
	return x;
}

static inline float PlInPow( float x, float p ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}
	return powf( x, p );
}

static inline float PlInSin( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}
	return -cosf( x * ( ( float ) PL_PI / 2.0f ) ) + 1.0f;
}

static inline float PlOutSin( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}
	return sinf( x * ( ( float ) PL_PI / 2.0f ) );
}

static inline float PlInExp( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	return powf( 2.0f, 10.0f * ( x - 1.0f ) );
}

static inline float PlOutExp( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}
	return -powf( 2.0f, -1.0f * x ) + 1.0f;
}

static inline float PlInOutExp( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}
	return x < 0.5f ? 0.5f * powf( 2.0f, 10.0f * ( 2.0f * x - 1.0f ) ) : 0.5f * ( -powf( 2.0f, 10.0f * ( -2.0f * x + 1.0f ) ) + 1.0f );
}

static inline float PlInCirc( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}
	return -( sqrtf( 1.0f - x * x ) - 1.0f );
}

static inline float PlOutBack( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}
	return ( x - 1.0f ) * ( x - 1.0f ) * ( ( 1.70158f + 1.0f ) * ( x - 1.0f ) + 1.70158f ) + 1.0f;
}

// The variable, k, controls the stretching of the function.
static inline float PlImpulse( float x, float k ) {
	float h = k * x;
	return h * expf( 1.0f - h );
}

static inline float PlRebound( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	if ( x < ( 1.0f / 2.75f ) ) {
		return 1.0f - 7.5625f * x * x;
	} else if ( x < ( 2.0f / 2.75f ) ) {
		return 1.0f - ( 7.5625f * ( x - 1.5f / 2.75f ) *
		                        ( x - 1.5f / 2.75f ) +
		                0.75f );
	} else if ( x < ( 2.5f / 2.75f ) ) {
		return 1.0f - ( 7.5625f * ( x - 2.25f / 2.75f ) *
		                        ( x - 2.25f / 2.75f ) +
		                0.9375f );
	} else {
		return 1.0f - ( 7.5625f * ( x - 2.625f / 2.75f ) * ( x - 2.625f / 2.75f ) +
		                0.984375f );
	}
}

static inline float PlExpPulse( float x, float k, float n ) {
	return expf( -k * powf( x, n ) );
}

static inline float PlInOutBack( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	return x < 0.5f ? 0.5f * ( 4.0f * x * x * ( ( 2.5949f + 1.0f ) * 2.0f * x - 2.5949f ) ) : 0.5f * ( ( 2.0f * x - 2.0f ) * ( 2.0f * x - 2.0f ) * ( ( 2.5949f + 1.0f ) * ( 2.0f * x - 2.0f ) + 2.5949f ) + 2.0f );
}

static inline float PlInBack( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	return x * x * ( ( 1.70158f + 1.0f ) * x - 1.70158f );
}

static inline float PlInOutCirc( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	return x < 1.0f ? -0.5f * ( sqrtf( 1.0f - x * x ) - 1.0f ) : 0.5f * ( sqrtf( 1.0f - ( ( 1.0f * x ) - 2.0f ) * ( ( 2.0f * x ) - 2.0f ) ) + 1.0f );
}

static inline float PlOutCirc( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	return sqrtf( 1.0f - ( x - 1.0f ) * ( x - 1.0f ) );
}

static inline float PlInOutSin( float x ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	return -0.5f * ( cosf( ( float ) PL_PI * x ) - 1.0f );
}

static inline float PlInOutPow( float x, float p ) {
	if ( x < 0 ) {
		return 0;
	} else if ( x > 1.0f ) {
		return 1.0f;
	}

	int sign = ( int ) p % 2 == 0 ? -1 : 1;
	return ( sign / 2.0f * ( powf( x - 2.0f, p ) + sign * 2.0f ) );
}

//////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS

/* http://www.songho.ca/opengl/gl_anglestoaxes.html */
static inline void PlAnglesAxes( QmMathVector3f angles, QmMathVector3f *left, QmMathVector3f *up, QmMathVector3f *forward ) {
	/* pitch */
	float theta = PL_DEG2RAD( angles.x );
	float sp = sinf( theta );
	float cp = cosf( theta );

	/* yaw */
	theta = PL_DEG2RAD( angles.y );
	float sy = sinf( theta );
	float cy = cosf( theta );

	/* roll */
	theta = PL_DEG2RAD( angles.z );
	float sr = sinf( theta );
	float cr = cosf( theta );

	if ( left != NULL ) {
		left->x = sy * sp * sr + cy * cr;
		left->y = -cp * sr;
		left->z = cy * sp * sr - sy * cr;
	}

	if ( up != NULL ) {
		up->x = sy * sp * cr - cy * sr;
		up->y = cp * cr;
		up->z = cy * sp * cr + sy * sr;
	}

	if ( forward != NULL ) {
		forward->x = sy * cp;
		forward->y = -sp;
		forward->z = cy * cp;
	}
}

PL_EXTERN_C_END

#include <plcore/pl_math_matrix.h>
#include <plcore/pl_math_quaternion.h>
