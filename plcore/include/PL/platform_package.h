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

#pragma once

#include <PL/platform.h>
#include <PL/platform_filesystem.h>

typedef enum PLCompressionType {
	PL_COMPRESSION_NONE,
	PL_COMPRESSION_ZLIB,

	PL_MAX_COMPRESSION_FORMATS
} PLCompressionType;

typedef struct PLPackageIndex {
	size_t offset;
	char fileName[ PL_SYSTEM_MAX_PATH ];
	size_t fileSize;
	size_t compressedSize;
	PLCompressionType compressionType;
} PLPackageIndex;

typedef struct PLPackage {
	char path[PL_SYSTEM_MAX_PATH];
	unsigned int    table_size;
	PLPackageIndex* table;
	struct {
		uint8_t* (* LoadFile)( PLFile* package, PLPackageIndex* index );
	} internal;
} PLPackage;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN PLPackage *plCreatePackageHandle( const char *path, unsigned int tableSize, uint8_t*(*OpenFile)( PLFile *filePtr, PLPackageIndex *index ) );

PL_EXTERN PLPackage* plLoadPackage( const char* path );
PL_EXTERN PLFile* plLoadPackageFile( PLPackage* package, const char* path );
PL_EXTERN PLFile *plLoadPackageFileByIndex( PLPackage *package, unsigned int index );
PL_EXTERN void plDestroyPackage( PLPackage* package );

PL_EXTERN void plRegisterPackageLoader( const char* ext, PLPackage* (* LoadFunction)( const char* path ) );
PL_EXTERN void plRegisterStandardPackageLoaders( void );
PL_EXTERN void plClearPackageLoaders( void );

PL_EXTERN const char *plGetPackagePath( const PLPackage *package );
PL_EXTERN unsigned int plGetPackageTableSize( const PLPackage *package );
PL_EXTERN unsigned int plGetPackageTableIndex( const PLPackage *package, const char *indexName );

const char *plGetPackageFileName( const PLPackage *package, unsigned int index );

#endif

PL_EXTERN_C_END
