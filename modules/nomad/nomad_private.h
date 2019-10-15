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

#include "nomad.h"

/* Private API */

typedef char Od3String[20];

typedef struct Od3Header {
    char            identifier[4];
    unsigned int    version[2];
    unsigned int    offset_materials;
    unsigned int    offset_vertices;
    unsigned int    offset_triangles;
    unsigned int    offset_quads;
    unsigned int    offset_meshes;
    unsigned int    offset_doors;
    unsigned int    offset_cameras;
    unsigned int    offset_lights;
    char            u0[180];
    
} Od3Header;

/************************************/
/* 3DO Mesh */

typedef struct Od3Mesh {
    unsigned char   flags[4];
    unsigned int    mover_flags;
    unsigned int    id;
    unsigned int    ed_id;
    Od3String       name;
    PLVector3       position;
    unsigned int    parent;
    unsigned int    child;
    unsigned int    next_child;
    unsigned int    u0;
    unsigned int    num_vertices;
    unsigned int    num_triangles;
    unsigned int    num_quads;

} Od3Mesh;

/************************************/
/* 3DO Light */

typedef struct Od3Light {
    unsigned short  flag0;
    unsigned short  flag1;
    Od3String       name;
    float           u0;
    float           u1;
    float           intensity;
    float           u2;
    float           u3;
    PLColour        colour;
    PLVector3       points[6];
} Od3Light;

Od3Light* Od3_CreateLight(void);
void Od3_DestroyLight(Od3Light* ptr);

void Od3_SerializeLight(Od3Light* ptr, FILE* fp);
void Od3_DeserializeLight(Od3Light* ptr, FILE* fp);

inline static void Od3_SetLightName(Od3Light* ptr, const char* name) { strncpy(ptr->name, name, sizeof(ptr->name)); }
inline static void Od3_SetLightIntensity(Od3Light* ptr, float intensity) { ptr->intensity = intensity; }
inline static void Od3_SetLightColour(Od3Light* ptr, const PLColour* colour) { ptr->colour = *colour; }

/************************************/
/* 3DO Camera */

typedef struct Od3Camera {
    Od3String   name;
    PLVector3   position;
    PLVector3   target;
    float       u0;
    float       fov;
} Od3Camera;

Od3Camera* Od3_CreateCamera(
    const char* name, 
    const PLVector3* position, 
    const PLVector3* target,
    float u0,
    float fov);
void Od3_DestroyCamera(Od3Camera* ptr);

void Od3_SerializeCamera(Od3Camera* ptr, FILE* fp);
void Od3_DeserializeCamera(Od3Camera* ptr, FILE* fp);

inline static void Od3_SetCameraName(Od3Camera* ptr, const char* name) { strncpy(ptr->name, name, sizeof(ptr->name)); }
inline static void Od3_SetCameraPosition(Od3Camera* ptr, const PLVector3* position) { ptr->position = *position; }
inline static void Od3_SetCameraTarget(Od3Camera* ptr, const PLVector3* target) { ptr->target = *target; }
inline static void Od3_SetCameraFov(Od3Camera* ptr, float fov) { ptr->fov = fov; }

/************************************/
/* 3DO Door */

typedef struct Od3Door {
    Od3String       name;
    unsigned int    u0;     /* link? */
    unsigned int    u1;     /* link? */
} Od3Door;

Od3Door* Od3_CreateDoor(unsigned int u0, unsigned int u1);
void Od3_DestroyDoor(Od3Door* ptr);
