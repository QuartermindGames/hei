/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

/******************************************************************/
/* Vectors */

// 2D

typedef struct PLVector2 {
	float x, y;

#ifdef __cplusplus
	PLVector2( float a, float b ) : x( a ), y( b ) {}
	PLVector2() : x( 0 ), y( 0 ) {}

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
	bool operator!=( float a ) const { return ( ( x != a ) && ( y != a ) ); }

	PLVector2 operator*( const PLVector2 &a ) const { return PLVector2( x * a.x, y * a.y ); }
	PLVector2 operator*( float a ) const { return PLVector2( x * a, y * a ); }

	PLVector2 operator/( const PLVector2 &a ) const { return PLVector2( x / a.x, y / a.y ); }
	PLVector2 operator/( float a ) const { return PLVector2( x / a, y / a ); }

	PLVector2 operator+( const PLVector2 &a ) const { return PLVector2( x + a.x, y + a.y ); }
	PLVector2 operator+( float a ) const { return PLVector2( x + a, y + a ); }

	PLVector2 operator-( const PLVector2 &a ) const { return PLVector2( x - a.x, y - a.y ); }
	PLVector2 operator-( float a ) const { return PLVector2( x - a, y - a ); }

	float Length() { return std::sqrt( x * x + y * y ); }

	PLVector2 Normalize() {
		PLVector2 out;
		float length = Length();
		if ( length != 0 ) {
			out.Set( x / length, y / length );
		}
		return out;
	}

	void Set( float a, float b ) {
		x = a;
		y = b;
	}
	void Clear() {
		x = 0;
		y = 0;
	}
#endif
} PLVector2;

#ifndef __cplusplus

#define PLVector2( x, y ) \
	( PLVector2 ) { x, y }

#endif

extern PL_DLL const PLVector2 pl_vecOrigin2;

inline static PLVector2 PlClampVector2( const PLVector2 *v, float min, float max ) {
	return PLVector2( PlClamp( min, v->x, max ), PlClamp( min, v->y, max ) );
}

inline static PLVector2 PlAddVector2( PLVector2 v, PLVector2 v2 ) {
	v.x += v2.x;
	v.y += v2.y;
	return v;
}

inline static PLVector2 PlSubtractVector2( const PLVector2 *a, const PLVector2 *b ) {
	return PLVector2( a->x - b->x, a->y - b->y );
}

inline static PLVector2 PlScaleVector2( const PLVector2 *v, const PLVector2 *scale ) {
	return PLVector2( v->x * scale->x, v->y * scale->y );
}

inline static PLVector2 PlScaleVector2F( const PLVector2 *v, float scale ) {
	return PLVector2( v->x * scale, v->y * scale );
}

inline static PLVector2 PlDivideVector2( PLVector2 v, PLVector2 v2 ) {
	v.x /= v2.x;
	v.y /= v2.y;
	return v;
}

inline static PLVector2 PlDivideVector2F( const PLVector2 *v, float f ) {
	return PLVector2( v->x / f, v->y / f );
}

inline static bool PlCompareVector2( const PLVector2 *v, const PLVector2 *v2 ) {
	return ( ( v->x == v2->x ) && ( v->y == v2->y ) );
}

inline static float PlVector2DotProduct( const PLVector2 *a, const PLVector2 *b ) {
	return a->x * b->x + a->y * b->y;
}

inline static float PlGetVector2Length( const PLVector2 *v ) {
	return sqrtf( PlVector2DotProduct( v, v ) );
}

inline static PLVector2 plNormalizeVector2( const PLVector2 *v ) {
	float length = PlGetVector2Length( v );
	return PLVector2( v->x / length, v->y / length );
}

inline static PLVector2 PlComputeLineNormal( const PLVector2 *x, const PLVector2 *y ) {
	PLVector2 v = PLVector2( x->y - y->y, y->x - x->x );
	return plNormalizeVector2( &v );
}

// 3D

typedef struct PLVector3 {
	float x, y, z;

#ifdef __cplusplus
	inline PLVector3() : x( 0 ), y( 0 ), z( 0 ) {}
	inline PLVector3( float _x, float _y, float _z ) : x( _x ), y( _y ), z( _z ) {}
	inline explicit PLVector3( const float *v ) {
		x = v[ 0 ];
		y = v[ 1 ];
		z = v[ 2 ];
	}

	inline PLVector3 &operator=( PLVector2 v ) {
		x = v.x;
		y = v.y;
		return *this;
	}

	inline PLVector3 &operator=( float a ) {
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

	inline void operator+=( PLVector3 a ) {
		x += a.x;
		y += a.y;
		z += a.z;
	}

	inline void operator+=( float a ) {
		x += a;
		y += a;
		z += a;
	}

	inline bool operator==( const PLVector3 &a ) const {
		return ( ( x == a.x ) && ( y == a.y ) && ( z == a.z ) );
	}

	inline bool operator==( float f ) const {
		return ( ( x == f ) && ( y == f ) && ( z == f ) );
	}

	inline bool operator!=( const PLVector3 &a ) const {
		return !( a == *this );
	}

	inline bool operator!=( float f ) const {
		return ( ( x != f ) && ( y != f ) && ( z != f ) );
	}

	inline PLVector3 operator*( PLVector3 a ) const {
		return { x * a.x, y * a.y, z * a.z };
	}

	inline PLVector3 operator*( float a ) const {
		return { x * a, y * a, z * a };
	}

	inline PLVector3 operator-( PLVector3 a ) const {
		return { x - a.x, y - a.y, z - a.z };
	}

	inline PLVector3 operator-( float a ) const {
		return { x - a, y - a, z - a };
	}

	PLVector3 inline operator-() const {
		return { -x, -y, -z };
	}

	PLVector3 inline operator+( PLVector3 a ) const {
		return { x + a.x, y + a.y, z + a.z };
	}

	PLVector3 inline operator+( float a ) const {
		return { x + a, y + a, z + a };
	}

	PLVector3 inline operator/( const PLVector3 &a ) const {
		return { x / a.x, y / a.y, z / a.z };
	}

	PLVector3 inline operator/( float a ) const {
		return { x / a, y / a, z / a };
	}

	inline float &operator[]( const unsigned int i ) {
		return *( ( &x ) + i );
	}

	inline bool operator>( const PLVector3 &v ) const {
		return ( x > v.x ) || ( x == v.x && y > v.y ) || ( x == v.x && y == v.y && z > v.z );
	}

	inline bool operator<( const PLVector3 &v ) const {
		return ( x < v.x ) || ( x == v.x && y < v.y ) || ( x == v.x && y == v.y && z < v.z );
	}

	inline bool operator>=( const PLVector3 &v ) const {
		return *this > v || *this == v;
	}

	inline bool operator<=( const PLVector3 &v ) const {
		return *this < v || *this == v;
	}

	inline float Length() const {
		return std::sqrt( x * x + y * y + z * z );
	}

	inline float DotProduct( PLVector3 a ) const {
		return ( x * a.x + y * a.y + z * a.z );
	}

	inline PLVector3 CrossProduct( PLVector3 a ) const {
		return { y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x };
	}

	inline PLVector3 Normalize() const {
		return ( *this ) / Length();
	}

	inline float Difference( PLVector3 v ) const {
		return ( ( *this ) - v ).Length();
	}

	void inline Set( float _x, float _y, float _z ) {
		x = _x;
		y = _y;
		z = _z;
	}
#endif
} PLVector3;

#ifndef __cplusplus

#define PLVector3( x, y, z ) \
	( PLVector3 ) { ( float ) x, ( float ) y, ( float ) z }

#endif

extern PL_DLL const PLVector3 pl_vecOrigin3;

#define PlVectorIndex( VECTOR, INDEX ) ( ( float * ) &( VECTOR ) )[ INDEX ]
/* todo: add bound checking to the below implementation??? Or just remove!? */
#define PlVector3Index( VECTOR, INDEX ) PlVectorIndex( VECTOR, INDEX )

inline static PLVector3 PlClampVector3( const PLVector3 *v, float min, float max ) {
	return PLVector3(
	        PlClamp( min, v->x, max ),
	        PlClamp( min, v->y, max ),
	        PlClamp( min, v->z, max ) );
}

inline static PLVector3 PlAddVector3( PLVector3 v, PLVector3 v2 ) {
	v.x += v2.x;
	v.y += v2.y;
	v.z += v2.z;
	return v;
}

inline static PLVector3 PlAddVector3F( PLVector3 v, float f ) {
	v.x += f;
	v.y += f;
	v.z += f;
	return v;
}

inline static PLVector3 PlSubtractVector3( PLVector3 v, PLVector3 v2 ) {
	v.x -= v2.x;
	v.y -= v2.y;
	v.z -= v2.z;
	return v;
}

inline static PLVector3 PlSubtractVector3F( PLVector3 v, float f ) {
	v.x -= f;
	v.y -= f;
	v.z -= f;
	return v;
}

inline static PLVector3 PlScaleVector3( PLVector3 v, PLVector3 v2 ) {
	v.x *= v2.x;
	v.y *= v2.y;
	v.z *= v2.z;
	return v;
}

inline static PLVector3 PlScaleVector3F( PLVector3 v, float f ) {
	v.x *= f;
	v.y *= f;
	v.z *= f;
	return v;
}

inline static PLVector3 PlDivideVector3( PLVector3 v, PLVector3 v2 ) {
	v.x /= v2.x;
	v.y /= v2.y;
	v.z /= v2.z;
	return v;
}

inline static PLVector3 PlInverseVector3( PLVector3 v ) {
	v.x = -v.x;
	v.y = -v.y;
	v.z = -v.z;
	return v;
}

inline static bool PlCompareVector3( const PLVector3 *v, const PLVector3 *v2 ) {
	return ( ( v->x == v2->x ) && ( v->y == v2->y ) && ( v->z == v2->z ) );
}

inline static PLVector3 PlVector3CrossProduct( PLVector3 v, PLVector3 v2 ) {
	return PLVector3(
	        v.y * v2.z - v.z * v2.y,
	        v.z * v2.x - v.x * v2.z,
	        v.x * v2.y - v.y * v2.x );
}

inline static PLVector3 PlVector3Max( PLVector3 v, PLVector3 v2 ) {
	return PLVector3(
	        v.x > v2.x ? v.x : v2.x,
	        v.y > v2.y ? v.y : v2.y,
	        v.z > v2.z ? v.z : v2.z );
}

inline static PLVector3 PlVector3Min( PLVector3 v, PLVector3 v2 ) {
	return PLVector3(
	        v.x < v2.x ? v.x : v2.x,
	        v.y < v2.y ? v.y : v2.y,
	        v.z < v2.z ? v.z : v2.z );
}

inline static float PlVector3DotProduct( PLVector3 v, PLVector3 v2 ) {
	return ( v.x * v2.x + v.y * v2.y + v.z * v2.z );
}

inline static float PlVector3Length( const PLVector3 v ) {
	return sqrtf( v.x * v.x + v.y * v.y + v.z * v.z );
}

inline static PLVector3 PlNormalizeVector3( PLVector3 v ) {
	float length = PlVector3Length( v );
	if ( length != 0 ) {
		v.x /= length;
		v.y /= length;
		v.z /= length;
	}
	return v;
}

inline static const char *PlPrintVector3( const PLVector3 *v, PLVariableType format ) {
	static char s[ 64 ];
	if ( format == pl_int_var ) snprintf( s, 32, "%i %i %i", ( int ) v->x, ( int ) v->y, ( int ) v->z );
	else
		snprintf( s, 64, "%f %f %f", v->x, v->y, v->z );
	return s;
}

inline static const char *PlPrintVector2( const PLVector2 *v, PLVariableType format ) {
	static char s[ 64 ];
	if ( format == pl_int_var ) snprintf( s, 32, "%i %i", ( int ) v->x, ( int ) v->y );
	else
		snprintf( s, 64, "%f %f", v->x, v->y );
	return s;
}

/******************************************************************/

typedef struct PLVector4 {
#ifdef __cplusplus
	PLVector4() : x( 0.0f ), y( 0.0f ), z( 0.0f ), w( 0.0f ) {}
	PLVector4( float x, float y, float z, float w ) : x( x ), y( y ), z( z ), w( w ) {}
#endif

	float x, y, z, w;
} PLVector4;

#ifndef __cplusplus

#define PLVector4( x, y, z, w ) \
	( PLVector4 ) { ( float ) x, ( float ) y, ( float ) z, ( float ) w }

#endif

extern PL_DLL const PLVector4 pl_vecOrigin4;

inline static PLVector4 PlAddVector4( PLVector4 v, PLVector4 v2 ) {
	v.x += v2.x;
	v.y += v2.y;
	v.z += v2.z;
	v.w += v2.w;
	return v;
}

/* Vector Length */

/******************************************************************/
/* Vector Divide */

inline static PLVector3 PlDivideVector3F( PLVector3 v, float v2 ) {
	v.x /= v2;
	v.y /= v2;
	v.z /= v2;
	return v;
}

inline static PLVector4 PlDivideVector4F( PLVector4 v, float v2 ) {
	v.x /= v2;
	v.y /= v2;
	v.z /= v2;
	v.w /= v2;
	return v;
}

/**
 * Function that works similarly to D3DXPlaneDotCoord.
 */
inline static float PlGetPlaneDotProduct( const PLVector4 *plane, const PLVector3 *vector ) {
	return plane->x * vector->x + plane->y * vector->y + plane->z * vector->z + plane->w * 1.0f;
}

/**
 * Function that works similarly to D3DXPlaneNormalize.
 */
inline static PLVector4 PlNormalizePlane( PLVector4 plane ) {
	float l = PlVector3Length( PLVector3( plane.x, plane.y, plane.z ) );

	plane.x /= l;
	plane.y /= l;
	plane.z /= l;
	plane.w /= l;

	return plane;
}
