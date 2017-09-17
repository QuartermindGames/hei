
#pragma once

typedef struct PLPackageIndex {
    char name[256];
    size_t length;
    size_t offset;

    uint8_t *data;
} PLPackageIndex;

typedef struct PLPackage {
    char *path;

    unsigned int table_size;
    PLPackageIndex *table;
} PLPackage;