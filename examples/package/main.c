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
#include <PL/platform_model.h>
#include <PL/platform_filesystem.h>

#include "../shared.h"

#define TITLE "Package Example"

int main(int argc, char **argv) {
    plInitialize(argc, argv);

#if 0
    PLPackage *new_package = plCreatePackage("./packages/models.package");
    if(new_package == NULL) {
        PRINT_ERROR("Failed to create new package!\n");
    }

    PLModel *model = plLoadModel("./Models/throne.mdl");
    if(model == NULL) {
        PRINT_ERROR("Failed to load model!\n");
    }

    uint8_t *data = plSerializeModel(model, PL_SERIALIZE_MODEL_BASE);
    plAddPackageBlob(new_package, PL_PACKAGE_MODEL, data);

    plDeleteModel(model);

    plWritePackage(new_package);

    plDeletePackage(new_package);
#endif

    PLPackage *pack = plLoadPackage("./packs/Others.lst", true);
    if(pack == NULL) {
        printf("%s\n", plGetError());
        return EXIT_FAILURE;
    }

    plCreateDirectory("./packs/extracted/");
    for(unsigned int i = 0; i < pack->table_size; ++i) {
        char out_path[PL_SYSTEM_MAX_PATH];
        snprintf(out_path, sizeof(out_path), "./packs/extracted/%s", pack->table[i].name);
        FILE *fout = fopen(out_path, "wb");
        if(fout == NULL) {
            printf("failed to open %s for writing\n", out_path);
            continue;
        }

        fwrite(pack->table[i].data, pack->table[i].length, 1, fout);
        fclose(fout);
    }

    plDeletePackage(pack);

    plShutdown();

    return 0;
}