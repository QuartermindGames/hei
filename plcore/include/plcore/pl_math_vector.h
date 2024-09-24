/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

typedef struct PLVector2 {
	float x, y;
} PLVector2;

typedef struct PLVector3 {
	float x, y, z;
} PLVector3;

typedef struct PLVector4 {
	float x, y, z, w;
} PLVector4;

#define PL_VECTOR2( X, Y ) \
	( PLVector2 ) { X, Y }
#define PL_VECTOR3( X, Y, Z ) \
	( PLVector3 ) { X, Y, Z }
#define PL_VECTOR4( X, Y, Z, W ) \
	( PLVector4 ) { X, Y, Z, W }

/* these will give you the number of elements in each type
 * of vector. 												*/
#define PL_MVECNUM( x ) ( sizeof( x ) / sizeof( float ) )
#ifndef __cplusplus
PL_STATIC_ASSERT( ( PL_MVECNUM( PLVector2 ) == 2 ), "unexpected vector element num" );
PL_STATIC_ASSERT( PL_MVECNUM( PLVector3 ) == 3, "unexpected vector element num" );
PL_STATIC_ASSERT( PL_MVECNUM( PLVector4 ) == 4, "unexpected vector element num" );
#endif
#define PL_MVEC2NUM PL_GETMVECNUM( PLVector2 )
#define PL_MVEC3NUM PL_GETMVECNUM( PLVector3 )
#define PL_MVEC4NUM PL_GETMVECNUM( PLVector4 )

#define PL_VECTOR_I( VECTOR, INDEX )  ( ( float * ) &( VECTOR ) )[ INDEX ]
/* todo: add bound checking to the below implementation??? Or just remove!? */
#define PL_VECTOR3_I( VECTOR, INDEX ) PL_VECTOR_I( VECTOR, INDEX )

/****************************************
 ****************************************/

extern PL_DLL const PLVector2 pl_vecOrigin2;
extern PL_DLL const PLVector3 pl_vecOrigin3;
extern PL_DLL const PLVector4 pl_vecOrigin4;

typedef struct PLMatrix4 PLMatrix4;

PLVector3 PlTransformVector3( const PLVector3 *v, const PLMatrix4 *m );
PLVector4 PlTransformVector4( const PLVector4 *v, const PLMatrix4 *m );

PLVector2 PlAddVector2( PLVector2 v, PLVector2 v2 );
PLVector3 PlAddVector3( PLVector3 v, PLVector3 v2 );
PLVector3 PlAddVector3F( PLVector3 v, float f );
PLVector4 PlAddVector4( PLVector4 v, PLVector4 v2 );

PLVector2 PlDivideVector2( PLVector2 v, PLVector2 v2 );
PLVector2 PlDivideVector2F( const PLVector2 *v, float f );
PLVector3 PlDivideVector3( PLVector3 v, PLVector3 v2 );
PLVector3 PlDivideVector3F( PLVector3 v, float v2 );
PLVector4 PlDivideVector4F( PLVector4 v, float v2 );

PLVector2 PlSubtractVector2( const PLVector2 *a, const PLVector2 *b );
PLVector3 PlSubtractVector3( PLVector3 v, PLVector3 v2 );
PLVector3 PlSubtractVector3F( PLVector3 v, float f );

PLVector2 PlScaleVector2( const PLVector2 *v, const PLVector2 *scale );
PLVector2 PlScaleVector2F( const PLVector2 *v, float scale );
PLVector3 PlScaleVector3( PLVector3 v, PLVector3 v2 );
PLVector3 PlScaleVector3F( PLVector3 v, float f );

PLVector3 PlInverseVector3( PLVector3 v );

float PlVector2DotProduct( const PLVector2 *a, const PLVector2 *b );
float PlVector3DotProduct( PLVector3 v, PLVector3 v2 );
float PlGetPlaneDotProduct( const PLVector4 *plane, const PLVector3 *vector );

PLVector3 PlVector3CrossProduct( PLVector3 v, PLVector3 v2 );

PLVector3 PlVector3Min( PLVector3 v, PLVector3 v2 );

PLVector3 PlVector3Max( PLVector3 v, PLVector3 v2 );

float PlGetVector2Length( const PLVector2 *v );
float PlVector3Length( const PLVector3 v );

PLVector2 plNormalizeVector2( const PLVector2 *v );
PLVector3 PlNormalizeVector3( PLVector3 v );
PLVector4 PlNormalizePlane( PLVector4 plane );

bool PlCompareVector2( const PLVector2 *v, const PLVector2 *v2 );
bool PlCompareVector3( const PLVector3 *v, const PLVector3 *v2 );

PLVector2 PlClampVector2( const PLVector2 *v, float min, float max );
PLVector3 PlClampVector3( const PLVector3 *v, float min, float max );
PLVector4 PlClampVector4( const PLVector4 *v, float min, float max );

const char *PlPrintVector2( const PLVector2 *v, PLVariableType format );
const char *PlPrintVector3( const PLVector3 *v, PLVariableType format );
const char *PlPrintVector4( const PLVector4 *v, PLVariableType format );

PLVector2 PlComputeLineNormal( const PLVector2 *x, const PLVector2 *y );

bool PlIsVectorNaN( float *v, uint8_t numElements );

inline static bool PlIsVector2NaN( const PLVector2 *v ) { return PlIsVectorNaN( ( float * ) v, 2 ); }
inline static bool PlIsVector3NaN( const PLVector3 *v ) { return PlIsVectorNaN( ( float * ) v, 3 ); }
inline static bool PlIsVector4NaN( const PLVector4 *v ) { return PlIsVectorNaN( ( float * ) v, 4 ); }

/****************************************
 * C++ Helper Classes
 ****************************************/

#ifdef __cplusplus

namespace hei {
	struct Vector2 : PLVector2 {
		Vector2( float a, float b ) {
			x = a;
			y = b;
		}
		Vector2( const PLVector2 &v ) {
			x = v.x;
			y = v.y;
		}
		Vector2() {
			x = 0.0f;
			y = 0.0f;
		}

		PLVector2 &operator=( float a ) {
			x = a;
			y = a;
			return *this;
		}

		void operator*=( PLVector2 a ) {
			x *= a.x;
			y *= a.y;
		}
		void operator*=( float a ) {
			x *= a;
			y *= a;
		}

		void operator/=( const PLVector2 &a ) {
			x /= a.x;
			y /= a.y;
		}
		void operator/=( float a ) {
			x /= a;
			y /= a;
		}

		void operator+=( const PLVector2 &a ) {
			x += a.x;
			y += a.y;
		}
		void operator+=( float a ) {
			x += a;
			y += a;
		}

		void operator-=( const PLVector2 &a ) {
			x -= a.x;
			y -= a.y;
		}
		void operator-=( float a ) {
			x -= a;
			y -= a;
		}

		bool operator==( const PLVector2 &a ) const { return ( ( x == a.x ) && ( y == a.y ) ); }
		bool operator==( float a ) const { return ( ( x == a ) && ( y == a ) ); }

		bool operator!=( const PLVector2 &a ) const { return ( ( x != a.x ) || ( y != a.y ) ); }
		bool operator!=( float a ) const { return ( ( x != a ) && ( y != a ) ); }

		PLVector2 operator*( const PLVector2 &a ) const { return Vector2( x * a.x, y * a.y ); }
		PLVector2 operator*( float a ) const { return Vector2( x * a, y * a ); }

		PLVector2 operator/( const PLVector2 &a ) const { return Vector2( x / a.x, y / a.y ); }
		PLVector2 operator/( float a ) const { return Vector2( x / a, y / a ); }

		PLVector2 operator+( const PLVector2 &a ) const { return Vector2( x + a.x, y + a.y ); }
		PLVector2 operator+( float a ) const { return Vector2( x + a, y + a ); }

		PLVector2 operator-( const PLVector2 &a ) const { return Vector2( x - a.x, y - a.y ); }
		PLVector2 operator-( float a ) const { return Vector2( x - a, y - a ); }

		inline float Length() const { return PlGetVector2Length( this ); }

		inline void Clamp( float min, float max ) {
			PLVector2 v = PlClampVector2( this, min, max );
			x = v.x;
			y = v.y;
		}

		Vector2 Normalize() {
			Vector2 out;
			float length = Length();
			if ( length != 0 ) {
				out.Set( x / length, y / length );
			}
			return out;
		}

		inline void Set( float a, float b ) {
			x = a;
			y = b;
		}

		inline void Clear() {
			x = 0;
			y = 0;
		}
	};

	struct Vector3 : PLVector3 {
		inline Vector3() {
			x = y = z = 0.0f;
		}
		inline Vector3( float _x, float _y, float _z ) {
			x = _x;
			y = _y;
			z = _z;
		}
		inline Vector3( const PLVector3 &v ) : Vector3( v.x, v.y, v.z ) {}
		inline explicit Vector3( const float *v ) : Vector3( v[ 0 ], v[ 1 ], v[ 2 ] ) {}

		inline Vector3 &operator=( const PLVector3 &v ) {
			x = v.x;
			y = v.y;
			return *this;
		}

		inline Vector3 &operator=( float a ) {
			x = a;
			y = a;
			z = a;
			return *this;
		}

		inline void operator*=( const PLVector3 &v ) {
			x *= v.x;
			y *= v.y;
			z *= v.z;
		}

		inline void operator*=( float a ) {
			x *= a;
			y *= a;
			z *= a;
		}

		inline void operator+=( const PLVector3 &a ) {
			x += a.x;
			y += a.y;
			z += a.z;
		}

		inline void operator+=( float a ) {
			x += a;
			y += a;
			z += a;
		}

		inline bool operator==( const Vector3 &a ) const {
			return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) );
		}

		inline bool operator==( float f ) const {
			return ( ( x == f ) && ( y == f ) && ( z == f ) );
		}

		inline bool operator!=( const Vector3 &a ) const {
			return !( a == *this );
		}

		inline bool operator!=( float f ) const {
			return ( ( x != f ) && ( y != f ) && ( z != f ) );
		}

		inline Vector3 operator*( Vector3 a ) const {
			return { x * a.x, y * a.y, z * a.z };
		}

		inline Vector3 operator*( float a ) const {
			return { x * a, y * a, z * a };
		}

		inline Vector3 operator-( Vector3 a ) const {
			return { x - a.x, y - a.y, z - a.z };
		}

		inline Vector3 operator-( float a ) const {
			return { x - a, y - a, z - a };
		}

		Vector3 inline operator-() const {
			return { -x, -y, -z };
		}

		Vector3 inline operator+( Vector3 a ) const {
			return { x + a.x, y + a.y, z + a.z };
		}

		Vector3 inline operator+( float a ) const {
			return { x + a, y + a, z + a };
		}

		Vector3 inline operator/( const Vector3 &a ) const {
			return { x / a.x, y / a.y, z / a.z };
		}

		Vector3 inline operator/( float a ) const {
			return { x / a, y / a, z / a };
		}

		inline float &operator[]( const unsigned int i ) {
			return *( ( &x ) + i );
		}

		inline bool operator>( const Vector3 &v ) const {
			return ( x > v.x ) || ( x == v.x && y > v.y ) || ( x == v.x && y == v.y && z > v.z );
		}

		inline bool operator<( const Vector3 &v ) const {
			return ( x < v.x ) || ( x == v.x && y < v.y ) || ( x == v.x && y == v.y && z < v.z );
		}

		inline bool operator>=( const Vector3 &v ) const {
			return *this > v || *this == v;
		}

		inline bool operator<=( const Vector3 &v ) const {
			return *this < v || *this == v;
		}

		inline float Length() const { return PlVector3Length( *this ); }

		inline void Clamp( float min, float max ) {
			PLVector3 v = PlClampVector3( this, min, max );
			x = v.x;
			y = v.y;
			z = v.z;
		}

		inline float DotProduct( PLVector3 a ) const { return PlVector3DotProduct( *this, a ); }
		inline PLVector3 CrossProduct( PLVector3 a ) const { return PlVector3CrossProduct( *this, a ); }

		inline void Normalize() {
			PLVector3 v = PlNormalizeVector3( *this );
			x = v.x;
			y = v.y;
			z = v.z;
		}

		inline float Difference( Vector3 v ) const {
			return ( ( *this ) - v ).Length();
		}

		void inline Set( float _x, float _y, float _z ) {
			x = _x;
			y = _y;
			z = _z;
		}
	};
}// namespace hei

#endif
