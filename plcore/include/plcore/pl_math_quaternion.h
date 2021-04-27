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
/* Quaternions */

typedef struct PLQuaternion {
	float x, y, z, w;

#ifdef __cplusplus
	PLQuaternion( float a, float b, float c, float d ) : x( a ), y( b ), z( c ), w( d ) {}
	PLQuaternion( float a, float b, float c ) : x( a ), y( b ), z( c ), w( 0 ) {}
	PLQuaternion() : x( 0 ), y( 0 ), z( 0 ), w( 0 ) {}
#endif
} PLQuaternion;

#ifndef __cplusplus

#   define PLQuaternion( x, y, z, w )   (PLQuaternion){ (float)x, (float)y, (float)z, (float) w }

#endif

PL_INLINE static PLQuaternion PlMultiplyQuaternion( const PLQuaternion* q, const PLQuaternion* q2 ) {
	return PLQuaternion
	(
		( q->x * q2->w ) + ( q->w * q2->x ) + ( q->y * q2->z ) - ( q->z * q2->y ),
		( q->y * q2->w ) + ( q->w * q2->y ) + ( q->z * q2->x ) - ( q->x * q2->z ),
		( q->z * q2->w ) + ( q->w * q2->z ) + ( q->x * q2->y ) - ( q->y * q2->x ),
		( q->w * q2->w ) - ( q->x * q2->x ) - ( q->y * q2->y ) - ( q->z * q2->z )
	);
}

PL_INLINE static PLQuaternion PlMultiplyQuaternion3FV( const PLQuaternion* q, const PLVector3* v ) {
	return PLQuaternion
	(
		( q->w * v->x ) + ( q->y * v->z ) - ( q->z * v->y ),
		( q->w * v->y ) + ( q->z * v->x ) - ( q->x * v->z ),
		( q->w * v->z ) + ( q->x * v->y ) - ( q->y * v->x ),
		-( q->x * v->x ) - ( q->y * v->y ) - ( q->z * v->z )
	);
}

PL_INLINE static PLQuaternion PlScaleQuaternion( const PLQuaternion* q, float a ) {
	return PLQuaternion( q->x * a, q->y * a, q->z * a, q->w * a );
}

PL_INLINE static PLQuaternion PlAddQuaternion( const PLQuaternion* q, const PLQuaternion* q2 ) {
	return PLQuaternion( q->x + q2->x, q->y + q2->y, q->z + q2->z, q->w + q2->w );
}

PL_INLINE static PLQuaternion PlAddQuaternionF( const PLQuaternion* q, float a ) {
	return PLQuaternion( q->x + a, q->y + a, q->z + a, q->z + a );
}

PL_INLINE static PLQuaternion PlInverseQuaternion( const PLQuaternion* q ) {
	return PLQuaternion( -q->x, -q->y, -q->z, q->w );
}

PL_INLINE static float PlQuaternionLength( const PLQuaternion* q ) {
	return sqrtf( q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w );
}

PL_INLINE static PLQuaternion PlNormalizeQuaternion( const PLQuaternion* q ) {
	float l = PlQuaternionLength( q );
	if ( l > 0 ) {
		float i = 1 / l;
		return PlScaleQuaternion( q, i );
	}

	return *q;
}

PL_INLINE static const char*PlPrintQuaternion( const PLQuaternion* q ) {
	static char s[32] = { '\0' };
	snprintf( s, 32, "%i %i %i %i", ( int ) q->x, ( int ) q->y, ( int ) q->z, ( int ) q->w );
	return s;
}

PL_INLINE static void PlComputeQuaternionW( PLQuaternion* q ) {
	float t = 1.f - ( q->x * q->x ) - ( q->y * q->y ) - ( q->z * q->z );
	if ( t < 0 ) {
		q->w = 0;
	} else {
		q->w = -sqrtf( t );
	}
}

/* pulled from here: http://tfc.duke.free.fr/coding/md5-specs-en.html */
PL_INLINE static PLQuaternion PlRotateQuaternionPoint( const PLQuaternion* q, const PLVector3* point ) {
	PLQuaternion b = PlInverseQuaternion( q );
	b = PlNormalizeQuaternion( &b );

	PLQuaternion a = PlMultiplyQuaternion3FV( q, point );
	return PlMultiplyQuaternion( &a, &b );
}
