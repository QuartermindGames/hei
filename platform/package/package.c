
#include "package_private.h"

PLPackage *plLoadPackage(const char *path) {
    if(!plFileExists(path)) {
        _plReportError(PL_RESULT_FILEREAD, "Failed to load package, %s!", path);
        return NULL;
    }

    FILE *fin = fopen(path, "rb");
    if(fin == NULL) {

    }

    const char *extension = plGetFileExtension(path);
    if(extension[0] != '\0') {
        if((!strcmp(extension, "mad") || !strcmp(extension, "mtd")) && _plMADFormatCheck(path, fin)) {

        }
    } else { // probably less safe loading solution, your funeral!
        if(_plMADFormatCheck(path, fin)) {

        }
    }
}

void plUnloadPackage(PLPackage *package) {

}

uint8_t *plLoadPackageFile(PLPackage *package, const char *file) {

}