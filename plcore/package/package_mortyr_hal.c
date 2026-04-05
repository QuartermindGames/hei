// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"
#include "qmos/public/qm_os_memory.h"

/* Mortyr's APUK package format */

#define HAL_MAGIC QM_OS_MAGIC_TO_NUM( 'A', 'P', 'U', 'K' )

QmFsPackage *PlParseHalPackage_( QmFsFile *file ) {
	int32_t magic = qm_fs_file_read_int32( file, false, NULL );
	if ( magic != HAL_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid package identifier" );
		return NULL;
	}

	typedef struct FileIndex {
		uint32_t size;
		uint32_t offset;
		int32_t unknown[ 2 ];
		char name[ 16 ];
	} FileIndex;

	bool status;
	uint32_t numFiles = qm_fs_file_read_int32( file, false, &status );
	if ( !status ) {
		PlCloseFile( file );
		return NULL;
	}

	/* make sure the file table is valid */
	size_t tableSize = sizeof( FileIndex ) * numFiles;
	if ( tableSize + 32 > qm_fs_file_get_size( file ) ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "invalid file table" );
		return NULL;
	}

	/* 24 bytes of nothing, can't think what this was intended for... */
	if ( !qm_fs_file_seek( file, 24, QM_FS_SEEK_CUR ) ) {
		return NULL;
	}

	FileIndex *indices = QM_OS_MEMORY_MALLOC_( sizeof( FileIndex ) * numFiles );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		indices[ i ].size = qm_fs_file_read_int32( file, false, &status );
		indices[ i ].offset = qm_fs_file_read_int32( file, false, &status );
		indices[ i ].unknown[ 0 ] = qm_fs_file_read_int32( file, false, &status );
		indices[ i ].unknown[ 1 ] = qm_fs_file_read_int32( file, false, &status );

		if ( PlReadFile( file, indices[ i ].name, 1, 16 ) != 16 ) {
			status = false;
		}
	}

	if ( !status ) {
		qm_os_memory_free( indices );
		return NULL;
	}

	/* yay, we're finally done - now to setup the package object */

	QmFsPackage *package = PlCreatePackageHandle( qm_fs_file_get_path( file ), numFiles, NULL );
	for ( unsigned int i = 0; i < package->maxTableSize; ++i ) {
		PLPackageIndex *index = &package->table[ i ];
		index->offset = indices[ i ].offset;
		index->fileSize = indices[ i ].size;
		snprintf( index->fileName, sizeof( index->fileName ), "%s", indices[ i ].name );
	}

	qm_os_memory_free( indices );

	return package;
}
