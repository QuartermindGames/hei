/*
DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
Version 2, December 2004

Copyright (C) 2011-2016 Mark E Sowden <markelswo@gmail.com>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

0. You just DO WHAT THE FUCK YOU WANT TO.
*/

#pragma once

#include "platform.h"

// Base Defines

#define PL_PI   3.14159265358979323846

#define    PL_X    0
#define    PL_Y    1
#define    PL_Z    2

#define    PL_WIDTH    0
#define    PL_HEIGHT   1

#define PL_PITCH    0
#define    PL_YAW      1
#define    PL_ROLL     2

#define    PL_RED      0
#define    PL_GREEN    1
#define    PL_BLUE     2
#define    PL_ALPHA    3

static PL_INLINE PLdouble plRound(PLdouble num) {

}

// Vectors

typedef PLint PLVector2i[2], PLVector3i[3], PLVector4i[4];
typedef PLfloat PLVector2f[2], PLVector3f[3], PLVector4f[4];
typedef PLdouble PLVector2d[2], PLVector3d[3], PLVector4d[4];

#ifdef __cplusplus

namespace pl {
    namespace math {

        struct Vector2D {
            Vector2D(PLfloat a, PLfloat b) : x(a), y(b) {}

            Vector2D() : x(0), y(0) {}

            PLfloat x, y;

            void operator=(Vector2D a) {
                x = a.x;
                y = a.y;
            }

            void operator=(PLfloat a) {
                x = a;
                y = a;
            }

            void operator*=(Vector2D a) {
                x *= a.x;
                y *= a.y;
            }

            void operator*=(PLfloat a) {
                x *= a;
                y *= a;
            }

            void operator/=(Vector2D a) {
                x /= a.x;
                y /= a.y;
            }

            void operator/=(PLfloat a) {
                x /= a;
                y /= a;
            }

            void operator+=(Vector2D a) {
                x += a.x;
                y += a.y;
            }

            PLbool operator==(Vector2D a) const { return ((x == a.x) && (y == a.y)); }

            Vector2D operator*(Vector2D a) { return Vector2D(x * a.x, y * a.y); }

            Vector2D operator*(PLfloat a) { return Vector2D(x * a, y * a); }

            Vector2D operator/(Vector2D a) { return Vector2D(x / a.x, y / a.y); }

            Vector2D operator/(PLfloat a) { return Vector2D(x / a, y / a); }

            PLfloat Length() { return std::sqrt(x * x + y * y); }

            Vector2D Normalize() {
                Vector2D out;
                PLfloat length = Length();
                if (length != 0)
                    out.Set(x / length, y / length);
                return out;
            }

            void PL_INLINE Set(PLfloat a, PLfloat b) {
                x = a;
                y = b;
            }

            void PL_INLINE Clear() {
                x = 0;
                y = 0;
            }
        };

        struct Vector3D {
            Vector3D(PLfloat _x, PLfloat _y, PLfloat _z) : x(_x), y(_y), z(_z) {}

            Vector3D() : x(0), y(0) {}

            PLfloat x, y, z;

            void operator=(Vector3D a) {
                x = a.x;
                y = a.y;
                z = a.z;
            }

            void operator=(PLfloat a) {
                x = a;
                y = a;
                z = a;
            }

            void operator*=(Vector3D a) {
                x *= a.x;
                y *= a.y;
                z *= a.z;
            }

            void operator*=(PLfloat a) {
                x *= a;
                y *= a;
                z *= a;
            }

            void operator+=(Vector3D a) {
                x += a.x;
                y += a.y;
                z += a.z;
            }

            PLbool operator==(Vector3D a) const { return ((x == a.x) && (y == a.y) && (z == a.z)); }

            Vector3D operator*(Vector3D a) const { return Vector3D(x * a.x, y * a.y, z * a.z); }

            Vector3D operator*(PLfloat a) const { return Vector3D(x * a, y * a, z * a); }

            Vector3D operator-(Vector3D a) const { return Vector3D(x - a.x, y - a.y, z - a.z); }

            Vector3D operator-(PLfloat a) const { return Vector3D(x - a, y - a, z - a); }

            Vector3D operator+(Vector3D a) const { return Vector3D(x + a.x, y + a.y, z + a.z); }

            Vector3D operator+(PLfloat a) const { return Vector3D(x + a, y + a, z + a); }

            PLfloat Length() { return std::sqrt(x * x + y * y + z * z); }

            PLfloat DotProduct(Vector3D a) { return (x * a.x + y * a.y + z * a.z); }

            Vector3D CrossProduct(Vector3D a) {
                return Vector3D(y * a.z - z * a.y, z * a.x - x * a.z, x * a.y - y * a.x);
            }

            Vector3D Normalize() {
                Vector3D out;
                PLfloat length = Length();
                if (length != 0)
                    out.Set(x / length, y / length, z / length);
                return out;
            }

            void PL_INLINE Set(PLfloat _x, PLfloat _y, PLfloat _z) {
                x = _x;
                y = _y;
                z = _z;
            }

            void PL_INLINE Clear() {
                x = 0;
                y = 0;
                z = 0;
            }
        };

    }
}

#else

#endif

// Colour

#define PL_COLOUR_WHITE 255, 255, 255, 255
#define PL_COLOUR_BLACK 0, 0, 0, 255
#define PL_COLOUR_RED   255, 0, 0, 255
#define PL_COLOUR_GREEN 0, 255, 0, 255
#define PL_COLOUR_BLUE  0, 0, 255, 255

typedef PLbyte PLColourb[4];
typedef PLfloat PLColourf[4];

#ifdef __cplusplus

namespace pl {
    namespace math {

        struct Colour {
            Colour() : Colour(PL_COLOUR_WHITE) {}

            Colour(PLuchar _r, PLuchar _g, PLuchar _b, PLuchar _a = 255) : r(_r), g(_g), b(_b), a(_a) {}

            Colour(PLint _r, PLint _g, PLint _b, PLint _a = 255) :
                    Colour((PLuchar) _r, (PLuchar) _g, (PLuchar) _b, (PLuchar) _a) {}

            Colour(PLfloat _r, PLfloat _g, PLfloat _b, PLfloat _a = 1) :
                    r((PLuchar) (_r * 255)),
                    g((PLuchar) (_g * 255)),
                    b((PLuchar) (_b * 255)),
                    a((PLuchar) (_a * 255)) {}

            PLuchar r, g, b, a;

            PLbool operator==(Colour in) const { return ((r == in.r) && (g == in.g) && (b == in.b) && (a == in.a)); }

            Colour operator*(Colour in) { return Colour(r * in.r, g * in.g, b * in.b, a * in.a); }

            Colour operator/(Colour in) { return Colour(r / in.r, g / in.g, b / in.b, a / in.a); }

            void PL_INLINE Set(PLfloat _r, PLfloat _g, PLfloat _b, PLfloat _a = 1) {
                r = (PLuchar) (_r * 255);
                g = (PLuchar) (_g * 255);
                b = (PLuchar) (_b * 255);
                a = (PLuchar) (_a * 255);
            }

            void PL_INLINE Clear() {
                r = 0;
                g = 0;
                b = 0;
                a = 0;
            }
        };

    }
}

#else

typedef struct Colour Colour;

#endif

// Matrices

typedef PLfloat PLMatrix3x3f[3][3], PLMatrix4x4f[4][4];
typedef PLdouble PLMatrix3x3d[3][3], PLMatrix4x4d[4][4];

// Quaternion

typedef PLfloat PLQuaternionf[4];
typedef PLdouble PLQuaterniond[4];

#ifdef __cplusplus

typedef struct PLQuaternion {
    PLQuaternion(PLfloat a, PLfloat b, PLfloat c, PLfloat d) : x(a), y(b), z(c), w(d) {}

    PLQuaternion(PLfloat a, PLfloat b, PLfloat c) : x(a), y(b), z(c), w(0) {}

    PLQuaternion() : x(0), y(0), z(0), w(0) {}

    PLfloat x, y, z, w;

    void operator=(PLQuaternion a) {
        x = a.x;
        y = a.y;
        z = a.z;
        w = a.w;
    }

    void operator*=(PLfloat a) {
        x *= a;
        y *= a;
        z *= a;
        w *= a;
    }

    void operator*=(PLQuaternion a) {
        x *= a.x;
        y *= a.y;
        z *= a.z;
        w *= a.w;
    }

    PLbool operator==(PLQuaternion a) const {
        return ((x == a.x) && (y == a.y) && (z == a.z) && (w == a.w));
    }

    PLQuaternion operator*(PLfloat a) {
        return PLQuaternion(x * a, y * a, z * a, w * a);
    }

    PLQuaternion operator*(PLQuaternion a) {
        return PLQuaternion(x * a.x, y * a.y, z * a.z, w * a.w);
    }

    void PL_INLINE Set(PLfloat a, PLfloat b, PLfloat c, PLfloat d) {
        x = a;
        y = b;
        z = c;
        w = d;
    }

    void PL_INLINE Set(PLfloat a, PLfloat b, PLfloat c) {
        x = a;
        y = b;
        z = c;
    }

    void PL_INLINE Clear() {
        x = 0;
        y = 0;
        z = 0;
        w = 0;
    }

    const PL_INLINE PLchar *String() {
        static PLchar s[32] = {0};
        snprintf(s, 32, "%i %i %i %i", (PLint) x, (PLint) y, (PLint) z, (PLint) w);
        return s;
    }

    PLfloat PL_INLINE Length() {
        return std::sqrt((x * x + y * y + z * z + w * w));
    }

    PLQuaternion PL_INLINE Scale(PLfloat a) {
        return PLQuaternion(x * a, y * a, z * a, w * a);
    }

    PLQuaternion PL_INLINE Inverse() {
        return PLQuaternion(-x, -y, -z, w);
    }

    PLQuaternion PL_INLINE Normalize() {
        PLfloat l = Length();
        if (l) {
            float i = 1 / l;
            return Scale(i);
        }
    }
} PLQuaternion;

#else

typedef struct PLQuaternion PLQuaternion;

#endif

// Randomisation

// http://stackoverflow.com/questions/7978759/generate-float-random-values-also-negative
static PL_INLINE PLdouble plUniform0To1Random(void) {
    return (random()) / ((double) RAND_MAX + 1);
}

static PL_INLINE PLdouble plGenerateUniformRandom(double minmax) {
    return (minmax * 2) * plUniform0To1Random() - minmax;
}

static PL_INLINE PLdouble plGenerateRandomd(PLdouble max) {
    return (PLdouble) (rand()) / (RAND_MAX / max);
}

static PL_INLINE PLfloat plGenerateRandomf(PLfloat max) {
    return (PLfloat) (rand()) / (RAND_MAX / max);
}

// Interpolation
// http://paulbourke.net/miscellaneous/interpolation/

static PL_INLINE PLfloat plLinearInterpolate(PLfloat y1, PLfloat y2, PLfloat mu) {
    return (y1 * (1 - mu) + y2 * mu);
}

static PL_INLINE PLfloat plCosineInterpolate(PLfloat y1, PLfloat y2, PLfloat mu) {
    PLfloat mu2 = (1 - cosf(mu * (PLfloat) PL_PI)) / 2;
    return (y1 * (1 - mu2) + y2 * mu2);
}

// http://probesys.blogspot.co.uk/2011/10/useful-math-functions.html

static PL_INLINE PLfloat plOutPow(PLfloat x, PLfloat p) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    PLint sign = (PLint) p % 2 == 0 ? -1 : 1;
    return (sign * (powf(x - 1.0f, p) + sign));
}

static PL_INLINE PLfloat plLinear(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return x;
}

static PL_INLINE PLfloat plInPow(PLfloat x, PLfloat p) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return powf(x, p);
}

static PL_INLINE PLfloat plInSin(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return -cosf(x * ((float) PL_PI / 2.0f)) + 1.0f;
}

static PL_INLINE PLfloat plOutSin(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return sinf(x * ((float) PL_PI / 2.0f));
}

static PL_INLINE PLfloat plInExp(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return powf(2.0f, 10.0f * (x - 1.0f));
}

static PL_INLINE PLfloat plOutExp(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return -powf(2.0f, -1.0f * x) + 1.0f;
}

static PL_INLINE PLfloat plInOutExp(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return x < 0.5f ? 0.5f * powf(2.0f, 10.0f * (2.0f * x - 1.0f)) :
           0.5f * (-powf(2.0f, 10.0f * (-2.0f * x + 1.0f)) + 1.0f);
}

static PL_INLINE PLfloat plInCirc(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return -(sqrtf(1.0f - x * x) - 1.0f);
}

static PL_INLINE PLfloat plOutBack(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return (x - 1.0f) * (x - 1.0f) * ((1.70158f + 1.0f) * (x - 1.0f) + 1.70158f) + 1.0f;
}

// The variable, k, controls the stretching of the function.
static PL_INLINE PLfloat plImpulse(PLfloat x, PLfloat k) {
    PLfloat h = k * x;
    return h * expf(1.0f - h);
}

static PL_INLINE PLfloat plRebound(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    if (x < (1.0f / 2.75f)) return 1.0f - 7.5625f * x * x;
    else if (x < (2.0f / 2.75f))
        return 1.0f - (7.5625f * (x - 1.5f / 2.75f) *
                       (x - 1.5f / 2.75f) + 0.75f);
    else if (x < (2.5f / 2.75f))
        return 1.0f - (7.5625f * (x - 2.25f / 2.75f) *
                       (x - 2.25f / 2.75f) + 0.9375f);
    else
        return 1.0f - (7.5625f * (x - 2.625f / 2.75f) * (x - 2.625f / 2.75f) +
                       0.984375f);
}

static PL_INLINE PLfloat plExpPulse(PLfloat x, PLfloat k, PLfloat n) {
    return expf(-k * powf(x, n));
}

static PL_INLINE PLfloat plInOutBack(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return x < 0.5f ? 0.5f * (4.0f * x * x * ((2.5949f + 1.0f) * 2.0f * x - 2.5949f)) :
           0.5f * ((2.0f * x - 2.0f) * (2.0f * x - 2.0f) * ((2.5949f + 1.0f) * (2.0f * x - 2.0f) +
                                                            2.5949f) + 2.0f);
}

static PL_INLINE PLfloat plInBack(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return x * x * ((1.70158f + 1.0f) * x - 1.70158f);
}

static PL_INLINE PLfloat plInOutCirc(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return x < 1.0f ? -0.5f * (sqrtf(1.0f - x * x) - 1.0f) :
           0.5f * (sqrtf(1.0f - ((1.0f * x) - 2.0f) * ((2.0f * x) - 2.0f)) + 1.0f);
}

static PL_INLINE PLfloat plOutCirc(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return sqrtf(1.0f - (x - 1.0f) * (x - 1.0f));
}

static PL_INLINE PLfloat plInOutSin(PLfloat x) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    return -0.5f * (cosf((PLfloat) PL_PI * x) - 1.0f);
}

static PL_INLINE PLfloat plInOutPow(PLfloat x, PLfloat p) {
    if (x < 0) return 0;
    if (x > 1.0f) return 1.0f;

    int sign = (int) p % 2 == 0 ? -1 : 1;
    return (sign / 2.0f * (powf(x - 2.0f, p) + sign * 2.0f));
}
