/**
 * Hei Platform Library
 * Copyright (C) 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_package.h>

typedef struct TREIndex {
	uint32_t offset;
	uint32_t size;
	uint32_t unk0;
	uint32_t hash;
} TREIndex;
PL_STATIC_ASSERT( sizeof( TREIndex ) == 16, "needs to be 16 bytes" );

static TREIndex *parse_tre_index( PLFile *file, TREIndex *out ) {
	bool status;
	out->offset = ( uint32_t ) PlReadInt32( file, false, &status );
	if ( !status ) {
		return NULL;
	}
	out->size = ( uint32_t ) PlReadInt32( file, false, &status );
	if ( !status ) {
		return NULL;
	}
	out->unk0 = ( uint32_t ) PlReadInt32( file, false, &status );
	if ( !status ) {
		return NULL;
	}
	out->hash = ( uint32_t ) PlReadInt32( file, false, &status );
	if ( !status ) {
		return NULL;
	}
	return out;
}

static PLPackage *parse_tre_file( PLFile *file ) {
	uint32_t numFiles = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid tre file" );
		return NULL;
	}

	/* right, bit of extra crap just to be sure we're dealing with a valid file */
	size_t size = PlGetFileSize( file );
	size_t tocSize = sizeof( TREIndex ) * numFiles;
	if ( tocSize >= size ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid tre file" );
		return NULL;
	}
	/* try loading in the first index and ensure that's all dandy too */
	TREIndex tmp;
	if ( parse_tre_index( file, &tmp ) == NULL ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid tre file" );
		return NULL;
	}
	if ( tmp.offset < ( tocSize + 4 ) || tmp.offset >= size ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid tre file" );
		return NULL;
	}

	PlFileSeek( file, 4, PL_SEEK_SET );

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	if ( package == NULL ) {
		return NULL;
	}

	for ( unsigned int i = 0; i < numFiles; ++i ) {
		if ( parse_tre_index( file, &tmp ) == NULL ) {
			PlDestroyPackage( package );
			return NULL;
		}

		package->table[ i ].offset = tmp.offset;
		package->table[ i ].fileSize = tmp.size;
		snprintf( package->table[ i ].fileName, sizeof( package->table[ i ].fileSize ), "%X", tmp.hash );
	}

	return package;
}

PLPackage *asa_format_tre_load( const char *path ) {
	PLFile *file = PlOpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}
	PLPackage *package = parse_tre_file( file );
	PlCloseFile( file );
	return package;
}
