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
/* Matrices */

/* I know, this is disgusting... */
#define pl_m3pos( row, col ) m[(row) * 3 + (col)]
#define pl_m4pos( row, col ) m[(row) * 4 + (col)]

typedef struct PLMatrix3 {
	float m[9];
	/* 0 0 0
	 * 0 0 0
	 * 0 0 0
	 */

#ifdef __cplusplus

	PLMatrix3() = default;

	PLMatrix3( PLVector3 x, PLVector3 y, PLVector3 z ) {
		m[ 0 ] = x.x;
		m[ 3 ] = y.x;
		m[ 6 ] = z.x;
		m[ 1 ] = x.y;
		m[ 4 ] = y.y;
		m[ 7 ] = z.y;
		m[ 2 ] = x.z;
		m[ 5 ] = y.z;
		m[ 8 ] = z.z;
	}

#endif
} PLMatrix3;

typedef struct PLMatrix3x4 {
	float m[12];
	/* 0 0 0 0
	 * 0 0 0 0
	 * 0 0 0 0
	 */
} PLMatrix3x4;

typedef struct PLMatrix4 PLMatrix4;

PL_INLINE static PLMatrix4 plAddMatrix4( PLMatrix4 m, PLMatrix4 m2 );
PL_INLINE static PLMatrix4 plSubtractMatrix4( PLMatrix4 m, PLMatrix4 m2 );
PL_INLINE static PLMatrix4 plScaleMatrix4( PLMatrix4 m, PLVector3 scale );
PL_INLINE static PLMatrix4 plMultiplyMatrix4( PLMatrix4 m, PLMatrix4 m2 );
PL_INLINE static PLMatrix4 plRotateMatrix4( float angle, PLVector3 axis );
PL_INLINE static PLMatrix4 plTranslateMatrix4( PLVector3 v );
PL_INLINE static PLVector3 plGetMatrix4Translation( const PLMatrix4 *m );
PL_INLINE static PLVector3 plGetMatrix4Angle( const PLMatrix4* m );

typedef struct PLMatrix4 {
	float m[16];
	/* 0 0 0 0
	 * 0 0 0 0
	 * 0 0 0 0
	 * 0 0 0 0
	 */

#ifdef __cplusplus
	PL_INLINE void Identity() {
		m[ 0 ] = 1;
		m[ 1 ] = 0;
		m[ 2 ] = 0;
		m[ 3 ] = 0;
		m[ 4 ] = 0;

		m[ 5 ] = 1;
		m[ 6 ] = 0;
		m[ 7 ] = 0;
		m[ 8 ] = 0;
		m[ 9 ] = 0;

		m[ 10 ] = 1;
		m[ 11 ] = 0;
		m[ 12 ] = 0;
		m[ 13 ] = 0;
		m[ 14 ] = 0;

		m[ 15 ] = 1;
	}

	PL_INLINE void Transpose( const PLMatrix4& m2 ) {
		for ( unsigned int j = 0; j < 4; ++j ) {
			for ( unsigned int i = 0; i < 4; ++i ) {
				pl_m4pos( i, j ) = m2.pl_m4pos( j, i );
			}
		}
	}

	PL_INLINE PLMatrix4 Translate( const PLVector3& position ) {
		return *this *= plTranslateMatrix4( position );
	}

	PL_INLINE PLMatrix4 Rotate( float angle, const PLVector3& axis ) {
		return *this *= plRotateMatrix4( angle, axis );
	}

	PL_INLINE void Clear() {
		for ( float& i : m ) { i = 0; }
	}

	PL_INLINE PLVector3 GetAngle() {
		return plGetMatrix4Angle( this );
	}

	PL_INLINE PLVector3 GetTranslation() {
		return plGetMatrix4Translation( this );
	}

	PL_INLINE PLMatrix4 operator+( PLMatrix4 m2 ) const {
		return plAddMatrix4( *this, m2 );
	}

	PL_INLINE PLMatrix4 operator-( PLMatrix4 m2 ) const {
		return plSubtractMatrix4( *this, m2 );
	}

	PL_INLINE PLMatrix4 operator*( PLVector3 v ) const {
		return plScaleMatrix4( *this, v );
	}

	PL_INLINE PLMatrix4 operator*( PLMatrix4 m2 ) const {
		return plMultiplyMatrix4( *this, m2 );
	}

	PL_INLINE PLMatrix4& operator*=( PLMatrix4 m2 ) {
		return *this = *this * m2;
	}

	PL_INLINE PLMatrix4& operator*=( PLVector3 v ) {
		return *this = *this * v;
	}
#endif
} PLMatrix4;

/* ClearMatrix */

PL_INLINE static void plClearMatrix3( PLMatrix3* m ) {
	for ( unsigned int i = 0; i < 9; ++i ) { m->m[ i ] = 0; }
}

PL_INLINE static void plClearMatrix4( PLMatrix4* m ) {
	for ( unsigned int i = 0; i < 16; ++i ) { m->m[ i ] = 0; }
}

/* Identity */

PL_INLINE static PLMatrix3 plMatrix3Identity( void ) {
	PLMatrix3 out;
	out.m[ 0 ] = 1;
	out.m[ 1 ] = 0;
	out.m[ 2 ] = 0;
	out.m[ 3 ] = 0;
	out.m[ 4 ] = 1;
	out.m[ 5 ] = 0;
	out.m[ 6 ] = 0;
	out.m[ 7 ] = 0;
	out.m[ 8 ] = 1;
	return out;
}

PL_INLINE static PLMatrix4 plMatrix4Identity( void ) {
	PLMatrix4 out = {
		{
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1,
		}
	};
	return out;
}

/* Transpose */

PL_INLINE static PLMatrix3 plTransposeMatrix3( PLMatrix3 m, PLMatrix3 m2 ) {
	for ( unsigned int j = 0; j < 3; ++j ) {
		for ( unsigned int i = 0; i < 3; ++i ) {
			m.pl_m3pos( i, j ) = m2.pl_m3pos( j, i );
		}
	}
	return m;
}

PL_INLINE static PLMatrix4 plTransposeMatrix4( PLMatrix4 m, PLMatrix4 m2 ) {
	for ( unsigned int j = 0; j < 4; ++j ) {
		for ( unsigned int i = 0; i < 4; ++i ) {
			m.pl_m4pos( i, j ) = m2.pl_m4pos( j, i );
		}
	}
	return m;
}

/* Add */

PL_INLINE static PLMatrix3 plAddMatrix3( PLMatrix3 m, PLMatrix3 m2 ) {
	for ( unsigned int i = 0; i < 3; ++i ) {
		for ( unsigned int j = 0; j < 3; ++j ) {
			m.pl_m3pos( i, j ) += m2.pl_m3pos( i, j );
		}
	}
	return m;
}

PL_INLINE static PLMatrix4 plAddMatrix4( PLMatrix4 m, PLMatrix4 m2 ) {
	for ( unsigned int i = 0; i < 4; ++i ) {
		for ( unsigned int j = 0; j < 4; ++j ) {
			m.pl_m4pos( i, j ) += m2.pl_m4pos( i, j );
		}
	}
	return m;
}

/* Subtract */

PL_INLINE static PLMatrix3 plSubtractMatrix3( PLMatrix3 m, PLMatrix3 m2 ) {
	for ( unsigned int i = 0; i < 3; ++i ) {
		for ( unsigned int j = 0; j < 3; ++j ) {
			m.pl_m3pos( i, j ) -= m2.pl_m3pos( i, j );
		}
	}
	return m;
}

PL_INLINE static PLMatrix4 plSubtractMatrix4( PLMatrix4 m, PLMatrix4 m2 ) {
	for ( unsigned int i = 0; i < 4; ++i ) {
		for ( unsigned int j = 0; j < 4; ++j ) {
			m.pl_m4pos( i, j ) -= m2.pl_m4pos( i, j );
		}
	}
	return m;
}

/* Multiply */

PL_INLINE static PLMatrix4 plMultiplyMatrix4( PLMatrix4 m, PLMatrix4 m2 ) {
	PLMatrix4 out;

	out.m[ 0 ] = m.m[ 0 ] * m2.m[ 0 ] + m.m[ 4 ] * m2.m[ 1 ] + m.m[ 8 ] * m2.m[ 2 ] + m.m[ 12 ] * m2.m[ 3 ];
	out.m[ 1 ] = m.m[ 1 ] * m2.m[ 0 ] + m.m[ 5 ] * m2.m[ 1 ] + m.m[ 9 ] * m2.m[ 2 ] + m.m[ 13 ] * m2.m[ 3 ];
	out.m[ 2 ] = m.m[ 2 ] * m2.m[ 0 ] + m.m[ 6 ] * m2.m[ 1 ] + m.m[ 10 ] * m2.m[ 2 ] + m.m[ 14 ] * m2.m[ 3 ];
	out.m[ 3 ] = m.m[ 3 ] * m2.m[ 0 ] + m.m[ 7 ] * m2.m[ 1 ] + m.m[ 11 ] * m2.m[ 2 ] + m.m[ 15 ] * m2.m[ 3 ];

	out.m[ 4 ] = m.m[ 0 ] * m2.m[ 4 ] + m.m[ 4 ] * m2.m[ 5 ] + m.m[ 8 ] * m2.m[ 6 ] + m.m[ 12 ] * m2.m[ 7 ];
	out.m[ 5 ] = m.m[ 1 ] * m2.m[ 4 ] + m.m[ 5 ] * m2.m[ 5 ] + m.m[ 9 ] * m2.m[ 6 ] + m.m[ 13 ] * m2.m[ 7 ];
	out.m[ 6 ] = m.m[ 2 ] * m2.m[ 4 ] + m.m[ 6 ] * m2.m[ 5 ] + m.m[ 10 ] * m2.m[ 6 ] + m.m[ 14 ] * m2.m[ 7 ];
	out.m[ 7 ] = m.m[ 3 ] * m2.m[ 4 ] + m.m[ 7 ] * m2.m[ 5 ] + m.m[ 11 ] * m2.m[ 6 ] + m.m[ 15 ] * m2.m[ 7 ];

	out.m[ 8 ] = m.m[ 0 ] * m2.m[ 8 ] + m.m[ 4 ] * m2.m[ 9 ] + m.m[ 8 ] * m2.m[ 10 ] + m.m[ 12 ] * m2.m[ 11 ];
	out.m[ 9 ] = m.m[ 1 ] * m2.m[ 8 ] + m.m[ 5 ] * m2.m[ 9 ] + m.m[ 9 ] * m2.m[ 10 ] + m.m[ 13 ] * m2.m[ 11 ];
	out.m[ 10 ] = m.m[ 2 ] * m2.m[ 8 ] + m.m[ 6 ] * m2.m[ 9 ] + m.m[ 10 ] * m2.m[ 10 ] + m.m[ 14 ] * m2.m[ 11 ];
	out.m[ 11 ] = m.m[ 3 ] * m2.m[ 8 ] + m.m[ 7 ] * m2.m[ 9 ] + m.m[ 11 ] * m2.m[ 10 ] + m.m[ 15 ] * m2.m[ 11 ];

	out.m[ 12 ] = m.m[ 0 ] * m2.m[ 12 ] + m.m[ 4 ] * m2.m[ 13 ] + m.m[ 8 ] * m2.m[ 14 ] + m.m[ 12 ] * m2.m[ 15 ];
	out.m[ 13 ] = m.m[ 1 ] * m2.m[ 12 ] + m.m[ 5 ] * m2.m[ 13 ] + m.m[ 9 ] * m2.m[ 14 ] + m.m[ 13 ] * m2.m[ 15 ];
	out.m[ 14 ] = m.m[ 2 ] * m2.m[ 12 ] + m.m[ 6 ] * m2.m[ 13 ] + m.m[ 10 ] * m2.m[ 14 ] + m.m[ 14 ] * m2.m[ 15 ];
	out.m[ 15 ] = m.m[ 3 ] * m2.m[ 12 ] + m.m[ 7 ] * m2.m[ 13 ] + m.m[ 11 ] * m2.m[ 14 ] + m.m[ 15 ] * m2.m[ 15 ];

	return out;
}

/* Rotate */

PL_INLINE static PLMatrix4 plRotateMatrix4( float angle, PLVector3 axis ) {
	float s = sinf( angle );
	float c = cosf( angle );
	float t = 1.0f - c;

	PLVector3 tv = PLVector3( t * axis.x, t * axis.y, t * axis.z );
	PLVector3 sv = PLVector3( s * axis.x, s * axis.y, s * axis.z );

	PLMatrix4 m;

	m.pl_m4pos( 0, 0 ) = tv.x * axis.x + c;
	m.pl_m4pos( 1, 0 ) = tv.x * axis.y + sv.z;
	m.pl_m4pos( 2, 0 ) = tv.x * axis.z - sv.y;

	m.pl_m4pos( 0, 1 ) = tv.x * axis.y - sv.z;
	m.pl_m4pos( 1, 1 ) = tv.y * axis.y + c;
	m.pl_m4pos( 2, 1 ) = tv.y * axis.z + sv.x;

	m.pl_m4pos( 0, 2 ) = tv.x * axis.z + sv.y;
	m.pl_m4pos( 1, 2 ) = tv.y * axis.z - sv.x;
	m.pl_m4pos( 2, 2 ) = tv.z * axis.z + c;

	m.pl_m4pos( 3, 0 ) = 0;
	m.pl_m4pos( 3, 1 ) = 0;
	m.pl_m4pos( 3, 2 ) = 0;
	m.pl_m4pos( 0, 3 ) = 0;
	m.pl_m4pos( 1, 3 ) = 0;
	m.pl_m4pos( 2, 3 ) = 0;
	m.pl_m4pos( 3, 3 ) = 1.0f;

	return m;
}

/******************************************************************/
/* Utility Functions */

PL_INLINE static PLVector3 plGetMatrix4Translation( const PLMatrix4 *m ) {
	return PLVector3( m->pl_m4pos( 0, 3 ), m->pl_m4pos( 1, 3 ), m->pl_m4pos( 2, 3 ) );
}

PL_INLINE static PLVector3 plGetMatrix4Angle( const PLMatrix4* m ) {
	PLVector3 out = PLVector3( 0, 0, 0 );
	out.y = plRadiansToDegrees( asinf( m->m[ 8 ] ) );
	if ( m->m[ 10 ] < 0 ) {
		if ( out.y >= 0 ) {
			out.y = 180.f - out.y;
		} else {
			out.y = -180.f - out.y;
		}
	}
	if ( m->m[ 0 ] > -PL_EPSILON && m->m[ 0 ] < PL_EPSILON ) {
		out.x = plRadiansToDegrees( atan2f( m->m[ 1 ], m->m[ 5 ] ) );
	} else {
		out.z = plRadiansToDegrees( atan2f( -m->m[ 4 ], m->m[ 0 ] ) );
		out.x = plRadiansToDegrees( atan2f( -m->m[ 9 ], m->m[ 10 ] ) );
	}
	return out;
}

PL_INLINE static bool plCompareMatrix( const PLMatrix4* m, const PLMatrix4* m2 ) {
	for ( unsigned int i = 0; i < 4; ++i ) {
		for ( unsigned int j = 0; j < 4; ++j ) {
			if ( m->pl_m4pos( i, j ) != m2->pl_m4pos( i, j ) ) {
				return false;
			}
		}
	}
	return true;
}

PL_INLINE static PLMatrix4 plScaleMatrix4( PLMatrix4 m, PLVector3 scale ) {
	m.pl_m4pos( 0, 0 ) *= scale.x;
	m.pl_m4pos( 1, 1 ) *= scale.y;
	m.pl_m4pos( 2, 2 ) *= scale.z;
	return m;
}

/* todo: keep this... ? */
//PL_INLINE static PLMatrix4 plMultiRotateMatrix4(PLMatrix4 m, float angle, PLVector3 axis) {
//  return plMultiplyMatrix4(m, plRotateMatrix4(angle, axis));
//}

PL_INLINE static PLMatrix4 plTranslateMatrix4( PLVector3 v ) {
	PLMatrix4 m = plMatrix4Identity();
	m.pl_m4pos( 0, 3 ) = v.x;
	m.pl_m4pos( 1, 3 ) = v.y;
	m.pl_m4pos( 2, 3 ) = v.z;
	return m;
}

PL_INLINE static PLMatrix4 plLookAt( PLVector3 eye, PLVector3 center, PLVector3 up ) {
	PLVector3 f = plNormalizeVector3( plSubtractVector3( center, eye ) );
	PLVector3 u = plNormalizeVector3( up );
	PLVector3 s = plNormalizeVector3( plVector3CrossProduct( f, u ) );
	u = plVector3CrossProduct( s, f );

	PLMatrix4 out = plMatrix4Identity();
	out.pl_m4pos( 0, 0 ) = s.x;
	out.pl_m4pos( 1, 0 ) = s.y;
	out.pl_m4pos( 2, 0 ) = s.z;
	out.pl_m4pos( 0, 1 ) = u.x;
	out.pl_m4pos( 1, 1 ) = u.y;
	out.pl_m4pos( 2, 1 ) = u.z;
	out.pl_m4pos( 0, 2 ) = -f.x;
	out.pl_m4pos( 1, 2 ) = -f.y;
	out.pl_m4pos( 2, 2 ) = -f.z;
	out.pl_m4pos( 3, 0 ) = -( plVector3DotProduct( s, eye ) );
	out.pl_m4pos( 3, 1 ) = -( plVector3DotProduct( u, eye ) );
	out.pl_m4pos( 3, 2 ) = plVector3DotProduct( f, eye );
	return out;
}

PL_INLINE static PLMatrix4 plFrustum( float left, float right, float bottom, float top, float near, float far ) {
	float m0 = 2.f * near;
	float m1 = right - left;
	float m2 = top - bottom;
	float m3 = far - near;
	return ( PLMatrix4 ) { {
							   m0 / m1, 0, 0, 0,
							   0, m0 / m2, 0, 0,
							   ( right + left ) / m1, ( top + bottom ) / m2, ( -far - near ) / m3, -1.f,
							   0, 0, 0, 1
						   } };
}

PL_INLINE static PLMatrix4 plOrtho( float left, float right, float bottom, float top, float near, float far ) {
	float tx = -( right + left ) / ( right - left );
	float ty = -( top + bottom ) / ( top - bottom );
	float tz = -( far + near ) / ( far - near );
	return ( PLMatrix4 ) { {
							   2 / ( right - left ), 0, 0, 0,
							   0, 2 / ( top - bottom ), 0, 0,
							   0, 0, -2 / ( far - near ), 0,
							   tx, ty, tz, 1
						   } };
}

PL_INLINE static PLMatrix4 plPerspective( float fov, float aspect, float near, float far ) {
	float y_max = near * tanf( fov * PL_PI / 360 );
	float x_max = y_max * aspect;
	return plFrustum( -x_max, x_max, -y_max, y_max, near, far );
}

/* Matrix Stack, sorta mirrors OpenGL behaviour */

typedef enum PLMatrixMode {
	PL_MODELVIEW_MATRIX,
	PL_PROJECTION_MATRIX,
	PL_TEXTURE_MATRIX,

	PL_NUM_MATRIX_MODES
} PLMatrixMode;

void plMatrixMode( PLMatrixMode mode );
PLMatrixMode plGetMatrixMode( void );

PLMatrix4 *plGetMatrix( PLMatrixMode mode );
void plLoadMatrix( const PLMatrix4 *matrix );
void plLoadIdentityMatrix( void );

void plMultiMatrix( const PLMatrix4 *matrix );
void plRotateMatrix( float angle, float x, float y, float z );
void plTranslateMatrix( PLVector3 vector );
void plScaleMatrix( PLVector3 scale );

void plPushMatrix( void );
void plPopMatrix( void );
