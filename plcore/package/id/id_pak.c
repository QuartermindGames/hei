// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "../package_private.h"

#include "qmos/public/qm_os.h"

#define PAK_MAGIC QM_OS_MAGIC_TO_NUM( 'P', 'A', 'C', 'K' )

#define PAK_INDEX_FILENAME_LENGTH 56
#define PAK_INDEX_LENGTH          64

QmFsPackage *PlParsePakPackage_( QmFsFile *file ) {
	uint32_t magic = qm_fs_file_read_int32( file, false, NULL );
	if ( magic != PAK_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic: %X", magic );
		return NULL;
	}

	size_t fileSize = qm_fs_file_get_size( file );
	uint32_t tocOffset = qm_fs_file_read_int32( file, false, NULL );
	if ( tocOffset >= fileSize ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid table offset: %u\n", tocOffset );
		return NULL;
	}

	uint32_t tocSize = qm_fs_file_read_int32( file, false, NULL );
	if ( tocSize == 0 || tocSize + tocOffset >= fileSize ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid table size: %u\n", tocSize );
		return NULL;
	}

	if ( !qm_fs_file_seek( file, tocOffset, QM_FS_SEEK_SET ) ) {
		return NULL;
	}

	unsigned int numFiles = tocSize / PAK_INDEX_LENGTH;
	QmFsPackage *package = PlCreatePackageHandle( qm_fs_file_get_path( file ), numFiles, NULL );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		PlReadFile( file, package->files[ i ].name, sizeof( char ), PAK_INDEX_FILENAME_LENGTH );
		package->files[ i ].offset = qm_fs_file_read_int32( file, false, NULL );
		package->files[ i ].size = qm_fs_file_read_int32( file, false, NULL );
	}

	return package;
}
