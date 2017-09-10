
#pragma once

typedef struct PLPackageIndex {
    char name[256];
    size_t length;
    size_t offset;
};

typedef struct PLPackage {
    const char *path;

    unsigned int table_size;
    PLPackageIndex *table;
} PLPackage;