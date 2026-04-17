// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_filesystem.h>
#include <plcore/pl_compression.h>

typedef struct QmFsPackageFile
{
	PLPath            sourcePath;// Where the file resides on disk - only matters if writing...
	PLPath            name;
	uint64_t          offset;
	size_t            size;
	size_t            compressedSize;
	PLCompressionType compressionType;
} QmFsPackageFile;

typedef struct QmFsPackage
{
	char            *path;
	unsigned int     numFiles;
	unsigned int     maxFiles;
	QmFsPackageFile *files;
	struct
	{
		void *( *LoadFile )( QmFsFile *package, QmFsPackageFile *index );
	} internal;
} QmFsPackage;

enum
{
	PL_PACKAGE_LOAD_FORMAT_ALL = 0,

	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_ZIP, 0 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_WAD_DOOM, 1 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_WAD_QUAKE, 2 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_MAD_GREMLIN, 3 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_PAK_QUAKE, 4 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_TAB_SFA, 5 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_BIN_FRESH, 6 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_DFS, 7 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_VPK_VTMB, 8 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_GRP, 9 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_VPP, 10 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_OPK, 11 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_INU, 12 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_ALL_ACCLAIM, 13 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_AFS, 14 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_AHF, 15 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_DAT_ANGEL, 16 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_HAL, 17 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_DAT_ICE3D, 18 ),
#if ( RAR_SUPPORTED == 1 )
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_RAR, 19 ),
#endif
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_FRD_PAK, 20 ),
	QM_OS_BIT_FLAG( PL_PACKAGE_LOAD_FORMAT_PAK_VENOM, 21 ),
};

PL_EXTERN_C

QmFsPackage *PlCreatePackageHandle( const char *path, unsigned int tableSize, void *( *OpenFile )( QmFsFile *filePtr, QmFsPackageFile *index ) );

QmFsPackage *PlLoadPackage( const char *path );
QmFsFile    *PlLoadPackageFile( QmFsPackage *package, const char *path );
QmFsFile    *PlLoadPackageFileByIndex( QmFsPackage *package, unsigned int index );
void         PlDestroyPackage( QmFsPackage *package );
void         PlExtractPackage( QmFsPackage *package, const char *path );

void         PlRegisterPackageLoader( const char *ext, QmFsPackage *( *LoadFunction )( const char *path ), QmFsPackage *( *ParseFunction )( QmFsFile * ) );
void         PlRegisterStandardPackageLoaders( unsigned int flags );
void         PlClearPackageLoaders( void );
const char **PlGetSupportedPackageFormats( unsigned int *numElements );

const char  *PlGetPackagePath( const QmFsPackage *package );
unsigned int PlGetPackageTableSize( const QmFsPackage *package );
int          PlGetPackageTableIndex( const QmFsPackage *package, const char *indexName );

const char *PlGetPackageFileName( const QmFsPackage *package, unsigned int index );

QmFsPackage *PlLoadZipPackage( const char *path );
QmFsPackage *PlParseZipPackage( QmFsFile *file );

QmFsPackage *PlParseVppPackage( QmFsFile *file );
QmFsPackage *PlParseVenomPakPackage( QmFsFile *file );

#if 0// Write API - excluded for now...

enum {
	PL_PACKAGE_WRITE_FORMAT_ALL = 0,

	PL_BITFLAG( PL_PACKAGE_WRITE_FORMAT_BIN_FRESH, 0 ),
};

#	define PL_PACKAGE_FORMAT_TAG_BIN_FRESH "fresh.bin"

#	define PL_PACKAGE_WRITE_ENABLED

typedef bool ( *PLWritePackageFunction )( QmFsPackage *package, const char *path );

void PlRegisterPackageWriter( const char *formatTag, PLWritePackageFunction writeFunction );
void PlRegisterStandardPackageWriters( unsigned int flags );
void PlClearPackageWriters( void );
QmFsPackageFile *PlAppendPackageFromFile( QmFsPackage *package, const char *source, const char *filename, PLCompressionType compressionType );
bool PlWritePackage( QmFsPackage *package, const char *path, const char *formatTag );

#endif

PL_EXTERN_C_END
