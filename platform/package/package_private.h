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

#include <PL/platform_filesystem.h>
#include <PL/platform_package.h>

#define PLPACKAGE_VERSION_MAJOR     1
#define PLPACKAGE_VERSION_MINOR     0

enum {
    PLPACKAGE_LEVEL_INDEX,
    PLPACKAGE_MODEL_INDEX,
    PLPACKAGE_TEXTURE_INDEX,
    PLPACKAGE_MATERIAL_INDEX,

    PLPACKAGE_VERTICES_INDEX,
    PLPACKAGE_TRIANGLES_INDEX,

    // Level
    PLPACKAGE_LEVEL_SECTORS_INDEX, // aka 'rooms', links in with triangles?
    PLPACKAGE_LEVEL_PORTALS_INDEX, // aka 'doors', links in with triangles and sectors?
    PLPACKAGE_LEVEL_OBJECTS_INDEX, // list of objects within the level

    // Model

    // Texture

    PLPACKAGE_UNKNOWN_INDEX,

    PLPACKAGE_LAST_INDEX
};

typedef struct PLPackageHeader { // (PACKAGE)
    uint8_t     identity[4];    // Descriptor/name of the data type. "PACK"
    uint8_t     version[2];     // Version of this type.

    uint32_t    num_indexes;    // Number of data indexes (each _should_ be a fixed size?)

    // followed by num_indexes + length
    // then followed by rest of data
} PLPackageHeader;

typedef struct PLPackageIndexHeader {
    uint16_t type;
    uint32_t length;

    // followed by type-specific index information
} PLPackageIndexHeader;

PL_EXTERN_C

PL_INLINE static void WritePackageHeader(FILE *handle, uint32_t num_indexes) {
    plAssert(handle);
    uint8_t identity [4]= { 'P', 'A', 'C', 'K' };
    fwrite(identity, sizeof(char), sizeof(identity), handle);
    uint8_t version [2]= {
            PLPACKAGE_VERSION_MAJOR,
            PLPACKAGE_VERSION_MINOR
    };
    fwrite(version, sizeof(uint8_t), sizeof(version), handle);
    fwrite(&num_indexes, sizeof(uint32_t), 1, handle);
}

PL_INLINE static void WritePackageIndexHeader(FILE *handle, uint16_t type, uint32_t length) {
    plAssert(handle);
    fwrite(&type, sizeof(uint16_t), 1, handle);
    fwrite(&length, sizeof(uint32_t), 1, handle);
}

/////////////////////////////////////////////////////////////////

PLPackage *LoadMADPackage(const char *filename, bool cache);
bool LoadMADPackageFile(FILE *fh, PLPackageIndex *pi);

PLPackage *LoadARTPackage(const char *filename, bool precache);
bool LoadARTPackageFile(FILE *fh, PLPackageIndex *pi);

PL_EXTERN_C_END

/////////////////////////////////////////////////////////////////

/*  Platform Level Format   */

typedef struct {
} PLLevelIndex;

typedef struct {
    uint32_t num_triangles;
    uint32_t num_portals;
} PLLevelSector;