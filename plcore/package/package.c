/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include "pl_private.h"
#include "package_private.h"
#include "filesystem_private.h"

#include "miniz/miniz.h"

/**
 * Generic loader for package files, since this is unlikely to change
 * in most cases.
 */
static uint8_t *LoadGenericPackageFile( PLFile *fh, PLPackageIndex *pi ) {
	FunctionStart();

	if ( !PlFileSeek( fh, ( signed ) pi->offset, PL_SEEK_SET ) ) {
		return NULL;
	}

	size_t size = ( pi->compressionType != PL_COMPRESSION_NONE ) ? pi->compressedSize : pi->fileSize;
	uint8_t *dataPtr = pl_malloc( size );
	if ( PlReadFile( fh, dataPtr, sizeof( uint8_t ), size ) != size ) {
		pl_free( dataPtr );
		return NULL;
	}

	if ( pi->compressionType == PL_COMPRESSION_ZLIB ) {
		uint8_t *decompressedPtr = pl_malloc( pi->fileSize );
		mz_ulong uncompressedLength = ( mz_ulong ) pi->fileSize;
		int status = mz_uncompress( decompressedPtr, &uncompressedLength, dataPtr, ( mz_ulong ) pi->compressedSize );

		pl_free( dataPtr );
		dataPtr = decompressedPtr;

		if ( status != MZ_OK ) {
			pl_free( dataPtr );
			PlReportErrorF( PL_RESULT_FILEREAD, "failed to decompress buffer (%s)", mz_error( status ) );
			return NULL;
		}
	}

	return dataPtr;
}

/**
 * Allocate a new package handle.
 */
PLPackage *PlCreatePackageHandle( const char *path, unsigned int tableSize, uint8_t *( *OpenFile )( PLFile *filePtr, PLPackageIndex *index ) ) {
	PLPackage *package = pl_malloc( sizeof( PLPackage ) );

	if ( OpenFile == NULL ) {
		package->internal.LoadFile = LoadGenericPackageFile;
	} else {
		package->internal.LoadFile = OpenFile;
	}

	package->table_size = tableSize;
	package->table = pl_calloc( tableSize, sizeof( PLPackageIndex ) );

	snprintf( package->path, sizeof( package->path ), "%s", path );

	return package;
}

/* Unloads package from memory
 */
void PlDestroyPackage( PLPackage *package ) {
	if ( package == NULL ) {
		return;
	}

	pl_free( package->table );
	pl_free( package );
}
#if 0// todo
void plWritePackage(PLPackage *package) {

}
#endif
/////////////////////////////////////////////////////////////////

typedef struct PLPackageLoader {
	const char *ext;
	PLPackage *( *LoadFunction )( const char *path );
} PLPackageLoader;

static PLPackageLoader package_loaders[ MAX_OBJECT_INTERFACES ];
static unsigned int num_package_loaders = 0;

void PlInitPackageSubSystem( void ) {
	PlClearPackageLoaders();
}

#if 0 /* todo */
void plQuerySupportedPackages(char **array, unsigned int *size) {
	static char
}
#endif

void PlClearPackageLoaders( void ) {
	memset( package_loaders, 0, sizeof( PLPackageLoader ) * MAX_OBJECT_INTERFACES );
	num_package_loaders = 0;
}

void PlRegisterPackageLoader( const char *ext, PLPackage *( *LoadFunction )( const char *path ) ) {
	package_loaders[ num_package_loaders ].ext = ext;
	package_loaders[ num_package_loaders ].LoadFunction = LoadFunction;
	num_package_loaders++;
}

void PlRegisterStandardPackageLoaders( void ) {
	/* outwars */
	PlRegisterPackageLoader( "ff", PlLoadFfPackage );
	/* hogs of war */
	PlRegisterPackageLoader( "mad", PlLoadMadPackage );
	PlRegisterPackageLoader( "mtd", PlLoadMadPackage );
	/* iron storm */
	PlRegisterPackageLoader( "lst", PlLoadLstPackage );
	/* starfox adventures */
	PlRegisterPackageLoader( "tab", PlLoadTabPackage );
	/* sentient */
	PlRegisterPackageLoader( "vsr", PlLoadVsrPackage );
	/* doom */
	PlRegisterPackageLoader( "wad", PlLoadWadPackage );
	/* eradicator */
	PlRegisterPackageLoader( "rid", PlLoadRidbPackage );
	PlRegisterPackageLoader( "rim", PlLoadRidbPackage );
	/* mortyr */
	PlRegisterPackageLoader( "hal", PlLoadApukPackage );
}

PLPackage *PlLoadPackage( const char *path ) {
	FunctionStart();

	const char *ext = PlGetFileExtension( path );
	for ( unsigned int i = 0; i < num_package_loaders; ++i ) {
		if ( package_loaders[ i ].LoadFunction == NULL ) {
			break;
		}

		if ( !PL_INVALID_STRING( ext ) && !PL_INVALID_STRING( package_loaders[ i ].ext ) ) {
			if ( pl_strncasecmp( ext, package_loaders[ i ].ext, sizeof( package_loaders[ i ].ext ) ) == 0 ) {
			    PLPackage *package = package_loaders[ i ].LoadFunction( path );
				if ( package != NULL ) {
				    strncpy( package->path, path, sizeof( package->path ) );
					return package;
				}
			}
		} else if ( PL_INVALID_STRING( ext ) && PL_INVALID_STRING( package_loaders[ i ].ext ) ) {
		    PLPackage *package = package_loaders[ i ].LoadFunction( path );
			if ( package != NULL ) {
			    strncpy( package->path, path, sizeof( package->path ) );
				return package;
			}
		}
	}

	return NULL;
}

PLFile *PlLoadPackageFile( PLPackage *package, const char *path ) {
	if ( package->internal.LoadFile == NULL ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "package has not been initialized, no LoadFile function assigned, aborting" );
		return NULL;
	}

	for ( unsigned int i = 0; i < package->table_size; ++i ) {
		if ( strcmp( path, package->table[ i ].fileName ) != 0 ) {
			continue;
		}

		/* load in the package */
		PLFile *packageFile = PlOpenFile( package->path, true );
		if ( packageFile == NULL ) {
			return NULL;
		}

		PLFile *file = NULL;

		uint8_t *dataPtr = package->internal.LoadFile( packageFile, &( package->table[ i ] ) );
		if ( dataPtr != NULL ) {
			file = pl_malloc( sizeof( PLFile ) );
			snprintf( file->path, sizeof( file->path ), "%s", package->table[ i ].fileName );
			file->size = package->table[ i ].fileSize;
			file->data = dataPtr;
			file->pos = file->data;
		}

		PlCloseFile( packageFile );

		return file;
	}

	PlReportErrorF( PL_RESULT_INVALID_PARM2, "failed to find file in package" );
	return NULL;
}

PLFile *PlLoadPackageFileByIndex( PLPackage *package, unsigned int index ) {
	if ( index >= package->table_size ) {
		PlReportBasicError( PL_RESULT_INVALID_PARM2 );
		return NULL;
	}

	return PlLoadPackageFile( package, package->table[ index ].fileName );
}

const char *PlGetPackagePath( const PLPackage *package ) {
	return package->path;
}

const char *PlGetPackageFileName( const PLPackage *package, unsigned int index ) {
	if ( index >= package->table_size ) {
		PlReportBasicError( PL_RESULT_INVALID_PARM2 );
		return NULL;
	}

	return package->table[ index ].fileName;
}

unsigned int PlGetPackageTableSize( const PLPackage *package ) {
	return package->table_size;
}

int PlGetPackageTableIndex( const PLPackage *package, const char *indexName ) {
	FunctionStart();

	for ( unsigned int i = 0; i < package->table_size; ++i ) {
		if ( strcmp( indexName, package->table[ i ].fileName ) != 0 ) {
			continue;
		}

		return i;
	}

	PlReportBasicError( PL_RESULT_INVALID_PARM2 );

	return -1;
}
