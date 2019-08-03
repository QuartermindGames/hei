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

#include "platform.h"

// Base Defines

#define PL_PI           3.14159265358979323846264338327950288f
#define PL_PI_DIV_180   (PL_PI / 180.f)

#define PL_TAU          6.28318530717958647692528676655900576f
#define PL_EPSILON      1.19209290e-7f

#define PL_MAX_UINT     (unsigned int)(-1)

enum {
    // Colours
    PL_RED = 0,
    PL_GREEN,
    PL_BLUE,
    PL_ALPHA
};

#define plFloatToByte(a)    (uint8_t)(roundf((a) * 255.f))
#define plByteToFloat(a)    ((a) / (float)255)

#define plClamp(min, val, max) (val) < (min) ? (min) : ((val) > (max) ? (max) : (val))

PL_INLINE static bool plIsPowerOfTwo(unsigned int num) {
    return (bool)((num != 0) && ((num & (~num + 1)) == num));
}

PL_INLINE static float plDegreesToRadians(float degrees) {
    return degrees * (PL_PI / 180);
}

PL_INLINE static float plRadiansToDegrees(float radians) {
    return radians * (180 / PL_PI);
}

/* https://stackoverflow.com/a/9194117 */
PL_INLINE static int plRoundUp(int num, int multiple) {
    return (num + multiple - 1) & -multiple;
}

#include "pl_math_vector.h"

/////////////////////////////////////////////////////////////////////////////////////
// Colour

typedef struct PLColour {
    uint8_t r, g, b, a;

#ifdef __cplusplus
    PLColour() : PLColour(255, 255, 255, 255) {

    }

    PLColour(uint8_t c, uint8_t c2, uint8_t c3, uint8_t c4 = 255) : r(c), g(c2), b(c3), a(c4) {

    }

    PLColour(int c, int c2, int c3, int c4 = 255) :
            PLColour((uint8_t) c, (uint8_t) c2, (uint8_t) c3, (uint8_t) c4) {

    }

    PLColour(float c, float c2, float c3, float c4 = 1) :
    r(plFloatToByte(c)),
    g(plFloatToByte(c2)),
    b(plFloatToByte(c3)),
    a(plFloatToByte(c4)) {

    }

    PL_INLINE PLVector4 ToVec4() const {
        return {plByteToFloat(r), plByteToFloat(g), plByteToFloat(b), plByteToFloat(a)};
    }

    PL_INLINE void operator *= (const PLColour &v) {
        r *= v.r; g *= v.g; b *= v.b; a *= v.a;
    }

    PL_INLINE void operator *= (float c) {
        r *= plFloatToByte(c); g *= plFloatToByte(c); b *= plFloatToByte(c); a *= plFloatToByte(c);
    }

    PL_INLINE void operator *= (uint8_t c) {
        r *= c; g *= c; g *= c; a *= c;
    }

    PL_INLINE void operator += (const PLColour &v) {
        r += v.r; g += v.g; b += v.b; a += v.a;
    }

    PL_INLINE void operator += (float c) {
        r += plFloatToByte(c); g += plFloatToByte(c); b += plFloatToByte(c); a += plFloatToByte(c);
    }

    PL_INLINE void operator -= (const PLColour &v) {
        r -= v.r; g -= v.g; b -= v.b; a -= v.a;
    }

    PL_INLINE void operator -= (float c) {
        r -= plFloatToByte(c); g -= plFloatToByte(c); b -= plFloatToByte(c); a -= plFloatToByte(c);
    }

    PL_INLINE void operator /= (const PLColour &v) {
        r /= v.r; g /= v.g; b /= v.b; a /= v.a;
    }

    PL_INLINE void operator /= (float c) {
        r /= c; g /= c; b /= c; a /= c;
    }

    PL_INLINE PLColour operator - (const PLColour &c) const {
        return {r - c.r, g - c.g, b - c.b, a - c.a};
    }

    PL_INLINE PLColour operator - (float c) const {
        return {r - plFloatToByte(c), g - plFloatToByte(c), b - plFloatToByte(c), a - plFloatToByte(c)};
    }

    PL_INLINE PLColour operator - (uint8_t c) const {
        return {r - c, g - c, b - c, a - c};
    }

    PL_INLINE PLColour operator - () const {
        return PLColour(-r, -g, -b, -a);
    }

    PL_INLINE PLColour operator * (const PLColour &v) const {
        return PLColour(r * v.r, g * v.g, b * v.b, a * v.a);
    }

    PL_INLINE PLColour operator + (const PLColour &v) const {
        return PLColour(r + v.r, g + v.g, b + v.b, a + v.a);
    }

    PL_INLINE PLColour operator + (float c) const {
        return {r + plFloatToByte(c), g + plFloatToByte(c), b + plFloatToByte(c), a + plFloatToByte(c)};
    }

    PL_INLINE PLColour operator / (const PLColour &v) const {
        return {r / v.r, g / v.g, b / v.b, a / v.a};
    }

    PL_INLINE PLColour operator / (float c) const {
        return PLColour(r / plFloatToByte(c), g / plFloatToByte(c), b / plFloatToByte(c), a / plFloatToByte(c));
    }

    PL_INLINE PLColour operator / (uint8_t c) const {
        return PLColour(r / c, g / c, b / c, a / c);
    }

    PL_INLINE uint8_t& operator [] (const unsigned int i) {
        return *((&r) + i);
    }

    PL_INLINE bool operator > (const PLColour &v) const {
        return ((r > v.r) && (g > v.g) && (b > v.b) && (a > v.a));
    }

    PL_INLINE bool operator < (const PLColour &v) const {
        return ((r < v.r) && (g < v.g) && (b < v.b) && (a < v.a));
    }

    PL_INLINE bool operator >= (const PLColour &v) const {
        return ((r >= v.r) && (g >= v.g) && (b >= v.b) && (a >= v.a));
    }

    PL_INLINE bool operator <= (const PLColour &v) const {
        return ((r <= v.r) && (g <= v.g) && (b <= v.b) && (a <= v.a));
    }
#endif
} PLColour;

PL_INLINE static PLColour plCreateColour4b(uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    PLColour c = { r, g, b, a };
    return c;
}

PL_INLINE static PLColour plCreateColour4f(float r, float g, float b, float a) {
    PLColour c = {
        plFloatToByte(r),
        plFloatToByte(g),
        plFloatToByte(b),
        plFloatToByte(a)
    };
    return c;
}

PL_INLINE static void plSetColour4b(PLColour *c, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    c->r = r; c->g = g; c->b = b; c->a = a;
}

PL_INLINE static void plSetColour4f(PLColour *c, float r, float g, float b, float a) {
    c->r = plFloatToByte(r);
    c->g = plFloatToByte(g);
    c->b = plFloatToByte(b);
    c->a = plFloatToByte(a);
}

PL_INLINE static void plClearColour(PLColour *c) {
    plSetColour4b(c, 0, 0, 0, 0);
}

PL_INLINE static bool plCompareColour(PLColour c, PLColour c2) {
    return ((c.r == c2.r) && (c.g == c2.g) && (c.b == c2.b) && (c.a == c2.a));
}

PL_INLINE static void plCopyColour(PLColour *c, PLColour c2) {
    c->r = c2.r; c->g = c2.g; c->b = c2.b; c->a = c2.a;
}

PL_INLINE static void plMultiplyColour(PLColour *c, PLColour c2) {
    c->r *= c2.r; c->g *= c2.g; c->b *= c2.b; c->a *= c2.a;
}

PL_INLINE static void plMultiplyColourf(PLColour *c, float a) {
    uint8_t a2 = plFloatToByte(a);
    c->r *= a2; c->g *= a2; c->b *= a2; c->a *= a2;
}

PL_INLINE static void plDivideColour(PLColour *c, PLColour c2) {
    c->r /= c2.r; c->g /= c2.g; c->b /= c2.b; c->a /= c2.a;
}

PL_INLINE static void plDivideColourf(PLColour *c, float a) {
    uint8_t a2 = plFloatToByte(a);
    c->r /= a2; c->g /= a2; c->b /= a2; c->a /= a2;
}

PL_INLINE static const char *plPrintColour(PLColour c) {
    static char s[16] = { '\0' };
    snprintf(s, 16, "%i %i %i %i", c.r, c.g, c.b, c.a);
    return s;
}

#ifndef __cplusplus

#   define PLColour(r, g, b, a) (PLColour){r  , g  , b  , a  }

#endif

#define PLColourRGB(r, g, b) (PLColour){r  , g  , b  , 255}
#define PLColourR(r)         (PLColour){r  , 255, 255, 255}
#define PLColourG(g)         (PLColour){255, g  , 255, 255}
#define PLColourB(b)         (PLColour){255, 255, b  , 255}
#define PLColourA(a)         (PLColour){255, 255, 255, a  }

/* pinks */
#define PL_COLOUR_PINK                      PLColourRGB(255, 192, 203)
#define PL_COLOUR_LIGHT_PINK                PLColourRGB(255, 182, 193)
#define PL_COLOUR_HOT_PINK                  PLColourRGB(255, 105, 180)
#define PL_COLOUR_DEEP_PINK                 PLColourRGB(255, 20 , 147)
#define PL_COLOUR_PALE_VIOLET_RED           PLColourRGB(219, 112, 147)
#define PL_COLOUR_MEDIUM_VIOLET_RED         PLColourRGB(199, 21 , 133)
/* reds */
#define PL_COLOUR_LIGHT_SALMON              PLColourRGB(255, 160, 122)
#define PL_COLOUR_SALMON                    PLColourRGB(250, 128, 114)
#define PL_COLOUR_DARK_SALMON               PLColourRGB(233, 150, 122)
#define PL_COLOUR_LIGHT_CORAL               PLColourRGB(240, 128, 128)
#define PL_COLOUR_INDIAN_RED                PLColourRGB(205, 92 , 92 )
#define PL_COLOUR_CRIMSON                   PLColourRGB(220, 20 , 60 )
#define PL_COLOUR_FIRE_BRICK                PLColourRGB(178, 34 , 34 )
#define PL_COLOUR_DARK_RED                  PLColourRGB(139, 0  , 0  )
#define PL_COLOUR_RED                       PLColourRGB(255, 0  , 0  )
/* oranges */
#define PL_COLOUR_ORANGE_RED                PLColourRGB(255, 69 , 0  )
#define PL_COLOUR_TOMATO                    PLColourRGB(255, 99 , 71 )
#define PL_COLOUR_CORAL                     PLColourRGB(255, 127, 80 )
#define PL_COLOUR_DARK_ORANGE               PLColourRGB(255, 140, 0  )
#define PL_COLOUR_ORANGE                    PLColourRGB(255, 165, 0  )
/* yellows */
#define PL_COLOUR_YELLOW                    PLColourRGB(255, 255, 0  )
#define PL_COLOUR_LIGHT_YELLOW              PLColourRGB(255, 255, 224)
#define PL_COLOUR_LEMON_CHIFFON             PLColourRGB(255, 250, 205)
#define PL_COLOUR_LIGHT_GOLDENROD_YELLOW    PLColourRGB(250, 250, 210)
#define PL_COLOUR_PAPAYA_WHIP               PLColourRGB(255, 239, 213)
#define PL_COLOUR_MOCCASIN                  PLColourRGB(255, 228, 181)
#define PL_COLOUR_PEACH_PUFF                PLColourRGB(255, 218, 185)
#define PL_COLOUR_PALE_GOLDENROD            PLColourRGB(238, 232, 170)
#define PL_COLOUR_KHAKI                     PLColourRGB(240, 230, 140)
#define PL_COLOUR_DARK_KHAKI                PLColourRGB(189, 183, 107)
#define PL_COLOUR_GOLD                      PLColourRGB(255, 215, 0  )
/* browns */
#define PL_COLOUR_CORNSILK                  PLColourRGB(255, 248, 220)
#define PL_COLOUR_BLANCHED_ALMOND           PLColourRGB(255, 235, 205)
#define PL_COLOUR_BISQUE                    PLColourRGB(255, 228, 196)
#define PL_COLOUR_NAVAJO_WHITE              PLColourRGB(255, 222, 173)
#define PL_COLOUR_WHEAT                     PLColourRGB(245, 222, 179)
#define PL_COLOUR_BURLY_WOOD                PLColourRGB(222, 184, 135)
#define PL_COLOUR_TAN                       PLColourRGB(210, 180, 140)
#define PL_COLOUR_ROSY_BROWN                PLColourRGB(188, 143, 143)
#define PL_COLOUR_SANDY_BROWN               PLColourRGB(244, 164, 96 )
#define PL_COLOUR_GOLDENROD                 PLColourRGB(218, 165, 32 )
#define PL_COLOUR_DARK_GOLDENROD            PLColourRGB(184, 134, 11 )
#define PL_COLOUR_PERU                      PLColourRGB(205, 133, 63 )
#define PL_COLOUR_CHOCOLATE                 PLColourRGB(210, 105, 30 )
#define PL_COLOUR_SADDLE_BROWN              PLColourRGB(139, 69 , 19 )
#define PL_COLOUR_SIENNA                    PLColourRGB(160, 82 , 45 )
#define PL_COLOUR_BROWN                     PLColourRGB(165, 42 , 42 )
#define PL_COLOUR_MAROON                    PLColourRGB(128, 0  , 0  )
/* greens */
#define PL_COLOUR_DARK_OLIVE_GREEN          PLColourRGB(85 , 107, 47 )
#define PL_COLOUR_OLIVE                     PLColourRGB(128, 128, 0  )
#define PL_COLOUR_OLIVE_DRAB                PLColourRGB(107, 142, 35 )
#define PL_COLOUR_YELLOW_GREEN              PLColourRGB(154, 205, 50 )
#define PL_COLOUR_LIME_GREEN                PLColourRGB(50 , 205, 50 )
#define PL_COLOUR_LIME                      PLColourRGB(0  , 255, 0  )
#define PL_COLOUR_LAWN_GREEN                PLColourRGB(124, 252, 0  )
#define PL_COLOUR_CHARTREUSE                PLColourRGB(127, 255, 0  )
#define PL_COLOUR_GREEN_YELLOW              PLColourRGB(173, 255, 47 )
#define PL_COLOUR_SPRING_GREEN              PLColourRGB(0  , 255, 127)
#define PL_COLOUR_MEDIUM_SPRING_GREEN       PLColourRGB(0  , 250, 154)
#define PL_COLOUR_LIGHT_GREEN               PLColourRGB(144, 238, 144)
#define PL_COLOUR_PALE_GREEN                PLColourRGB(152, 251, 152)
#define PL_COLOUR_DARK_SEA_GREEN            PLColourRGB(143, 188, 143)
#define PL_COLOUR_MEDIUM_AQUAMARINE         PLColourRGB(102, 205, 170)
#define PL_COLOUR_MEDIUM_SEA_GREEN          PLColourRGB(60 , 179, 113)
#define PL_COLOUR_SEA_GREEN                 PLColourRGB(46 , 139, 87 )
#define PL_COLOUR_FOREST_GREEN              PLColourRGB(34 , 139, 34 )
#define PL_COLOUR_GREEN                     PLColourRGB(0  , 128, 0  )
#define PL_COLOUR_DARK_GREEN                PLColourRGB(0  , 100, 0  )
/* cyans */
#define PL_COLOUR_AQUA                      PLColourRGB(0  , 255, 255)
#define PL_COLOUR_CYAN                      PL_COLOUR_AQUA
#define PL_COLOUR_LIGHT_CYAN                PLColourRGB(224, 255, 255)
#define PL_COLOUR_PALE_TURQUOISE            PLColourRGB(175, 238, 238)
#define PL_COLOUR_AQUAMARINE                PLColourRGB(127, 255, 212)
#define PL_COLOUR_TURQUOISE                 PLColourRGB(64 , 224, 208)
#define PL_COLOUR_MEDIUM_TURQUOISE          PLColourRGB(72 , 209, 204)
#define PL_COLOUR_DARK_TURQUOISE            PLColourRGB(0  , 206, 209)
#define PL_COLOUR_LIGHT_SEA_GREEN           PLColourRGB(32 , 178, 170)
#define PL_COLOUR_CADET_BLUE                PLColourRGB(95 , 158, 160)
#define PL_COLOUR_DARK_CYAN                 PLColourRGB(0  , 139, 139)
#define PL_COLOUR_TEAL                      PLColourRGB(0  , 128, 128)
/* blues */
#define PL_COLOUR_LIGHT_STEEL_BLUE          PLColourRGB(176, 196, 222)
#define PL_COLOUR_POWDER_BLUE               PLColourRGB(176, 224, 230)
#define PL_COLOUR_LIGHT_BLUE                PLColourRGB(173, 216, 230)
#define PL_COLOUR_SKY_BLUE                  PLColourRGB(135, 206, 235)
#define PL_COLOUR_LIGHT_SKY_BLUE            PLColourRGB(135, 206, 250)
#define PL_COLOUR_DEEP_SKY_BLUE             PLColourRGB(0  , 191, 255)
#define PL_COLOUR_DODGER_BLUE               PLColourRGB(30 , 144, 255)
#define PL_COLOUR_CORNFLOWER_BLUE           PLColourRGB(100, 149, 237)
#define PL_COLOUR_STEEL_BLUE                PLColourRGB(70 , 130, 180)
#define PL_COLOUR_ROYAL_BLUE                PLColourRGB(65 , 105, 225)
#define PL_COLOUR_BLUE                      PLColourRGB(0  , 0  , 255)
#define PL_COLOUR_MEDIUM_BLUE               PLColourRGB(0  , 0  , 205)
#define PL_COLOUR_DARK_BLUE                 PLColourRGB(0  , 0  , 139)
#define PL_COLOUR_NAVY                      PLColourRGB(0  , 0  , 128)
#define PL_COLOUR_MIDNIGHT_BLUE             PLColourRGB(25 , 25 , 112)
/* purples */
#define PL_COLOUR_LAVENDER                  PLColourRGB(230, 230, 250)
#define PL_COLOUR_THISTLE                   PLColourRGB(216, 191, 216)
#define PL_COLOUR_PLUM                      PLColourRGB(221, 160, 221)
#define PL_COLOUR_VIOLET                    PLColourRGB(238, 130, 238)
#define PL_COLOUR_ORCHID                    PLColourRGB(218, 112, 214)
#define PL_COLOUR_FUCHSIA                   PLColourRGB(255, 0  , 255)
#define PL_COLOUR_MAGENTA                   PL_COLOUR_FUCHSIA
#define PL_COLOUR_MEDIUM_ORCHID             PLColourRGB(186, 85 , 211)
#define PL_COLOUR_MEDIUM_PURPLE             PLColourRGB(147, 112, 219)
#define PL_COLOUR_BLUE_VIOLET               PLColourRGB(138, 42 , 226)
#define PL_COLOUR_DARK_VIOLET               PLColourRGB(148, 0  , 211)
#define PL_COLOUR_DARK_ORCHID               PLColourRGB(153, 50 , 204)
#define PL_COLOUR_DARK_MAGNENTA             PLColourRGB(139, 0  , 139)
#define PL_COLOUR_PURPLE                    PLColourRGB(128, 0  , 128)
#define PL_COLOUR_INDIGO                    PLColourRGB(75 , 0  , 130)
#define PL_COLOUR_DARK_SLATE_BLUE           PLColourRGB(72 , 61 , 139)
#define PL_COLOUR_SLATE_BLUE                PLColourRGB(106, 90 , 205)
#define PL_COLOUR_MEDIUM_SLATE_BLUE         PLColourRGB(123, 104, 238)
/* whites */
#define PL_COLOUR_WHITE                     PLColourRGB(255, 255, 255)
#define PL_COLOUR_SNOW                      PLColourRGB(255, 250, 250)
#define PL_COLOUR_HONEYDEW                  PLColourRGB(240, 255, 240)
#define PL_COLOUR_MINT_CREAM                PLColourRGB(245, 255, 250)
#define PL_COLOUR_AZURE                     PLColourRGB(240, 255, 255)
#define PL_COLOUR_ALICE_BLUE                PLColourRGB(240, 248, 255)
#define PL_COLOUR_GHOST_WHITE               PLColourRGB(248, 248, 255)
#define PL_COLOUR_WHITE_SMOKE               PLColourRGB(245, 245, 245)
#define PL_COLOUR_SEASHELL                  PLColourRGB(255, 245, 238)
#define PL_COLOUR_BEIGE                     PLColourRGB(245, 245, 220)
#define PL_COLOUR_OLD_LACE                  PLColourRGB(253, 245, 230)
#define PL_COLOUR_FLORAL_WHITE              PLColourRGB(255, 250, 240)
#define PL_COLOUR_IVORY                     PLColourRGB(255, 255, 240)
#define PL_COLOUR_ANTIQUE_WHITE             PLColourRGB(250, 235, 215)
#define PL_COLOUR_LINEN                     PLColourRGB(250, 240, 230)
#define PL_COLOUR_LAVENDER_BLUSH            PLColourRGB(255, 240, 245)
#define PL_COLOUR_MISTY_ROSE                PLColourRGB(255, 228, 225)
/* blacks */
#define PL_COLOUR_GAINSBORO                 PLColourRGB(220, 220, 220)
#define PL_COLOUR_LIGHT_GRAY                PLColourRGB(211, 211, 211)
#define PL_COLOUR_SILVER                    PLColourRGB(192, 192, 192)
#define PL_COLOUR_DARK_GRAY                 PLColourRGB(169, 169, 169)
#define PL_COLOUR_GRAY                      PLColourRGB(128, 128, 128)
#define PL_COLOUR_DIM_GRAY                  PLColourRGB(105, 105, 105)
#define PL_COLOUR_LIGHT_SLATE_GRAY          PLColourRGB(119, 135, 153)
#define PL_COLOUR_SLATE_GRAY                PLColourRGB(112, 128, 144)
#define PL_COLOUR_DARK_SLATE_GRAY           PLColourRGB(47 , 79 , 79 )
#define PL_COLOUR_BLACK                     PLColourRGB(0  , 0  , 0  )

// Quaternion

typedef struct PLQuaternion {
    float x, y, z, w;

#ifdef __cplusplus
    PLQuaternion(float a, float b, float c, float d) : x(a), y(b), z(c), w(d) {}
    PLQuaternion(float a, float b, float c) : x(a), y(b), z(c), w(0) {}
    PLQuaternion() : x(0), y(0), z(0), w(0) {}

    PL_INLINE void operator *= (float a) {
        x *= a; y *= a; z *= a; w *= a;
    }

    PL_INLINE void operator *= (PLQuaternion a) {
        x *= a.x;
        y *= a.y;
        z *= a.z;
        w *= a.w;
    }

    PL_INLINE bool operator == (PLQuaternion a) const {
        return ((x == a.x) && (y == a.y) && (z == a.z) && (w == a.w));
    }

    PL_INLINE PLQuaternion operator * (float a) {
        return {x * a, y * a, z * a, w * a};
    }

    PL_INLINE PLQuaternion operator * (PLQuaternion a) {
        return {x * a.x, y * a.y, z * a.z, w * a.w};
    }

    PL_INLINE void Set(float a, float b, float c, float d) {
        x = a; y = b; z = c; w = d;
    }

    PL_INLINE void Set(float a, float b, float c) {
        x = a;
        y = b;
        z = c;
    }

    PL_INLINE const char *String() {
        static char s[32] = { 0 };
        snprintf(s, 32, "%i %i %i %i", (int) x, (int) y, (int) z, (int) w);
        return s;
    }

    PL_INLINE float Length() {
        return std::sqrt((x * x + y * y + z * z + w * w));
    }

    PL_INLINE PLQuaternion Scale(float a) {
        return {x * a, y * a, z * a, w * a};
    }

    PL_INLINE PLQuaternion Inverse() {
        return {-x, -y, -z, w};
    }

#if 0
    PL_INLINE PLQuaternion Normalize() {
        float l = Length();
        if (l > 0) {
            float i = 1 / l;
            return Scale(i);
        }
    }
#endif
#endif
} PLQuaternion;

PL_INLINE static void plClearQuaternion(PLQuaternion *q) {
    q->x = q->y = q->z = q->w = 0;
}

PL_INLINE static void plMultiplyQuaternion(PLQuaternion *q, PLQuaternion q2) {
    q->x *= q2.x;
    q->y *= q2.y;
    q->z *= q2.z;
    q->w *= q2.w;
}

PL_INLINE static void plScaleQuaternion(PLQuaternion *q, float a) {
    q->x *= a;
    q->y *= a;
    q->z *= a;
    q->w *= a;
}

PL_INLINE static void plAddQuaternion(PLQuaternion *q, PLQuaternion q2) {
    q->x += q2.x;
    q->y += q2.y;
    q->z += q2.z;
    q->w += q2.w;
}

PL_INLINE static void plAddQuaternionf(PLQuaternion *q, float a) {
    q->x += a;
    q->y += a;
    q->z += a;
    q->w += a;
}

PL_INLINE static void plInverseQuaternion(PLQuaternion *q) {
    q->x = -q->x;
    q->y = -q->y;
    q->z = -q->z;
    q->w = -q->w;
}

PL_INLINE static float plQuaternionLength(const PLQuaternion *q) {
    return sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
}

PL_INLINE static void plNormalizeQuaternion(PLQuaternion *q) {
    float l = plQuaternionLength(q);
    if(l > 0) {
        float i = 1 / l;
        plScaleQuaternion(q, i);
    }
}

PL_INLINE static const char *plPrintQuaternion(PLQuaternion q) {
    static char s[32] = { '\0' };
    snprintf(s, 32, "%i %i %i %i", (int)q.x, (int)q.y, (int)q.z, (int)q.w);
    return s;
}

PL_INLINE static void plComputeQuaternionW(PLQuaternion *q) {
    float t = 1.f - (q->x * q->x) - (q->y * q->y) - (q->z * q->z);
    if(t < 0) {
        q->w = 0;
    } else {
        q->w = -sqrtf(t);
    }
}

/////////////////////////////////////////////////////////////////////////////////////
// Primitives

// Cube

// Sphere

typedef struct PLSphere {
    PLVector3 position;
    float radius;

    PLColour colour;
} PLSphere;

// Rectangle 2D

typedef struct PLRectangle2D {
    PLVector2 xy;
    PLVector2 wh;

    PLColour ul, ur, ll, lr;
} PLRectangle2D;

PL_INLINE static PLRectangle2D plCreateRectangle(
        PLVector2 xy, PLVector2 wh,
        PLColour ul, PLColour ur,
        PLColour ll, PLColour lr
) {
    return (PLRectangle2D){
            xy, wh,
            ul, ur,
            ll, lr
    };
}

PL_INLINE static void plClearRectangle(PLRectangle2D *r) {
    r->xy = r->wh = (PLVector2){0, 0};
    r->ul = r->ur = r->ll = r->lr = (PLColour){0, 0, 0, 0};
}

PL_INLINE static void plSetRectangleUniformColour(PLRectangle2D *r, PLColour colour) {
    r->ll = r->lr = r->ul = r->ur = colour;
}

/////////////////////////////////////////////////////////////////////////////////////
// Randomisation

// http://stackoverflow.com/questions/7978759/generate-float-random-values-also-negative
PL_INLINE static double plUniform0To1Random(void) {
    return (rand()) / ((double) RAND_MAX + 1);
}

PL_INLINE static double plGenerateUniformRandom(double minmax) {
    return (minmax * 2) * plUniform0To1Random() - minmax;
}

PL_INLINE static double plGenerateRandomd(double max) {
    return (double) (rand()) / (RAND_MAX / max);
}

PL_INLINE static float plGenerateRandomf(float max) {
    return (float) (rand()) / (RAND_MAX / max);
}

/////////////////////////////////////////////////////////////////////////////////////
// Interpolation
// http://paulbourke.net/miscellaneous/interpolation/

PL_INLINE static float plLinearInterpolate(float y1, float y2, float mu) {
    return (y1 * (1 - mu) + y2 * mu);
}

PL_INLINE static float plCosineInterpolate(float y1, float y2, float mu) {
    float mu2 = (1 - cosf(mu * (float) PL_PI)) / 2;
    return (y1 * (1 - mu2) + y2 * mu2);
}

// http://probesys.blogspot.co.uk/2011/10/useful-math-functions.html

PL_INLINE static float plOutPow(float x, float p) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    int sign = (int) p % 2 == 0 ? -1 : 1;
    return (sign * (powf(x - 1.0f, p) + sign));
}

PL_INLINE static float plLinear(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }
    return x;
}

PL_INLINE static float plInPow(float x, float p) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }
    return powf(x, p);
}

PL_INLINE static float plInSin(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }
    return -cosf(x * ((float) PL_PI / 2.0f)) + 1.0f;
}

PL_INLINE static float plOutSin(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }
    return sinf(x * ((float) PL_PI / 2.0f));
}

PL_INLINE static float plInExp(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    return powf(2.0f, 10.0f * (x - 1.0f));
}

PL_INLINE static float plOutExp(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }
    return -powf(2.0f, -1.0f * x) + 1.0f;
}

PL_INLINE static float plInOutExp(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }
    return x < 0.5f ? 0.5f * powf(2.0f, 10.0f * (2.0f * x - 1.0f)) :
           0.5f * (-powf(2.0f, 10.0f * (-2.0f * x + 1.0f)) + 1.0f);
}

PL_INLINE static float plInCirc(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }
    return -(sqrtf(1.0f - x * x) - 1.0f);
}

PL_INLINE static float plOutBack(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }
    return (x - 1.0f) * (x - 1.0f) * ((1.70158f + 1.0f) * (x - 1.0f) + 1.70158f) + 1.0f;
}

// The variable, k, controls the stretching of the function.
PL_INLINE static float plImpulse(float x, float k) {
    float h = k * x;
    return h * expf(1.0f - h);
}

PL_INLINE static float plRebound(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    if (x < (1.0f / 2.75f)) {
        return 1.0f - 7.5625f * x * x;
    } else if (x < (2.0f / 2.75f)) {
        return 1.0f - (7.5625f * (x - 1.5f / 2.75f) *
                       (x - 1.5f / 2.75f) + 0.75f);
    } else if (x < (2.5f / 2.75f)) {
        return 1.0f - (7.5625f * (x - 2.25f / 2.75f) *
                       (x - 2.25f / 2.75f) + 0.9375f);
    } else {
        return 1.0f - (7.5625f * (x - 2.625f / 2.75f) * (x - 2.625f / 2.75f) +
                       0.984375f);
    }
}

PL_INLINE static float plExpPulse(float x, float k, float n) {
    return expf(-k * powf(x, n));
}

PL_INLINE static float plInOutBack(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    return x < 0.5f ? 0.5f * (4.0f * x * x * ((2.5949f + 1.0f) * 2.0f * x - 2.5949f)) :
           0.5f * ((2.0f * x - 2.0f) * (2.0f * x - 2.0f) * ((2.5949f + 1.0f) * (2.0f * x - 2.0f) +
                                                            2.5949f) + 2.0f);
}

PL_INLINE static float plInBack(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    return x * x * ((1.70158f + 1.0f) * x - 1.70158f);
}

PL_INLINE static float plInOutCirc(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    return x < 1.0f ? -0.5f * (sqrtf(1.0f - x * x) - 1.0f) :
           0.5f * (sqrtf(1.0f - ((1.0f * x) - 2.0f) * ((2.0f * x) - 2.0f)) + 1.0f);
}

PL_INLINE static float plOutCirc(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    return sqrtf(1.0f - (x - 1.0f) * (x - 1.0f));
}

PL_INLINE static float plInOutSin(float x) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    return -0.5f * (cosf((float) PL_PI * x) - 1.0f);
}

PL_INLINE static float plInOutPow(float x, float p) {
    if (x < 0) {
        return 0;
    } else if (x > 1.0f) {
        return 1.0f;
    }

    int sign = (int) p % 2 == 0 ? -1 : 1;
    return (sign / 2.0f * (powf(x - 2.0f, p) + sign * 2.0f));
}

//////////////////////////////////////////////////////////////////////
// UTILITY FUNCTIONS

PL_INLINE static float plToRadians(float degrees) {
    return degrees * (PL_PI / 180.f);
}

/* http://www.songho.ca/opengl/gl_anglestoaxes.html */
PL_INLINE static void plAnglesAxes(PLVector3 angles, PLVector3 *left, PLVector3 *up, PLVector3 *forward) {
    /* pitch */
    float theta = angles.x * PL_PI_DIV_180;
    float sx = sinf(theta);
    float cx = cosf(theta);

    /* yaw */
    theta = angles.y * PL_PI_DIV_180;
    float sy = sinf(theta);
    float cy = cosf(theta);

    /* roll */
    theta = angles.z * PL_PI_DIV_180;
    float sz = sinf(theta);
    float cz = cosf(theta);

    left->x = cy * cz;
    left->y = sx * sy * cz + cx * sz;
    left->z = -cx * sy * cz + sx * sz;

    up->x = -cy * sz;
    up->y = -sx * sy * sz + cx * cz;
    up->z = cx * sy * sz + sx * cz;

    forward->x = sy;
    forward->y = -sx * cy;
    forward->z = cx * cy;
}

#include "pl_math_matrix.h"
