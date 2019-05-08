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

#include <PL/platform_package.h>
#include "package_private.h"

/* Loader for Sentient's VSR package format */

typedef struct VSRChunkHeader {
    char        identifier[4];
    uint32_t    length;
} VSRChunkHeader;

typedef struct VSRDirectoryIndex {
    uint32_t    start_off;      /* start offset for data */
    uint32_t    length;         /* length of the file in bytes */
    uint32_t    unknown0[8];
} VSRDirectoryIndex;
static_assert(sizeof(VSRDirectoryIndex) == 40, "needs to be 40 bytes");

typedef struct VSRDirectoryChunk {
    VSRChunkHeader      header;         /* DIRC */
    uint32_t            num_indices;    /* number of indices in this chunk */
} VSRDirectoryChunk;
static_assert(sizeof(VSRDirectoryChunk) == 12, "needs to be 12 bytes");

typedef struct VSRStringChunk {
    VSRChunkHeader  header;         /* STRT */
    uint32_t        num_indices;
} VSRStringChunk;

typedef struct VSRHeader {
    VSRChunkHeader      header;         /* VSR1 */
    uint32_t            unknown0;
    uint32_t            unknown1;       /* always 4 */
    uint32_t            unknown2;
    uint32_t            length_data;    /* length of file, minus header */
    uint32_t            num_indices;    /* number of files in the package */
    uint32_t            num_indices2;   /* ditto to the above? */
} VSRHeader;
static_assert(sizeof(VSRHeader) == 32, "needs to be 32 bytes");

static bool LoadVSRPackageFile(FILE *fh, PLPackageIndex *pi) {
    pi->file.data = pl_malloc(pi->file.size);
    if(pi->file.data == NULL) {
        return false;
    }

    if(fseek(fh, pi->offset, SEEK_SET) != 0 || fread(pi->file.data, pi->file.size, 1, fh) != 1) {
        free(pi->file.data);
        pi->file.data = NULL;
        return false;
    }

    return true;
}

PLPackage* plLoadVSRPackage(const char *path, bool cache) {
    FunctionStart();

    FILE* fp = fopen(path, "rb");
    if(fp == NULL) {
        ReportError(PL_RESULT_FILEREAD, plGetResultString(PL_RESULT_FILEREAD));
        return NULL;
    }

    VSRHeader           chunk_header;
    VSRDirectoryChunk   chunk_directory;
    VSRStringChunk      chunk_strings;

    VSRDirectoryIndex*  directories = NULL;

    /* load in all the file data first */

    fread(&chunk_header, sizeof(VSRHeader), 1, fp);
    if(strcmp(chunk_header.header.identifier, "1RSV") == 0) {
        fread(&chunk_directory, sizeof(VSRDirectoryChunk), 1, fp);
        if(strcmp(chunk_directory.header.identifier, "CRID") == 0) {
            if((directories = pl_malloc(sizeof(VSRDirectoryIndex) * chunk_directory.num_indices)) != NULL) {
                fread(directories, sizeof(VSRDirectoryIndex), chunk_directory.num_indices, fp);
            }


        } else {
            ReportBasicError(PL_RESULT_FILETYPE);
        }
    } else {
        ReportBasicError(PL_RESULT_FILETYPE);
    }

    fclose(fp);

    if(plGetFunctionResult() != PL_RESULT_SUCCESS) {
        free(directories);
        return NULL;
    }

    /* there isn't anything particularly special we can use from the header
     * besides using it for general validation */

    PLPackage* package = pl_malloc(sizeof(PLPackage));
    if(package != NULL) {
        package->internal.LoadFile = LoadVSRPackageFile;
        package->table_size = chunk_directory.num_indices;
        if((package->table = pl_calloc(package->table_size, sizeof(struct PLPackageIndex))) != NULL) {
            for(unsigned int i = 0; i < package->table_size; ++i) {
                PLPackageIndex* index = &package->table[i];
                //snprintf()
            }
        }
    }

    free(directories);

    if(package != NULL) {
        plDestroyPackage(package);
    }

    return NULL;
}
