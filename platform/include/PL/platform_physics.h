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

#include <PL/platform.h>
#include <PL/platform_math.h>

typedef struct PLAABB {
    PLVector3 mins, maxs;
} PLAABB;

PL_INLINE static void plAddAABB(PLAABB *b, PLAABB b2) {
    plAddVector3(&b->maxs, b2.maxs);
    plAddVector3(&b->mins, b2.mins);
}

PL_INLINE static bool plIntersectAABB(PLAABB b, PLAABB b2) {
#if 0 /* what the fuck is this crap? */
    PLVector3 dist_a = b2.mins;
    plSubtractVector3(&dist_a, b.maxs);
    PLVector3 dist_b = b.mins;
    plSubtractVector3(&dist_b, b2.maxs);
    PLVector3 dist = plVector3Max(dist_a, dist_b);
#else
    if(
            b.maxs.x < b2.mins.x ||
            b.maxs.y < b2.mins.y ||
            b.maxs.z < b2.mins.z ||

            b.mins.x > b2.maxs.x ||
            b.mins.y > b2.maxs.y ||
            b.mins.z > b2.maxs.z
            ) {
        return false;
    }
#endif

    return true;
}

PL_INLINE static bool plIntersectPoint(PLAABB b, PLVector3 point) {
    if(
            point.x > b.maxs.x ||
            point.x < b.mins.x ||

            point.y > b.maxs.y ||
            point.y < b.mins.y ||

            point.z > b.maxs.z ||
            point.z < b.mins.z
            ) {
        return false;
    }

    return true;
}

PL_INLINE static bool plIsSphereIntersecting(PLVector3 origin, float radius, PLVector3 position_b, float radius_b) {
    PLVector3 difference = origin;
    plSubtractVector3(&difference, position_b);
    float distance = plVector3Length(difference);
    float sum_radius = radius + radius_b;
    return distance < sum_radius;
}