/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_filesystem.h>

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
	char path[ PL_SYSTEM_MAX_PATH ];
	unsigned int table_size;
	PLPackageIndex *table;
	struct {
		uint8_t *( *LoadFile )( PLFile *package, PLPackageIndex *index );
	} internal;
} PLPackage;

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PL_EXTERN PLPackage *PlCreatePackageHandle( const char *path, unsigned int tableSize, uint8_t *( *OpenFile )( PLFile *filePtr, PLPackageIndex *index ) );

PL_EXTERN PLPackage *PlLoadPackage( const char *path );
PL_EXTERN PLFile *PlLoadPackageFile( PLPackage *package, const char *path );
PL_EXTERN PLFile *PlLoadPackageFileByIndex( PLPackage *package, unsigned int index );
PL_EXTERN void PlDestroyPackage( PLPackage *package );

PL_EXTERN void PlRegisterPackageLoader( const char *ext, PLPackage *( *LoadFunction )( const char *path ) );
PL_EXTERN void PlRegisterStandardPackageLoaders( void );
PL_EXTERN void PlClearPackageLoaders( void );

PL_EXTERN const char *PlGetPackagePath( const PLPackage *package );
PL_EXTERN unsigned int PlGetPackageTableSize( const PLPackage *package );
PL_EXTERN int PlGetPackageTableIndex( const PLPackage *package, const char *indexName );

const char *PlGetPackageFileName( const PLPackage *package, unsigned int index );

#endif

PL_EXTERN_C_END
