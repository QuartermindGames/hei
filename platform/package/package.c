
#include "package_private.h"

PLPackage *plLoadPackage(const char *path, bool precache) {
    if(!plFileExists(path)) {
        _plReportError(PL_RESULT_FILEREAD, "Failed to load package, %s!", path);
        return NULL;
    }

    PLPackage *package;

    const char *extension = plGetFileExtension(path);
    if(extension[0] != '\0') {
        if((!strcmp(extension, "mad") || !strcmp(extension, "mtd")) && (package = _plLoadMADPackage(path, precache)) != NULL) {
            return package;
        }
    } else { // probably less safe loading solution, your funeral!
        if((package = _plLoadMADPackage(path, precache)) != NULL) {
            return package;
        }
    }

    return NULL;
}

void plUnloadPackage(PLPackage *package) {
}

bool plLoadPackageFile(PLPackage *package, const char *file, const uint8_t **data, size_t *size) {
    for(unsigned int i = 0; i < package->table_size; ++i) {
        if(strcmp(file, package->table[i].name) == 0) {
            if(package->table[i].data == NULL) {
                FILE *fh = fopen(package->path, "rb");
                if (fh == NULL) {
                    return false;
                }

                if (!_plLoadMADPackageFile(fh, &(package->table[i]))) {
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
