/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
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

PL_INLINE static PLQuaternion plMultiplyQuaternion( const PLQuaternion* q, const PLQuaternion* q2 ) {
	return PLQuaternion
	(
		( q->x * q2->w ) + ( q->w * q2->x ) + ( q->y * q2->z ) - ( q->z * q2->y ),
		( q->y * q2->w ) + ( q->w * q2->y ) + ( q->z * q2->x ) - ( q->x * q2->z ),
		( q->z * q2->w ) + ( q->w * q2->z ) + ( q->x * q2->y ) - ( q->y * q2->x ),
		( q->w * q2->w ) - ( q->x * q2->x ) - ( q->y * q2->y ) - ( q->z * q2->z )
	);
}

PL_INLINE static PLQuaternion plMultiplyQuaternion3fv( const PLQuaternion* q, const PLVector3* v ) {
	return PLQuaternion
	(
		( q->w * v->x ) + ( q->y * v->z ) - ( q->z * v->y ),
		( q->w * v->y ) + ( q->z * v->x ) - ( q->x * v->z ),
		( q->w * v->z ) + ( q->x * v->y ) - ( q->y * v->x ),
		-( q->x * v->x ) - ( q->y * v->y ) - ( q->z * v->z )
	);
}

PL_INLINE static PLQuaternion plScaleQuaternion( const PLQuaternion* q, float a ) {
	return PLQuaternion( q->x * a, q->y * a, q->z * a, q->w * a );
}

PL_INLINE static PLQuaternion plAddQuaternion( const PLQuaternion* q, const PLQuaternion* q2 ) {
	return PLQuaternion( q->x + q2->x, q->y + q2->y, q->z + q2->z, q->w + q2->w );
}

PL_INLINE static PLQuaternion plAddQuaternionf( const PLQuaternion* q, float a ) {
	return PLQuaternion( q->x + a, q->y + a, q->z + a, q->z + a );
}

PL_INLINE static PLQuaternion plInverseQuaternion( const PLQuaternion* q ) {
	return PLQuaternion( -q->x, -q->y, -q->z, q->w );
}

PL_INLINE static float plQuaternionLength( const PLQuaternion* q ) {
	return sqrtf( q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w );
}

PL_INLINE static PLQuaternion plNormalizeQuaternion( const PLQuaternion* q ) {
	float l = plQuaternionLength( q );
	if ( l > 0 ) {
		float i = 1 / l;
		return plScaleQuaternion( q, i );
	}

	return *q;
}

PL_INLINE static const char* plPrintQuaternion( const PLQuaternion* q ) {
	static char s[32] = { '\0' };
	snprintf( s, 32, "%i %i %i %i", ( int ) q->x, ( int ) q->y, ( int ) q->z, ( int ) q->w );
	return s;
}

PL_INLINE static void plComputeQuaternionW( PLQuaternion* q ) {
	float t = 1.f - ( q->x * q->x ) - ( q->y * q->y ) - ( q->z * q->z );
	if ( t < 0 ) {
		q->w = 0;
	} else {
		q->w = -sqrtf( t );
	}
}

/* pulled from here: http://tfc.duke.free.fr/coding/md5-specs-en.html */
PL_INLINE static PLQuaternion plRotateQuaternionPoint( const PLQuaternion* q, const PLVector3* point ) {
	PLQuaternion b = plInverseQuaternion( q );
	b = plNormalizeQuaternion( &b );

	PLQuaternion a = plMultiplyQuaternion3fv( q, point );
	return plMultiplyQuaternion( &a, &b );
}
