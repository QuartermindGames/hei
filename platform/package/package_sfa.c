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

#include "package_private.h"

#include "3rdparty/portable_endian.h"

/* Loader for SFA TAB/BIN format */

static uint8_t* LoadTABPackageFile( PLFile* fh, PLPackageIndex* pi ) {
	uint8_t* dataPtr = pl_malloc( pi->fileSize );
	if ( !plFileSeek( fh, (signed)pi->offset, PL_SEEK_SET ) || plReadFile( fh, dataPtr, pi->fileSize, 1 ) != 1 ) {
		pl_free( dataPtr );
		return NULL;
	}

	return dataPtr;
}

PLPackage* plLoadTABPackage( const char* path ) {
	char bin_path[PL_SYSTEM_MAX_PATH + 1];
	strncpy( bin_path, path, strlen( path ) - 3 );
	strncat( bin_path, "bin", PL_SYSTEM_MAX_PATH );
	if ( !plFileExists( bin_path ) ) {
		ReportError( PL_RESULT_FILEPATH, "failed to open bin package at \"%s\", aborting", bin_path );
		return NULL;
	}

	size_t tab_size = plGetLocalFileSize( path );
	if ( tab_size == 0 ) {
		ReportError( PL_RESULT_FILESIZE, plGetResultString( PL_RESULT_FILESIZE ) );
		return NULL;
	}

	PLFile* fp = plOpenFile( path, false );
	if ( fp == NULL ) {
		return NULL;
	}

	typedef struct TabIndex {
		uint32_t start;
		uint32_t end;
	} TabIndex;

	unsigned int num_indices = ( unsigned int ) ( tab_size / sizeof( TabIndex ) );

	TabIndex* indices = pl_malloc( num_indices * sizeof( TabIndex ) );
	int ret = plReadFile( fp, indices, sizeof( TabIndex ), num_indices );
	plCloseFile( fp );

	if ( ret != num_indices ) {
		pl_free( indices );
		return NULL;
	}

	/* swap be to le */
	for ( unsigned int i = 0; i < num_indices; ++i ) {
		if ( indices[ i ].start > tab_size || indices[ i ].end > tab_size ) {
			pl_free( indices );
			ReportError( PL_RESULT_FILESIZE, "offset outside of file bounds" );
			return NULL;
		}

		indices[ i ].start = be32toh( indices[ i ].start );
		indices[ i ].end = be32toh( indices[ i ].end );
	}

	PLPackage* package = pl_malloc( sizeof( PLPackage ) );
	if ( package != NULL ) {
		memset( package, 0, sizeof( PLPackage ) );

		strncpy( package->path, bin_path, sizeof( package->path ) );

		package->internal.LoadFile = LoadTABPackageFile;
		package->table_size = num_indices;
		package->table = pl_calloc( package->table_size, sizeof( struct PLPackageIndex ) );
		if ( package->table != NULL ) {
			for ( unsigned int i = 0; i < num_indices; ++i ) {
				PLPackageIndex* index = &package->table[ i ];
				snprintf( index->fileName, sizeof( index->fileName ), "%u", i );
				index->fileSize = indices[ i ].end - indices[ i ].start;
				index->offset = indices[ i ].start;
			}
		} else {
			plDestroyPackage( package );
			package = NULL;
		}
	}

	pl_free( indices );

	return package;
}
