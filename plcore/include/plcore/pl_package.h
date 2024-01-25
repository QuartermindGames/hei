/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_filesystem.h>
#include <plcore/pl_compression.h>

typedef struct PLPackageIndex {
	PLPath sourcePath;// Where the file resides on disk - only matters if writing...
	PLPath fileName;
	size_t offset;
	size_t fileSize;
	size_t compressedSize;
	PLCompressionType compressionType;
} PLPackageIndex;

typedef struct PLPackage {
	PLPath path;
	unsigned int table_size;
	unsigned int maxTableSize;
	PLPackageIndex *table;
	struct {
		void *( *LoadFile )( PLFile *package, PLPackageIndex *index );
	} internal;
} PLPackage;

enum {
	PL_PACKAGE_LOAD_FORMAT_ALL = 0,

	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_ZIP, 0 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_WAD_DOOM, 1 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_WAD_QUAKE, 2 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_MAD_GREMLIN, 3 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_PAK_QUAKE, 4 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_TAB_SFA, 5 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_BIN_FRESH, 6 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_DFS, 7 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_VPK_VTMB, 8 ),
	PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_GRP, 9 ),
    PL_BITFLAG( PL_PACKAGE_LOAD_FORMAT_VPP, 10 ),
};

enum {
	PL_PACKAGE_WRITE_FORMAT_ALL = 0,

	PL_BITFLAG( PL_PACKAGE_WRITE_FORMAT_BIN_FRESH, 0 ),
};

#define PL_PACKAGE_FORMAT_TAG_BIN_FRESH "fresh.bin"

PL_EXTERN_C

#if !defined( PL_COMPILE_PLUGIN )

PLPackage *PlCreatePackageHandle( const char *path, unsigned int tableSize, void *( *OpenFile )( PLFile *filePtr, PLPackageIndex *index ) );

PLPackage *PlLoadPackage( const char *path );
PLFile *PlLoadPackageFile( PLPackage *package, const char *path );
PLFile *PlLoadPackageFileByIndex( PLPackage *package, unsigned int index );
void PlDestroyPackage( PLPackage *package );
void PlExtractPackage( PLPackage *package, const char *path );

void PlRegisterPackageLoader( const char *ext, PLPackage *( *LoadFunction )( const char *path ), PLPackage *( *ParseFunction )( PLFile * ) );
void PlRegisterStandardPackageLoaders( unsigned int flags );
void PlClearPackageLoaders( void );
const char **PlGetSupportedPackageFormats( unsigned int *numElements );

const char *PlGetPackagePath( const PLPackage *package );
unsigned int PlGetPackageTableSize( const PLPackage *package );
int PlGetPackageTableIndex( const PLPackage *package, const char *indexName );

const char *PlGetPackageFileName( const PLPackage *package, unsigned int index );

PLPackage *PlLoadZipPackage( const char *path );
PLPackage *PlParseZipPackage( PLFile *file );

PLPackage *PlParseVppPackage( PLFile *file );

#	if 0// Write API - excluded for now...

#define PL_PACKAGE_WRITE_ENABLED

typedef bool ( *PLWritePackageFunction )( PLPackage *package, const char *path );

void PlRegisterPackageWriter( const char *formatTag, PLWritePackageFunction writeFunction );
void PlRegisterStandardPackageWriters( unsigned int flags );
void PlClearPackageWriters( void );
PLPackageIndex *PlAppendPackageFromFile( PLPackage *package, const char *source, const char *filename, PLCompressionType compressionType );
bool PlWritePackage( PLPackage *package, const char *path, const char *formatTag );

#	endif

#endif

PL_EXTERN_C_END
