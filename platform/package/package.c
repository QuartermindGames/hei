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

#include "package_private.h"

PLPackage *plCreatePackage(const char *dest) {

}

void PurgePackageData(PLPackage *package) {
    plAssert(package);

    for(unsigned int i = 0; i < package->table_size; ++i) {
        if(package->table[i].data != NULL) {
            free(package->table[i].data);
            package->table[i].data = NULL;
        }
    }
}

/* Unloads package from memory
 */
void plDeletePackage(PLPackage *package) {
    plAssert(package);

    PurgePackageData(package);
    free(package->table);
    free(package);
}

void plWritePackage(PLPackage *package) {

}

/////////////////////////////////////////////////////////////////

typedef struct PLPackageLoader {
    const char *extensions[32];
    unsigned int num_extensions;

    PLPackage*(*LoadPackage)(const char *path, bool precache);
} PLPackageLoader;

PLPackageLoader load_packs[]= {
        { { "package" }, 1, NULL },

        // Third-party package formats
        { { "mad", "mtd" }, 2, LoadMADPackage },
        { { "dat", "art" }, 2, LoadARTPackage },
};
unsigned int num_load_packs = plArrayElements(load_packs);

PLPackage *plLoadPackage(const char *path, bool cache) {
    if(!plFileExists(path)) {
        ReportError(PL_RESULT_FILEREAD, "Failed to load package, %s!", path);
        return NULL;
    }

    const char *ext = plGetFileExtension(path);
    if(ext[0] != '\0') {
        for(unsigned int i = 0; i < num_load_packs; ++i) {
            for (unsigned int j = 0; j < load_packs[i].num_extensions; ++j) {
                if (pl_strcasecmp(ext, load_packs[i].extensions[j]) == 0) {
                    PLPackage *package = load_packs[i].LoadPackage(path, cache);
                    if (package != NULL) {
                        strncpy(package->path, path, sizeof(package->path));
                        return package;
                    }
                    break;
                }
            }
        }
    } else {
        for(unsigned int i = 0; i < num_load_packs; ++i) {
            PLPackage *package = load_packs[i].LoadPackage(path, cache);
            if(package != NULL) {
                strncpy(package->path, path, sizeof(package->path));
                return package;
            }
        }
    }

    return NULL;
}

bool plLoadPackageFile(PLPackage *package, const char *file, const uint8_t **data, size_t *size) {
    for(unsigned int i = 0; i < package->table_size; ++i) {
        if(strcmp(file, package->table[i].name) == 0) {
            if(package->table[i].data == NULL) {
                FILE *fh = fopen(package->path, "rb");
                if (fh == NULL) {
                    return false;
                }

                if (!LoadMADPackageFile(fh, &(package->table[i]))) {
                    fclose(fh);
                    return false;
                }
            }

            *data = package->table[i].data;
            *size = package->table[i].length;
            return true;
        }
    }

    return false;
}
