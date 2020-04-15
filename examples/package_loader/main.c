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

#include <PL/platform_math.h>
#include <PL/platform_console.h>
#include <PL/platform_model.h>
#include <PL/platform_package.h>
#include <PL/platform_filesystem.h>

#include "../shared.h"

int main(int argc, char **argv) {
    plInitialize(argc, argv);
    plSetupLogOutput("./package.log");

    plRegisterStandardPackageLoaders();
    plRegisterStandardModelLoaders();

    if(argc < 2) {
        PRINT(" package_loader <path> -<optional mode>\n");
        PRINT("  -extract : extract all files from the package\n");
        return EXIT_SUCCESS;
    }

    enum {
        MODE_VIEW,
        MODE_EXTRACT,
    };
    unsigned int mode = MODE_VIEW;
    if(plHasCommandLineArgument("-extract")) {
        PRINT("Extraction Mode\n");
        mode = MODE_EXTRACT;
    } else {
        PRINT("Viewing Mode\n");
    }

    char package_path[PL_SYSTEM_MAX_PATH];
    strncpy(package_path, argv[1], sizeof(package_path));
    if(package_path[0] == '\0') {
        PRINT_ERROR("Invalid path for package, aborting!\n");
    }

    PLPackage* package = plLoadPackage(package_path);
    if(package == NULL) {
        PRINT_ERROR("Failed to load package \"%s\" (%s)!\n", package_path, plGetError());
    }

    for(unsigned int i = 0; i < package->table_size; ++i) {
        char desc[PL_SYSTEM_MAX_PATH];
        if(package->table[i].fileName[0] != '\0') {
            strcpy(desc, package->table[i].fileName);
        } else {
            sprintf(desc, "%d", i);
        }

        PRINT(
                "id:   %d\n"
                "name: %s\n"
                "size: %lu\n"
                "offset: %lu\n"
                "----------------\n",
                i,
                package->table[i].fileName,
                (unsigned long) package->table[i].fileSize,
                (unsigned long) package->table[i].offset
                );

        if(mode == MODE_EXTRACT) {
            PLFile* filePtr = plLoadPackageFile( package, package->table[ i ].fileName );
            if( filePtr == NULL ) {
                PRINT( "Failed to load \"%s\" from package, skipping!\nERR: %s\n", desc, plGetError() );
                continue;
            }

            char out[PL_SYSTEM_MAX_PATH];
            snprintf(out, sizeof(out), "./extract/%s", desc);
            plCreateDirectory("./extract/");

            uint8_t *data = plGetFileData( filePtr );
            size_t length = plGetFileSize( filePtr );
            if( !plWriteFile( out, data, length ) ) {
                PRINT( "Failed to write \"%s\"!\nERR: %s\n", desc, plGetError() );
                continue;
            }

            PRINT( "Wrote \"%s\"\n", out );
        }
    }

    plDestroyPackage(package);
    plShutdown();

    return EXIT_SUCCESS;
}