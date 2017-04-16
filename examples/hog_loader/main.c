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

#include <platform_log.h>
#include <platform_filesystem.h>

// BEHOLD! The sloppiest application ever written!!

typedef struct FACHeader { // 00000000 00000000 00000000 00000000 0A000000 101E0219 081E3700 4D004200
    uint32_t padding[4];   // This is always blank
    uint32_t num_blocks; // Number of FACThingy0s
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t unknown4;
} FACHeader;

typedef struct FACTriangle {
    /* Seems to be some struct that provides an id among some other data... Probably the size of each?
     * So the id might be 65, further down the file there's a block to correspond to this.
     */
    uint32_t crap[8];
} FACBlock;

#define LOG "hog_loader"

#define PRINT(...) printf(__VA_ARGS__); plWriteLog(LOG, __VA_ARGS__);

void load_fac_file(const char *path) {
    FACHeader header;
    memset(&header, 0, sizeof(FACHeader));

    PRINT("Opening %s\n", path);

    FILE *file = fopen(path, "r");
    if(!file) {
        PRINT("Failed to load file %s!\n", path);
        return;
    }

    if(fread(&header, sizeof(FACHeader), 1, file) != 1) {
        PRINT("Invalid file header...\n");
        goto CLEANUP;
    }

    for(int i = 0; i < plArrayElements(header.padding); i++) {
        if(header.padding[0] != 0) {
            PRINT("Invalid FAC file!\n");
            goto CLEANUP;
        }
    }

    PRINT("num_blocks: %d\n", header.num_blocks);
    PRINT("unknown2: %d\n", header.unknown2);

    if(header.num_blocks != 0) {
        FACTriangle thingy0[header.num_blocks];
        if(fread(thingy0, sizeof(FACTriangle), header.num_blocks, file) != header.num_blocks) {
            PRINT("Invalid thingy size!!\n");
            goto CLEANUP;
        }
        printf("Got thingies!\n");
    }

    CLEANUP:
    fclose(file);
}

int main(int argc, char **argv) {
    plInitialize(argc, argv, PL_SUBSYSTEM_LOG);

    plClearLog(LOG);

    const char *arg = plGetCommandLineArgument("-path");
    if(arg[0] == '\0') {
        PRINT("Please specify a path using the -path argument!\n");
        return -1;
    }

    plScanDirectory(arg, ".fac", load_fac_file);

    return 0;
}