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
#include "platform_private.h"
#include "package_private.h"

#include <PL/platform_console.h>

/* 4X Technologies Package Format
 * These come in both an IBF and LST format;
 * the LST file lists all of the contents
 * of the IBF.
 *
 * There doesn't appear to be any form of
 * compression used on these packages.
 */

PLPackage *LoadLSTPackage(const char *path, bool cache) {
    FILE *fh = fopen(path, "rb");
    if(fh == NULL) {
        return NULL;
    }

    PLPackage *package = NULL;

    /* any files we're going to load will be from the IBF, not the LST */
    char ibf_path[PL_SYSTEM_MAX_PATH];
    strncpy(ibf_path, path, strlen(path) - 3);
    strncat(ibf_path, "ibf", sizeof(ibf_path));
    if(!plFileExists(ibf_path)) {
        ReportError(PL_RESULT_FILEPATH, "failed to open ibf package at \"%s\", aborting", ibf_path);
        goto ABORT;
    }

    DebugPrint("LST %s\n", path);
    DebugPrint("IBF %s\n", ibf_path);

    /* grab the IBF size so we can do some sanity checking later */
    size_t ibf_size = plGetFileSize(ibf_path);
    if(ibf_size == 0) {
        ReportError(PL_RESULT_FILESIZE, "invalid ibf \"%s\" size of 0, aborting", ibf_path);
        goto ABORT;
    }

    /* read in the ident */
    char ident[8];
    if(fread(ident, sizeof(char), 8, fh) != 8) {
        ReportError(PL_RESULT_FILETYPE, "failed to read identification, aborting");
        goto ABORT;
    }

    /* not sure... not really needed either?
     * appears to skip over some of the initial
     * header information in the data file for
     * the first block, though could be by chance -
     * doesn't seem necessary in our case because
     * we have a separate position given to us
     * specifically with the data offset
     */
    fseek(fh, 4, SEEK_CUR);

    struct {
        char name[64];
        uint32_t data_offset;
        uint32_t data_length;
    } index;

    /* sanity checking */
    unsigned int num_indices = (unsigned int)((plGetFileSize(path) - 8) / sizeof(index));
    if(num_indices > 4096) {
        ReportError(PL_RESULT_FILESIZE, "larger than expected package, aborting");
        goto ABORT;
    }

    package = malloc(sizeof(PLPackage));
    if(package == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));
        goto ABORT;
    }

    memset(package, 0, sizeof(PLPackage));

    package->table_size = num_indices;
    package->table = calloc(num_indices, sizeof(struct PLPackageIndex));
    if(package->table == NULL) {
        goto ABORT;
    }

    strncpy(package->path, ibf_path, sizeof(package->path));

    for(unsigned int i = 0; i < num_indices; ++i) {
        if(feof(fh) != 0) {
            PrintWarning("unexpected end of package in %s, ignoring!\n", path);
            break;
        }

        if(fread(&index, sizeof(index), 1, fh) != 1) {
            ReportError(PL_RESULT_FILEREAD, "failed to read index at %d, aborting", ftell(fh));
            goto ABORT;
        }

        DebugPrint("LST INDEX %s\n", index.name);

        if(index.data_offset >= ibf_size || (uint64_t)(index.data_offset) + (uint64_t)(index.data_length) > ibf_size) {
            ReportError(PL_RESULT_FILESIZE, "offset/length falls beyond IBF size, aborting");
            goto ABORT;
        }

        strncpy(package->table[i].name, index.name, sizeof(index.name));
        package->table[i].name[sizeof(index.name)] = '\0';
        package->table[i].length = index.data_length;
        package->table[i].offset = index.data_offset;
    }

    fclose(fh);

    if(cache) {
        fh = fopen(package->path, "rb");
        if(fh == NULL) {
            ReportError(PL_RESULT_FILEERR, "failed to open ibf \"%s\", aborting", package->path);
            goto ABORT;
        }

        for(unsigned int i = 0; i < package->table_size; ++i) {
            LoadLSTPackageFile(fh, &(package->table[i]));
        }
        fclose(fh);
    }

    return package;

    ABORT:

    if(package != NULL) {
        plDeletePackage(package);
    }

    fclose(fh);

    return NULL;
}

bool LoadLSTPackageFile(FILE *fh, PLPackageIndex *pi) {
    pi->data = malloc(pi->length);
    if(pi->data == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, "Failed to allocate %d bytes!\n", pi->length);
        return false;
    }

    if(fseek(fh, pi->offset, SEEK_SET) != 0 || fread(pi->data, pi->length, 1, fh) != 1) {
        free(pi->data);
        pi->data = NULL;

        return false;
    }

    return true;
}