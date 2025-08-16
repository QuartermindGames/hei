// Copyright © 2020-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

/////////////////////////////////////////////////////////////////////////////////////
// Vector2f
/////////////////////////////////////////////////////////////////////////////////////

typedef struct QmMathVector2f
{
	union
	{
		struct
		{
			float x;
			float y;
		};

		float v[ 2 ];
	};
} QmMathVector2f;

static inline QmMathVector2f qm_math_vector2f( const float x, const float y )
{
	return ( QmMathVector2f ) { .x = x, .y = y };
}

static inline QmMathVector2f qm_math_vector2f_add( const QmMathVector2f src, const QmMathVector2f add )
{
	return qm_math_vector2f( src.x + add.x, src.y + add.y );
}

static inline QmMathVector2f qm_math_vector2f_scale( const QmMathVector2f src, const QmMathVector2f scale )
{
	return qm_math_vector2f( src.x * scale.x, src.y * scale.y );
}

static inline QmMathVector2f qm_math_vector2f_scale_float( const QmMathVector2f src, const float scale )
{
	return qm_math_vector2f_scale( src, qm_math_vector2f( scale, scale ) );
}

static inline QmMathVector2f qm_math_vector2f_invert( const QmMathVector2f src )
{
	return qm_math_vector2f( -src.x, -src.y );
}

static inline float qm_math_vector2f_dot_product( const QmMathVector2f a, const QmMathVector2f b )
{
	return a.x * b.x + a.y * b.y;
}

/////////////////////////////////////////////////////////////////////////////////////
// Vector3f
/////////////////////////////////////////////////////////////////////////////////////

typedef struct QmMathVector3f
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};

		float v[ 3 ];
	};
} QmMathVector3f;

static inline QmMathVector3f qm_math_vector3f( const float x, const float y, const float z )
{
	return ( QmMathVector3f ) { .x = x, .y = y, .z = z };
}

static inline QmMathVector3f qm_math_vector3f_add( const QmMathVector3f src, const QmMathVector3f add )
{
	return qm_math_vector3f( src.x + add.x, src.y + add.y, src.z + add.z );
}

static inline QmMathVector3f qm_math_vector3f_scale( const QmMathVector3f src, const QmMathVector3f scale )
{
	return qm_math_vector3f( src.x * scale.x, src.y * scale.y, src.z * scale.z );
}

static inline QmMathVector3f qm_math_vector3f_scale_float( const QmMathVector3f src, const float scale )
{
	return qm_math_vector3f_scale( src, qm_math_vector3f( scale, scale, scale ) );
}

static inline QmMathVector3f qm_math_vector3f_invert( const QmMathVector3f src )
{
	return qm_math_vector3f( -src.x, -src.y, -src.z );
}

static inline float qm_math_vector3f_dot_product( const QmMathVector3f a, const QmMathVector3f b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/////////////////////////////////////////////////////////////////////////////////////
// Vector4f
/////////////////////////////////////////////////////////////////////////////////////

typedef struct QmMathVector4f
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};

		float v[ 4 ];
	};
} QmMathVector4f;

static inline QmMathVector4f qm_math_vector4f( const float x, const float y, const float z, const float w )
{
	return ( QmMathVector4f ) { .x = x, .y = y, .z = z, .w = w };
}

static inline QmMathVector4f qm_math_vector4f_scale( const QmMathVector4f src, const QmMathVector4f scale )
{
	return qm_math_vector4f( src.x * scale.x, src.y * scale.y, src.z * scale.z, src.w * scale.w );
}

static inline QmMathVector4f qm_math_vector4f_scale_float( const QmMathVector4f src, const float scale )
{
	return qm_math_vector4f_scale( src, qm_math_vector4f( scale, scale, scale, scale ) );
}

static inline QmMathVector4f qm_math_vector4f_invert( const QmMathVector4f src )
{
	return qm_math_vector4f( -src.x, -src.y, -src.z, -src.z );
}

static inline float qm_math_vector4f_dot_product( const QmMathVector4f a, const QmMathVector4f b )
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}
