/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

PL_EXTERN_C

/******************************************************************/
/* Matrices */

#define PL_M3_POS( ROW, COL ) ( ( ROW ) + 3 * ( COL ) )
#define PL_M4_POS( ROW, COL ) ( ( ROW ) + 4 * ( COL ) )

/* I know, this is disgusting... */
#define pl_m3pos( row, col ) m[ PL_M3_POS( row, col ) ]
#define pl_m4pos( row, col ) m[ PL_M4_POS( row, col ) ]

typedef struct PLMatrix3 {
	union {
		float m[ 9 ];
		float mm[ 3 ][ 3 ];// row, col
	};
} PLMatrix3;

typedef struct PLMatrix4 {
	union {
		float m[ 16 ];
		float mm[ 4 ][ 4 ];// row, col
	};
} PLMatrix4;

inline static PLMatrix4 PlAddMatrix4( PLMatrix4 m, PLMatrix4 m2 );
inline static PLMatrix4 PlSubtractMatrix4( PLMatrix4 m, PLMatrix4 m2 );
inline static PLMatrix4 PlScaleMatrix4( PLMatrix4 m, PLVector3 scale );
inline static PLMatrix4 PlMultiplyMatrix4( const PLMatrix4 *m, const PLMatrix4 *m2 );
PLMatrix4 PlRotateMatrix4( float angle, const PLVector3 *axis );
inline static PLMatrix4 PlTranslateMatrix4( PLVector3 v );
inline static PLMatrix4 PlInverseMatrix4( PLMatrix4 m );

PLVector3 PlGetMatrix4Translation( const PLMatrix4 *m );
PLVector3 PlGetMatrix4Angle( const PLMatrix4 *m );

PLVector2 PlConvertWorldToScreen( const PLVector3 *position, const PLMatrix4 *viewProjMatrix, const int *viewport, float *w, bool flip );
PLVector3 PlConvertScreenToWorld( PLVector3 windowCoordinate, PLMatrix4 modelView, PLMatrix4 projection, const int viewport[ 4 ] );

void PlExtractMatrix4Directions( const PLMatrix4 *matrix, PLVector3 *left, PLVector3 *up, PLVector3 *forward );
;

#ifdef __cplusplus
namespace hei {
	struct Matrix4 : PLMatrix4 {
		Matrix4() {
			Identity();
		}

		inline void Identity() {
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

		inline void Transpose( const Matrix4 &m2 ) {
			for ( unsigned int j = 0; j < 4; ++j ) {
				for ( unsigned int i = 0; i < 4; ++i ) {
					pl_m4pos( i, j ) = m2.pl_m4pos( j, i );
				}
			}
		}

		////////////////////////////////////////////////////////

		inline Matrix4 Translate( const PLVector3 &position ) {
			return *this *= PlTranslateMatrix4( position );
		}

		inline void SetTranslation( const hei::Vector3 &position ) {
			mm[ 3 ][ 0 ] = position.x;
			mm[ 3 ][ 1 ] = position.y;
			mm[ 3 ][ 2 ] = position.z;
		}

		inline PLVector3 GetTranslation() {
			return PlGetMatrix4Translation( this );
		}

		////////////////////////////////////////////////////////

		inline Matrix4 Rotate( float angle, const PLVector3 &axis ) {
			return *this *= PlRotateMatrix4( angle, &axis );
		}

		inline void Clear() {
			for ( float &i : m ) { i = 0; }
		}

		inline PLVector3 GetAngle() {
			return PlGetMatrix4Angle( this );
		}

		inline PLMatrix4 operator+( PLMatrix4 m2 ) const {
			return PlAddMatrix4( *this, m2 );
		}

		inline PLMatrix4 operator-( PLMatrix4 m2 ) const {
			return PlSubtractMatrix4( *this, m2 );
		}

		inline PLMatrix4 operator*( PLVector3 v ) const {
			return PlScaleMatrix4( *this, v );
		}

		inline PLMatrix4 operator*( PLMatrix4 m2 ) const {
			return PlMultiplyMatrix4( this, &m2 );
		}

		inline Matrix4 &operator=( const PLMatrix4 &m2 ) {
			memcpy( m, m2.m, sizeof( m ) );
			return *this;
		}

		inline Matrix4 &operator*=( PLMatrix4 m2 ) {
			return *this = *this * m2;
		}

		inline Matrix4 &operator*=( PLVector3 v ) {
			return *this = *this * v;
		}
	};
}// namespace hei
#endif

/* ClearMatrix */

inline static void PlClearMatrix3( PLMatrix3 *m ) {
	PL_ZERO( m, sizeof( PLMatrix3 ) );
}

inline static void PlClearMatrix4( PLMatrix4 *m ) {
	PL_ZERO( m, sizeof( PLMatrix4 ) );
}

/* Identity */

inline static PLMatrix3 PlMatrix3Identity( void ) {
	PLMatrix3 m = {};
	m.mm[ 0 ][ 0 ] = 1.0f;
	m.mm[ 1 ][ 1 ] = 1.0f;
	m.mm[ 2 ][ 2 ] = 1.0f;
	return m;
}

inline static PLMatrix4 PlMatrix4Identity( void ) {
	PLMatrix4 m = {};
	m.mm[ 0 ][ 0 ] = 1.0f;
	m.mm[ 1 ][ 1 ] = 1.0f;
	m.mm[ 2 ][ 2 ] = 1.0f;
	m.mm[ 3 ][ 3 ] = 1.0f;
	return m;
}

/* Transpose */

inline static PLMatrix3 PlTransposeMatrix3( PLMatrix3 m, PLMatrix3 m2 ) {
	for ( unsigned int j = 0; j < 3; ++j ) {
		for ( unsigned int i = 0; i < 3; ++i ) {
			m.pl_m3pos( i, j ) = m2.pl_m3pos( j, i );
		}
	}
	return m;
}

inline static PLMatrix4 PlTransposeMatrix4( const PLMatrix4 *m ) {
	PLMatrix4 out;
	for ( unsigned int j = 0; j < 4; ++j ) {
		for ( unsigned int i = 0; i < 4; ++i ) {
			out.pl_m4pos( i, j ) = m->pl_m4pos( j, i );
		}
	}

	return out;
}

/* Add */

inline static PLMatrix3 PlAddMatrix3( PLMatrix3 m, PLMatrix3 m2 ) {
	for ( unsigned int i = 0; i < 3; ++i ) {
		for ( unsigned int j = 0; j < 3; ++j ) {
			m.pl_m3pos( i, j ) += m2.pl_m3pos( i, j );
		}
	}
	return m;
}

inline static PLMatrix4 PlAddMatrix4( PLMatrix4 m, PLMatrix4 m2 ) {
	for ( unsigned int i = 0; i < 4; ++i ) {
		for ( unsigned int j = 0; j < 4; ++j ) {
			m.pl_m4pos( i, j ) += m2.pl_m4pos( i, j );
		}
	}
	return m;
}

/* Subtract */

inline static PLMatrix3 PlSubtractMatrix3( PLMatrix3 m, PLMatrix3 m2 ) {
	for ( unsigned int i = 0; i < 3; ++i ) {
		for ( unsigned int j = 0; j < 3; ++j ) {
			m.pl_m3pos( i, j ) -= m2.pl_m3pos( i, j );
		}
	}
	return m;
}

inline static PLMatrix4 PlSubtractMatrix4( PLMatrix4 m, PLMatrix4 m2 ) {
	for ( unsigned int i = 0; i < 4; ++i ) {
		for ( unsigned int j = 0; j < 4; ++j ) {
			m.pl_m4pos( i, j ) -= m2.pl_m4pos( i, j );
		}
	}
	return m;
}

/* Multiply */

inline static PLMatrix4 PlMultiplyMatrix4( const PLMatrix4 *m, const PLMatrix4 *m2 ) {
	PLMatrix4 out;
	for ( unsigned int col = 0; col < 4; ++col ) {
		for ( unsigned int row = 0; row < 4; ++row ) {
			float sum = 0.0f;
			for ( unsigned int i = 0; i < 4; ++i ) {
				sum += m->mm[ i ][ row ] * m2->mm[ col ][ i ];
			}
			out.mm[ col ][ row ] = sum;
		}
	}

	return out;
}

/******************************************************************/
/* Utility Functions */

inline static bool PlCompareMatrix( const PLMatrix4 *m, const PLMatrix4 *m2 ) {
	for ( unsigned int i = 0; i < 4; ++i ) {
		for ( unsigned int j = 0; j < 4; ++j ) {
			if ( m->pl_m4pos( i, j ) != m2->pl_m4pos( i, j ) ) {
				return false;
			}
		}
	}
	return true;
}

inline static PLMatrix4 PlScaleMatrix4( PLMatrix4 m, PLVector3 scale ) {
	m.pl_m4pos( 0, 0 ) *= scale.x;
	m.pl_m4pos( 0, 1 ) *= scale.x;
	m.pl_m4pos( 0, 2 ) *= scale.x;
	m.pl_m4pos( 0, 3 ) *= scale.x;
	m.pl_m4pos( 1, 0 ) *= scale.y;
	m.pl_m4pos( 1, 1 ) *= scale.y;
	m.pl_m4pos( 1, 2 ) *= scale.y;
	m.pl_m4pos( 1, 3 ) *= scale.y;
	m.pl_m4pos( 2, 0 ) *= scale.z;
	m.pl_m4pos( 2, 1 ) *= scale.z;
	m.pl_m4pos( 2, 2 ) *= scale.z;
	m.pl_m4pos( 2, 3 ) *= scale.z;
	return m;
}

inline static PLMatrix4 PlTranslateMatrix4( PLVector3 v ) {
	PLMatrix4 m = PlMatrix4Identity();
	m.mm[ 3 ][ 0 ] = v.x;
	m.mm[ 3 ][ 1 ] = v.y;
	m.mm[ 3 ][ 2 ] = v.z;
	return m;
}

inline static PLMatrix4 PlInverseMatrix4( PLMatrix4 m ) {
	PLMatrix4 out;

	out.m[ 0 ] =
	        m.m[ 5 ] * m.m[ 10 ] * m.m[ 15 ] -
	        m.m[ 5 ] * m.m[ 11 ] * m.m[ 14 ] -
	        m.m[ 9 ] * m.m[ 6 ] * m.m[ 15 ] +
	        m.m[ 9 ] * m.m[ 7 ] * m.m[ 14 ] +
	        m.m[ 13 ] * m.m[ 6 ] * m.m[ 11 ] -
	        m.m[ 13 ] * m.m[ 7 ] * m.m[ 10 ];
	out.m[ 4 ] =
	        -m.m[ 4 ] * m.m[ 10 ] * m.m[ 15 ] +
	        m.m[ 4 ] * m.m[ 11 ] * m.m[ 14 ] +
	        m.m[ 8 ] * m.m[ 6 ] * m.m[ 15 ] -
	        m.m[ 8 ] * m.m[ 7 ] * m.m[ 14 ] -
	        m.m[ 12 ] * m.m[ 6 ] * m.m[ 11 ] +
	        m.m[ 12 ] * m.m[ 7 ] * m.m[ 10 ];
	out.m[ 8 ] =
	        m.m[ 4 ] * m.m[ 9 ] * m.m[ 15 ] -
	        m.m[ 4 ] * m.m[ 11 ] * m.m[ 13 ] -
	        m.m[ 8 ] * m.m[ 5 ] * m.m[ 15 ] +
	        m.m[ 8 ] * m.m[ 7 ] * m.m[ 13 ] +
	        m.m[ 12 ] * m.m[ 5 ] * m.m[ 11 ] -
	        m.m[ 12 ] * m.m[ 7 ] * m.m[ 9 ];
	out.m[ 12 ] =
	        -m.m[ 4 ] * m.m[ 9 ] * m.m[ 14 ] +
	        m.m[ 4 ] * m.m[ 10 ] * m.m[ 13 ] +
	        m.m[ 8 ] * m.m[ 5 ] * m.m[ 14 ] -
	        m.m[ 8 ] * m.m[ 6 ] * m.m[ 13 ] -
	        m.m[ 12 ] * m.m[ 5 ] * m.m[ 10 ] +
	        m.m[ 12 ] * m.m[ 6 ] * m.m[ 9 ];
	out.m[ 1 ] =
	        -m.m[ 1 ] * m.m[ 10 ] * m.m[ 15 ] +
	        m.m[ 1 ] * m.m[ 11 ] * m.m[ 14 ] +
	        m.m[ 9 ] * m.m[ 2 ] * m.m[ 15 ] -
	        m.m[ 9 ] * m.m[ 3 ] * m.m[ 14 ] -
	        m.m[ 13 ] * m.m[ 2 ] * m.m[ 11 ] +
	        m.m[ 13 ] * m.m[ 3 ] * m.m[ 10 ];
	out.m[ 5 ] =
	        m.m[ 0 ] * m.m[ 10 ] * m.m[ 15 ] -
	        m.m[ 0 ] * m.m[ 11 ] * m.m[ 14 ] -
	        m.m[ 8 ] * m.m[ 2 ] * m.m[ 15 ] +
	        m.m[ 8 ] * m.m[ 3 ] * m.m[ 14 ] +
	        m.m[ 12 ] * m.m[ 2 ] * m.m[ 11 ] -
	        m.m[ 12 ] * m.m[ 3 ] * m.m[ 10 ];
	out.m[ 9 ] =
	        -m.m[ 0 ] * m.m[ 9 ] * m.m[ 15 ] +
	        m.m[ 0 ] * m.m[ 11 ] * m.m[ 13 ] +
	        m.m[ 8 ] * m.m[ 1 ] * m.m[ 15 ] -
	        m.m[ 8 ] * m.m[ 3 ] * m.m[ 13 ] -
	        m.m[ 12 ] * m.m[ 1 ] * m.m[ 11 ] +
	        m.m[ 12 ] * m.m[ 3 ] * m.m[ 9 ];
	out.m[ 13 ] =
	        m.m[ 0 ] * m.m[ 9 ] * m.m[ 14 ] -
	        m.m[ 0 ] * m.m[ 10 ] * m.m[ 13 ] -
	        m.m[ 8 ] * m.m[ 1 ] * m.m[ 14 ] +
	        m.m[ 8 ] * m.m[ 2 ] * m.m[ 13 ] +
	        m.m[ 12 ] * m.m[ 1 ] * m.m[ 10 ] -
	        m.m[ 12 ] * m.m[ 2 ] * m.m[ 9 ];
	out.m[ 2 ] =
	        m.m[ 1 ] * m.m[ 6 ] * m.m[ 15 ] -
	        m.m[ 1 ] * m.m[ 7 ] * m.m[ 14 ] -
	        m.m[ 5 ] * m.m[ 2 ] * m.m[ 15 ] +
	        m.m[ 5 ] * m.m[ 3 ] * m.m[ 14 ] +
	        m.m[ 13 ] * m.m[ 2 ] * m.m[ 7 ] -
	        m.m[ 13 ] * m.m[ 3 ] * m.m[ 6 ];
	out.m[ 6 ] =
	        -m.m[ 0 ] * m.m[ 6 ] * m.m[ 15 ] +
	        m.m[ 0 ] * m.m[ 7 ] * m.m[ 14 ] +
	        m.m[ 4 ] * m.m[ 2 ] * m.m[ 15 ] -
	        m.m[ 4 ] * m.m[ 3 ] * m.m[ 14 ] -
	        m.m[ 12 ] * m.m[ 2 ] * m.m[ 7 ] +
	        m.m[ 12 ] * m.m[ 3 ] * m.m[ 6 ];
	out.m[ 10 ] =
	        m.m[ 0 ] * m.m[ 5 ] * m.m[ 15 ] -
	        m.m[ 0 ] * m.m[ 7 ] * m.m[ 13 ] -
	        m.m[ 4 ] * m.m[ 1 ] * m.m[ 15 ] +
	        m.m[ 4 ] * m.m[ 3 ] * m.m[ 13 ] +
	        m.m[ 12 ] * m.m[ 1 ] * m.m[ 7 ] -
	        m.m[ 12 ] * m.m[ 3 ] * m.m[ 5 ];
	out.m[ 14 ] =
	        -m.m[ 0 ] * m.m[ 5 ] * m.m[ 14 ] +
	        m.m[ 0 ] * m.m[ 6 ] * m.m[ 13 ] +
	        m.m[ 4 ] * m.m[ 1 ] * m.m[ 14 ] -
	        m.m[ 4 ] * m.m[ 2 ] * m.m[ 13 ] -
	        m.m[ 12 ] * m.m[ 1 ] * m.m[ 6 ] +
	        m.m[ 12 ] * m.m[ 2 ] * m.m[ 5 ];
	out.m[ 3 ] =
	        -m.m[ 1 ] * m.m[ 6 ] * m.m[ 11 ] +
	        m.m[ 1 ] * m.m[ 7 ] * m.m[ 10 ] +
	        m.m[ 5 ] * m.m[ 2 ] * m.m[ 11 ] -
	        m.m[ 5 ] * m.m[ 3 ] * m.m[ 10 ] -
	        m.m[ 9 ] * m.m[ 2 ] * m.m[ 7 ] +
	        m.m[ 9 ] * m.m[ 3 ] * m.m[ 6 ];
	out.m[ 7 ] =
	        m.m[ 0 ] * m.m[ 6 ] * m.m[ 11 ] -
	        m.m[ 0 ] * m.m[ 7 ] * m.m[ 10 ] -
	        m.m[ 4 ] * m.m[ 2 ] * m.m[ 11 ] +
	        m.m[ 4 ] * m.m[ 3 ] * m.m[ 10 ] +
	        m.m[ 8 ] * m.m[ 2 ] * m.m[ 7 ] -
	        m.m[ 8 ] * m.m[ 3 ] * m.m[ 6 ];
	out.m[ 11 ] =
	        -m.m[ 0 ] * m.m[ 5 ] * m.m[ 11 ] +
	        m.m[ 0 ] * m.m[ 7 ] * m.m[ 9 ] +
	        m.m[ 4 ] * m.m[ 1 ] * m.m[ 11 ] -
	        m.m[ 4 ] * m.m[ 3 ] * m.m[ 9 ] -
	        m.m[ 8 ] * m.m[ 1 ] * m.m[ 7 ] +
	        m.m[ 8 ] * m.m[ 3 ] * m.m[ 5 ];
	out.m[ 15 ] =
	        m.m[ 0 ] * m.m[ 5 ] * m.m[ 10 ] -
	        m.m[ 0 ] * m.m[ 6 ] * m.m[ 9 ] -
	        m.m[ 4 ] * m.m[ 1 ] * m.m[ 10 ] +
	        m.m[ 4 ] * m.m[ 2 ] * m.m[ 9 ] +
	        m.m[ 8 ] * m.m[ 1 ] * m.m[ 6 ] -
	        m.m[ 8 ] * m.m[ 2 ] * m.m[ 5 ];

	float d = m.m[ 0 ] * out.m[ 0 ] + m.m[ 1 ] * out.m[ 4 ] + m.m[ 2 ] * out.m[ 8 ] + m.m[ 3 ] * out.m[ 12 ];
	if ( d == 0 ) {
		return m;
	}

	d = 1.0f / d;

	for ( unsigned int i = 0; i < 16; ++i ) {
		out.m[ i ] *= d;
	}

	return out;
}

PLMatrix4 PlLookAt( PLVector3 eye, PLVector3 center, PLVector3 up );

inline static PLMatrix4 PlFrustum( float left, float right, float bottom, float top, float nearf, float farf ) {
	float m0 = 2.0f * nearf;
	float m1 = right - left;
	float m2 = top - bottom;
	float m3 = farf - nearf;

	PLMatrix4 m;

	m.mm[ 0 ][ 0 ] = m0 / m1;
	m.mm[ 0 ][ 1 ] = 0.0f;
	m.mm[ 0 ][ 2 ] = ( right + left ) / m1;
	m.mm[ 0 ][ 3 ] = 0.0f;

	m.mm[ 1 ][ 0 ] = 0.0f;
	m.mm[ 1 ][ 1 ] = m0 / m2;
	m.mm[ 1 ][ 2 ] = ( top + bottom ) / m2;
	m.mm[ 1 ][ 3 ] = 0.0f;

	m.mm[ 2 ][ 0 ] = 0.0f;
	m.mm[ 2 ][ 1 ] = 0.0f;
	m.mm[ 2 ][ 2 ] = -( farf + nearf ) / m3;
	m.mm[ 2 ][ 3 ] = -1.0f;

	m.mm[ 3 ][ 0 ] = 0.0f;
	m.mm[ 3 ][ 1 ] = 0.0f;
	m.mm[ 3 ][ 2 ] = -1.0f;
	m.mm[ 3 ][ 3 ] = 0.0f;

	return m;
}

inline static PLMatrix4 PlOrtho( float left, float right, float bottom, float top, float nearf, float farf ) {
	float tx = -( right + left ) / ( right - left );
	float ty = -( top + bottom ) / ( top - bottom );
	float tz = -( farf + nearf ) / ( farf - nearf );

	PLMatrix4 frustumMatrix;

	frustumMatrix.m[ 0 ] = 2 / ( right - left );
	frustumMatrix.m[ 1 ] = 0.0f;
	frustumMatrix.m[ 2 ] = 0.0f;
	frustumMatrix.m[ 3 ] = 0.0f;

	frustumMatrix.m[ 4 ] = 0.0f;
	frustumMatrix.m[ 5 ] = 2 / ( top - bottom );
	frustumMatrix.m[ 6 ] = 0.0f;
	frustumMatrix.m[ 7 ] = 0.0f;

	frustumMatrix.m[ 8 ] = 0.0f;
	frustumMatrix.m[ 9 ] = 0.0f;
	frustumMatrix.m[ 10 ] = -2 / ( farf - nearf );
	frustumMatrix.m[ 11 ] = 0.0f;

	frustumMatrix.m[ 12 ] = tx;
	frustumMatrix.m[ 13 ] = ty;
	frustumMatrix.m[ 14 ] = tz;
	frustumMatrix.m[ 15 ] = 1.0f;

	return frustumMatrix;
}

inline static PLMatrix4 PlPerspective( float fov, float aspect, float nearf, float farf ) {
	float y_max = nearf * tanf( fov * PL_PI / 360.0f );
	float x_max = y_max * aspect;
	return PlFrustum( -x_max, x_max, -y_max, y_max, nearf, farf );
}

/* Matrix Stack, sorta mirrors OpenGL behaviour
 * TODO: move this into plgraphics */

typedef enum PLMatrixMode {
	PL_MODELVIEW_MATRIX,
	PL_VIEW_MATRIX,
	PL_PROJECTION_MATRIX,
	PL_TEXTURE_MATRIX,

	PL_NUM_MATRIX_MODES
} PLMatrixMode;

void PlMatrixMode( PLMatrixMode mode );
PLMatrixMode PlGetMatrixMode( void );

PLMatrix4 *PlGetMatrix( PLMatrixMode mode );
void PlLoadMatrix( const PLMatrix4 *matrix );
void PlLoadIdentityMatrix( void );

void PlMultiMatrix( const PLMatrix4 *matrix );
void PlRotateMatrix( float angle, const PLVector3 *axis );
void PlRotateMatrix3f( float angle, float x, float y, float z );
void PlTranslateMatrix( PLVector3 vector );
void PlScaleMatrix( PLVector3 scale );
void PlInverseMatrix( void );

void PlPushMatrix( void );
void PlPopMatrix( void );

PL_EXTERN_C_END
