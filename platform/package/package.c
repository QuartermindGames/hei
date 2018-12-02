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

PLPackage *plCreatePackage(const char *dest) {
    if(dest == NULL || dest[0] == '\0') {
        ReportError(PL_RESULT_FILEPATH, "invalid path");
        return NULL;
    }

    PLPackage *package = pl_calloc(1, sizeof(PLPackage));
    if(package == NULL) {
        ReportError(PL_RESULT_MEMORY_ALLOCATION, plGetResultString(PL_RESULT_MEMORY_ALLOCATION));
        return NULL;
    }

    memset(package, 0, sizeof(PLPackage));

    strncpy(package->path, dest, sizeof(package->path));

    return NULL;
}

static void PurgePackageData(PLPackage *package) {
    plAssert(package);

    for(unsigned int i = 0; i < package->table_size; ++i) {
        if(package->table[i].file.data != NULL) {
            free(package->table[i].file.data);
            package->table[i].file.data = NULL;
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
#if 0 // todo
void plWritePackage(PLPackage *package) {

}
#endif
/////////////////////////////////////////////////////////////////

typedef struct PLPackageLoader {
    const char *ext;
    PLPackage*(*LoadFunction)(const char *path, bool cache);
} PLPackageLoader;

static PLPackageLoader package_interfaces[MAX_OBJECT_INTERFACES]= {
        { "package", NULL },

        // Third-party package formats

        /* Gremlin Interactive */
        { "mad", LoadMADPackage },
        { "mtd", LoadMADPackage },

        /* Cyclone */
        //{ "dat", LoadARTPackage },
        //{ "art", LoadARTPackage },
};
static unsigned int num_package_interfaces = 3;

void plClearPackageInterfaces(void) {
    memset(package_interfaces, 0, sizeof(PLPackageLoader) * MAX_OBJECT_INTERFACES);
    num_package_interfaces = 0;
}

void plRegisterPackageInterface(const char *ext, PLPackage*(*LoadFunction)(const char *path, bool cache)) {
    package_interfaces[num_package_interfaces].ext = ext;
    package_interfaces[num_package_interfaces].LoadFunction = LoadFunction;
    num_package_interfaces++;
}

PLPackage *plLoadPackage(const char *path, bool cache) {
    if(!plFileExists(path)) {
        ReportError(PL_RESULT_FILEREAD, "failed to load package, %s", path);
        return NULL;
    }

    const char *ext = plGetFileExtension(path);
    for(unsigned int i = 0; i < num_package_interfaces; ++i) {
        if(package_interfaces[i].LoadFunction == NULL) {
            break;
        }

        if(!plIsEmptyString(ext) && !plIsEmptyString(package_interfaces[i].ext)) {
            if(pl_strncasecmp(ext, package_interfaces[i].ext, sizeof(package_interfaces[i].ext)) == 0) {
                PLPackage *package = package_interfaces[i].LoadFunction(path, cache);
                if(package != NULL) {
                    return package;
                }
            }
        } else if(plIsEmptyString(ext) && plIsEmptyString(package_interfaces[i].ext)) {
            PLPackage *package = package_interfaces[i].LoadFunction(path, cache);
            if(package != NULL) {
                return package;
            }
        }
    }

    return NULL;
}

bool plLoadPackageFile(PLPackage *package, const char *file, const uint8_t **data, size_t *size) {
    if(package->internal.LoadFile == NULL) {
        ReportError(PL_RESULT_FILEREAD, "package has not been initialized, no LoadFile function assigned, aborting");
        return false;
    }

    for(unsigned int i = 0; i < package->table_size; ++i) {
        if(strcmp(file, package->table[i].file.name) == 0) {
            if(package->table[i].file.data == NULL) {
                FILE *fh = fopen(package->path, "rb");
                if (fh == NULL) {
                    return false;
                }

                if (!package->internal.LoadFile(fh, &(package->table[i]))) {
                    fclose(fh);
                    return false;
                }
            }

            *data = package->table[i].file.data;
            *size = package->table[i].file.size;
            return true;
        }
    }

    return false;
}
