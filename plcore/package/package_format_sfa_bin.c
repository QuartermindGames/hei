// SPDX-License-Identifier: MIT
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"

#include "3rdparty/portable_endian.h"
#include "qmos/public/qm_os_memory.h"

/* Loader for SFA TAB/BIN format */

PLPackage *PlLoadTabPackage( const char *path ) {
	char bin_path[ PL_SYSTEM_MAX_PATH + 1 ];
	strncpy( bin_path, path, strlen( path ) - 3 );
	strncat( bin_path, "bin", PL_SYSTEM_MAX_PATH );
	if ( !PlFileExists( bin_path ) ) {
		PlReportErrorF( PL_RESULT_FILEPATH, "failed to open bin package at \"%s\", aborting", bin_path );
		return NULL;
	}

	size_t tab_size = PlGetLocalFileSize( path );
	if ( tab_size == 0 ) {
		PlReportErrorF( PL_RESULT_FILESIZE, PlGetResultString( PL_RESULT_FILESIZE ) );
		return NULL;
	}

	PLFile *fp = PlOpenFile( path, false );
	if ( fp == NULL ) {
		return NULL;
	}

	typedef struct TabIndex {
		uint32_t start;
		uint32_t end;
	} TabIndex;

	unsigned int num_indices = ( unsigned int ) ( tab_size / sizeof( TabIndex ) );

	TabIndex *indices = QM_OS_MEMORY_MALLOC_( num_indices * sizeof( TabIndex ) );
	size_t ret = PlReadFile( fp, indices, sizeof( TabIndex ), num_indices );
	PlCloseFile( fp );

	if ( ret != num_indices ) {
		qm_os_memory_free( indices );
		return NULL;
	}

	/* swap be to le */
	for ( unsigned int i = 0; i < num_indices; ++i ) {
		if ( indices[ i ].start > tab_size || indices[ i ].end > tab_size ) {
			qm_os_memory_free( indices );
			PlReportErrorF( PL_RESULT_FILESIZE, "offset outside of file bounds" );
			return NULL;
		}

		indices[ i ].start = be32toh( indices[ i ].start );
		indices[ i ].end = be32toh( indices[ i ].end );
	}

	PLPackage *package = PlCreatePackageHandle( path, num_indices, NULL );
	for ( unsigned int i = 0; i < num_indices; ++i ) {
		PLPackageIndex *index = &package->table[ i ];
		snprintf( index->fileName, sizeof( index->fileName ), "%u", i );
		index->fileSize = indices[ i ].end - indices[ i ].start;
		index->offset = indices[ i ].start;
	}

	qm_os_memory_free( indices );

	return package;
}
