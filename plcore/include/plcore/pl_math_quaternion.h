/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

PL_EXTERN_C

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

inline static PLQuaternion PlMultiplyQuaternion( const PLQuaternion* q, const PLQuaternion* q2 ) {
	return PLQuaternion
	(
		( q->x * q2->w ) + ( q->w * q2->x ) + ( q->y * q2->z ) - ( q->z * q2->y ),
		( q->y * q2->w ) + ( q->w * q2->y ) + ( q->z * q2->x ) - ( q->x * q2->z ),
		( q->z * q2->w ) + ( q->w * q2->z ) + ( q->x * q2->y ) - ( q->y * q2->x ),
		( q->w * q2->w ) - ( q->x * q2->x ) - ( q->y * q2->y ) - ( q->z * q2->z )
	);
}

inline static PLQuaternion PlMultiplyQuaternion3FV( const PLQuaternion* q, const PLVector3* v ) {
	return PLQuaternion
	(
		( q->w * v->x ) + ( q->y * v->z ) - ( q->z * v->y ),
		( q->w * v->y ) + ( q->z * v->x ) - ( q->x * v->z ),
		( q->w * v->z ) + ( q->x * v->y ) - ( q->y * v->x ),
		-( q->x * v->x ) - ( q->y * v->y ) - ( q->z * v->z )
	);
}

inline static PLQuaternion PlScaleQuaternion( const PLQuaternion* q, float a ) {
	return PLQuaternion( q->x * a, q->y * a, q->z * a, q->w * a );
}

inline static PLQuaternion PlAddQuaternion( const PLQuaternion* q, const PLQuaternion* q2 ) {
	return PLQuaternion( q->x + q2->x, q->y + q2->y, q->z + q2->z, q->w + q2->w );
}

inline static PLQuaternion PlAddQuaternionF( const PLQuaternion* q, float a ) {
	return PLQuaternion( q->x + a, q->y + a, q->z + a, q->z + a );
}

inline static PLQuaternion PlInverseQuaternion( const PLQuaternion* q ) {
	return PLQuaternion( -q->x, -q->y, -q->z, q->w );
}

inline static float PlQuaternionDotProduct(const PLQuaternion* q, const PLQuaternion* q2) {
	/* todo: revisit... */
	return acosf( q->x * q2->x + q->y * q2->y + q->z * q2->z + q->w * q2->w );
}

inline static float PlQuaternionLength( const PLQuaternion* q ) {
	return sqrtf( q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w );
}

inline static PLQuaternion PlNormalizeQuaternion( const PLQuaternion* q ) {
	float l = PlQuaternionLength( q );
	if ( l > 0 ) {
		float i = 1 / l;
		return PlScaleQuaternion( q, i );
	}

	return *q;
}

inline static const char*PlPrintQuaternion( const PLQuaternion* q ) {
	static char s[32] = { '\0' };
	snprintf( s, 32, "%i %i %i %i", ( int ) q->x, ( int ) q->y, ( int ) q->z, ( int ) q->w );
	return s;
}

inline static void PlComputeQuaternionW( PLQuaternion* q ) {
	float t = 1.f - ( q->x * q->x ) - ( q->y * q->y ) - ( q->z * q->z );
	if ( t < 0 ) {
		q->w = 0;
	} else {
		q->w = -sqrtf( t );
	}
}

/* pulled from here: http://tfc.duke.free.fr/coding/md5-specs-en.html */
inline static PLQuaternion PlRotateQuaternionPoint( const PLQuaternion* q, const PLVector3* point ) {
	PLQuaternion b = PlInverseQuaternion( q );
	b = PlNormalizeQuaternion( &b );

	PLQuaternion a = PlMultiplyQuaternion3FV( q, point );
	return PlMultiplyQuaternion( &a, &b );
}

PL_EXTERN_C_END
