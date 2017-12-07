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

#include "../shared.h"

#define TITLE "Package Example"

int main(int argc, char **argv) {
    PRINT("\n " TITLE " (" __DATE__ ")\n"                                                   );
    PRINT(" Developed by...\n"                                                              );
    PRINT("   Mark \"hogsy\" Sowden (http://talonbrave.info/)\n"                            );
    PRINT("\n"                                                                              );
    PRINT("\n-------------------------------------------------------------------------\n\n" );

    plInitialize(argc, argv);

    PLPackage *new_package = plCreatePackage("./packages/models.package");
    if(new_package == NULL) {
        PRINT_ERROR("Failed to create new package!\n");
        return EXIT_FAILURE;
    }

    plDeletePackage(new_package);

#if 0
    plScanDirectory("./Models/", "mdl", load_mdl_temp, false);

    return EXIT_SUCCESS;
#else

    /* debris3
     * debris2
     * armor1
     * shellcasing2sm
     * medkit
     * lamp7
     * lamp
     * medlab
     * lion
     * ctable
     * throne
     * lamp4
     */

    PLModel *model = plLoadModel("./Models/throne.mdl");
    if(model == NULL) {
        PRINT_ERROR("Failed to load model!\n");
    }

    plShutdown();

    return 0;
#endif
}