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
/* Vectors */

// 2D

typedef struct PLVector2 {
  float x, y;

#ifdef __cplusplus
  PLVector2(float a, float b) : x(a), y(b) {}
  PLVector2() : x(0), y(0) {}

  PLVector2 &operator=(float a) { x = a; y = a; return *this; }

  void operator*=(PLVector2 a) { x *= a.x; y *= a.y; }
  void operator*=(float a) { x *= a; y *= a; }

  void operator/=(PLVector2 a) { x /= a.x; y /= a.y; }
  void operator/=(float a) { x /= a; y /= a; }

  void operator+=(PLVector2 a) { x += a.x; y += a.y; }
  void operator+=(float a) { x += a; y += a; }

  bool operator==(PLVector2 a) const { return ((x == a.x) && (y == a.y)); }
  bool operator==(float a) const { return ((x == a) && (y == a)); }
  bool operator != ( float a ) const { return ( ( x != a ) && ( y != a ) ); }

  PLVector2 operator*(PLVector2 a) const { return PLVector2(x * a.x, y * a.y); }
  PLVector2 operator*(float a) const { return PLVector2(x * a, y * a); }

  PLVector2 operator/(PLVector2 a) const { return PLVector2(x / a.x, y / a.y); }
  PLVector2 operator/(float a) const { return PLVector2(x / a, y / a); }

  PLVector2 operator+(PLVector2 a) const { return PLVector2(x + a.x, y + a.y); }
  PLVector2 operator+(float a) const { return PLVector2(x + a, y + a); }

  PLVector2 operator-(PLVector2 a) const { return PLVector2(x - a.x, y - a.y); }
  PLVector2 operator-(float a) const { return PLVector2(x - a, y - a); }

  float Length() { return std::sqrt(x * x + y * y); }

  PLVector2 Normalize() {
      PLVector2 out;
      float length = Length();
      if (length != 0) {
          out.Set(x / length, y / length);
      }
      return out;
  }

  void Set(float a, float b) { x = a; y = b; }
  void Clear() { x = 0; y = 0; }
#endif
} PLVector2;

#ifndef __cplusplus

#   define PLVector2(x, y)      (PLVector2){x, y}

#endif

PL_INLINE static PLVector2 plAddVector2( PLVector2 v, PLVector2 v2 ) {
    v.x += v2.x;
    v.y += v2.y;
    return v;
}

PL_INLINE static PLVector2 plSubtractVector2( const PLVector2 *a, const PLVector2 *b ) {
	return PLVector2( a->x - b->x, a->y - b->y );
}

PL_INLINE static PLVector2 plScaleVector2( const PLVector2 *v, const PLVector2 *scale ) {
    return PLVector2( v->x * scale->x, v->y * scale->y );
}

PL_INLINE static PLVector2 plScaleVector2f( const PLVector2 *v, float scale ) {
    return PLVector2( v->x * scale, v->y * scale );
}

PL_INLINE static PLVector2 plDivideVector2( PLVector2 v, PLVector2 v2 ) {
    v.x /= v2.x;
    v.y /= v2.y;
    return v;
}

PL_INLINE static PLVector2 plDivideVector2f( const PLVector2 *v, float f ) {
    return PLVector2( v->x / f, v->y / f );
}

PL_INLINE static bool plCompareVector2( const PLVector2 *v, const PLVector2 *v2 ) {
    return ( ( v->x == v2->x ) && ( v->y == v2->y ) );
}

PL_INLINE static float plVector2DotProduct( const PLVector2 *a, const PLVector2 *b ) {
	return a->x * b->x + a->y * b->y;
}

PL_INLINE static float plGetVector2Length( const PLVector2 *v ) {
    return sqrtf( plVector2DotProduct( v, v ) );
}

PL_INLINE static PLVector2 plNormalizeVector2( const PLVector2 *v ) {
    float length = plGetVector2Length( v );
    return PLVector2( v->x / length, v->y / length );
}

PL_INLINE static PLVector2 plComputeLineNormal( const PLVector2 *x, const PLVector2 *y ) {
	PLVector2 v = PLVector2( x->y - y->y, y->x - x->x );
    return plNormalizeVector2( &v );
}

// 3D

typedef struct PLVector3 {
  float x, y, z;

#ifdef __cplusplus
  PL_INLINE PLVector3() : x(0), y(0), z(0) {}
  PL_INLINE PLVector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
  PL_INLINE explicit PLVector3(const float *v) {
      x = v[0]; y = v[1]; z = v[2];
  }

  PL_INLINE PLVector3 &operator = (PLVector2 v) {
      x = v.x;
      y = v.y;
      return *this;
  }

  PL_INLINE PLVector3 &operator = (float a) {
      x = a;
      y = a;
      z = a;
      return *this;
  }

  PL_INLINE void operator *= (const PLVector3 &v) {
      x *= v.x; y *= v.y; z *= v.z;
  }

  PL_INLINE void operator *= (float a) {
      x *= a; y *= a; z *= a;
  }

  PL_INLINE void operator += (PLVector3 a) {
      x += a.x; y += a.y; z += a.z;
  }

  PL_INLINE void operator += (float a) {
      x += a; y += a; z += a;
  }

  PL_INLINE bool operator == (const PLVector3 &a) const {
      return ((x == a.x) && (y == a.y) && (z == a.z));
  }

  PL_INLINE bool operator == (float f) const {
      return ((x == f) && (y == f) && (z == f));
  }

  PL_INLINE bool operator != (const PLVector3 &a) const {
      return !(a == *this);
  }

  PL_INLINE bool operator != (float f) const {
      return ((x != f) && (y != f) && (z != f));
  }

  PL_INLINE PLVector3 operator * (PLVector3 a) const {
      return {x * a.x, y * a.y, z * a.z};
  }

  PL_INLINE PLVector3 operator * (float a) const {
      return {x * a, y * a, z * a};
  }

  PL_INLINE PLVector3 operator - (PLVector3 a) const {
      return {x - a.x, y - a.y, z - a.z};
  }

  PL_INLINE PLVector3 operator - (float a) const {
      return {x - a, y - a, z - a};
  }

  PLVector3 PL_INLINE operator - () const {
      return {-x, -y, -z};
  }

  PLVector3 PL_INLINE operator + (PLVector3 a) const {
      return {x + a.x, y + a.y, z + a.z};
  }

  PLVector3 PL_INLINE operator + (float a) const {
      return {x + a, y + a, z + a};
  }

  PLVector3 PL_INLINE operator / (const PLVector3 &a) const {
      return {x / a.x, y / a.y, z / a.z};
  }

  PLVector3 PL_INLINE operator / (float a) const {
      return {x / a, y / a, z / a};
  }

  PL_INLINE float& operator [] (const unsigned int i) {
      return *((&x) + i);
  }

  PL_INLINE bool operator > (const PLVector3 &v) const {
      return (x > v.x)
        || (x == v.x && y > v.y)
        || (x == v.x && y == v.y && z > v.z);
  }

  PL_INLINE bool operator < (const PLVector3 &v) const {
      return (x < v.x)
        || (x == v.x && y < v.y)
        || (x == v.x && y == v.y && z < v.z);
  }

  PL_INLINE bool operator >= (const PLVector3 &v) const {
      return *this > v || *this == v;
  }

  PL_INLINE bool operator <= (const PLVector3 &v) const {
      return *this < v || *this == v;
  }

  PL_INLINE float Length() const {
      return std::sqrt(x * x + y * y + z * z);
  }

  PL_INLINE float DotProduct(PLVector3 a) const {
      return (x * a.x + y * a.y + z * a.z);
  }

  PL_INLINE PLVector3 CrossProduct(PLVector3 a) const {
      return {y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x};
  }

  PL_INLINE PLVector3 Normalize() const {
      return (*this) / Length();
  }

  PL_INLINE float Difference(PLVector3 v) const {
      return ((*this) - v).Length();
  }

  void PL_INLINE Set(float _x, float _y, float _z) {
      x = _x; y = _y; z = _z;
  }
#endif
} PLVector3;

#ifndef __cplusplus

#   define PLVector3(x, y, z)   (PLVector3){ (float)x, (float)y, (float)z }

#endif

#define plVector3Index( VECTOR, INDEX ) ( ( float* ) &( VECTOR ) )[ INDEX ]

PL_INLINE static PLVector3 plClampVector3( const PLVector3* v, float min, float max ) {
    return PLVector3(
        plClamp( min, v->x, max ),
        plClamp( min, v->y, max ),
        plClamp( min, v->z, max )
    );
}

PL_INLINE static PLVector3 plAddVector3(PLVector3 v, PLVector3 v2) {
  v.x += v2.x;
  v.y += v2.y;
  v.z += v2.z;
  return v;
}

PL_INLINE static PLVector3 plSubtractVector3(PLVector3 v, PLVector3 v2) {
  v.x -= v2.x;
  v.y -= v2.y;
  v.z -= v2.z;
  return v;
}

PL_INLINE static PLVector3 plScaleVector3(PLVector3 v, PLVector3 v2) {
  v.x *= v2.x;
  v.y *= v2.y;
  v.z *= v2.z;
  return v;
}

PL_INLINE static PLVector3 plScaleVector3f(PLVector3 v, float f) {
  v.x *= f;
  v.y *= f;
  v.z *= f;
  return v;
}

PL_INLINE static PLVector3 plDivideVector3(PLVector3 v, PLVector3 v2) {
  v.x /= v2.x;
  v.y /= v2.y;
  v.z /= v2.z;
  return v;
}

PL_INLINE static PLVector3 plDivideVector3f(PLVector3 v, float v2) {
  v.x /= v2;
  v.y /= v2;
  v.z /= v2;
  return v;
}

PL_INLINE static PLVector3 plInverseVector3(PLVector3 v) {
  v.x = -v.x;
  v.y = -v.y;
  v.z = -v.z;
  return v;
}

PL_INLINE static bool plCompareVector3(const PLVector3 *v, const PLVector3 *v2) {
  return ((v->x == v2->x) && (v->y == v2->y) && (v->z == v2->z));
}

PL_INLINE static PLVector3 plVector3CrossProduct(PLVector3 v, PLVector3 v2) {
  return PLVector3(
      v.y * v2.z - v.z * v2.y,
      v.z * v2.x - v.x * v2.z,
      v.x * v2.y - v.y * v2.x
  );
}

PL_INLINE static PLVector3 plVector3Max(PLVector3 v, PLVector3 v2) {
  return PLVector3(
      v.x > v2.x ? v.x : v2.x,
      v.y > v2.y ? v.y : v2.y,
      v.z > v2.z ? v.z : v2.z
  );
}

PL_INLINE static PLVector3 plVector3Min(PLVector3 v, PLVector3 v2) {
  return PLVector3(
      v.x < v2.x ? v.x : v2.x,
      v.y < v2.y ? v.y : v2.y,
      v.z < v2.z ? v.z : v2.z
  );
}

PL_INLINE static float plVector3DotProduct(PLVector3 v, PLVector3 v2) {
  return (v.x * v2.x + v.y * v2.y + v.z * v2.z);
}

PL_INLINE static float plVector3Length(const PLVector3 *v) {
  return sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
}

PL_INLINE static PLVector3 plNormalizeVector3(PLVector3 v) {
  float length = plVector3Length(&v);
  if (length != 0) {
    v.x /= length;
    v.y /= length;
    v.z /= length;
  }
  return v;
}

PL_INLINE static const char *plPrintVector3(const PLVector3 *v, PLVariableType format) {
  static char s[64];
  if (format == pl_int_var) snprintf(s, 32, "%i %i %i", (int) v->x, (int) v->y, (int) v->z);
  else snprintf(s, 64, "%f %f %f", v->x, v->y, v->z);
  return s;
}

PL_INLINE static const char *plPrintVector2(const PLVector2 *v, PLVariableType format) {
  static char s[64];
  if (format == pl_int_var) snprintf(s, 32, "%i %i", (int) v->x, (int) v->y);
  else snprintf(s, 64, "%f %f", v->x, v->y);
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

PL_INLINE static PLVector4 plAddVector4(PLVector4 v, PLVector4 v2) {
  v.x += v2.x;
  v.y += v2.y;
  v.z += v2.z;
  v.w += v2.w;
  return v;
}

/**
 * Function that works similarly to D3DXPlaneDotCoord.
 */
PL_INLINE static float plGetPlaneDotProduct( const PLVector4 *plane, const PLVector3 *vector ) {
	return plane->x * vector->x + plane->y * vector->y + plane->z * vector->z + plane->w * 1.0f;
}

/**
 * Function that works similarly to D3DXPlaneNormalize.
 */
PL_INLINE static PLVector4 plNormalizePlane( PLVector4 plane ) {
	float normal = sqrtf( plane.x * plane.x + plane.y * plane.y + plane.y * plane.y );
	if ( normal > 0.0f ) {
		plane.x /= normal;
		plane.y /= normal;
		plane.z /= normal;
		plane.w /= normal;
	} else {
		plane.x = plane.y = plane.z = plane.w = 0.0f;
	}

	return plane;
}
