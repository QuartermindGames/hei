// SPDX-License-Identifier: MIT
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"
#include "qmos/public/qm_os_memory.h"

/* VPK as used by VTMB - doesn't appear to have any relation
 * to the VPK format introduced by Valve later on, hence the
 * 'vmtb_vpk' designation. */

#define VPK_TOC_OFFSET   5
#define VPK_TOC_NUMFILES 9

QmFsPackage *PlParseVpkPackage_( QmFsFile *file ) {
	size_t size = qm_fs_file_get_size( file );

	if ( !qm_fs_file_seek( file, size - VPK_TOC_NUMFILES, QM_FS_SEEK_SET ) ) {
		return NULL;
	}

	uint32_t numFiles = qm_fs_file_read_int32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "no files in package\n" );
		return NULL;
	}

	uint32_t tocOffset = qm_fs_file_read_int32( file, false, NULL );
	if ( tocOffset == 0 || tocOffset >= size ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid table offset: %u\n", tocOffset );
		return NULL;
	}

	if ( !qm_fs_file_seek( file, tocOffset, QM_FS_SEEK_SET ) ) {
		return NULL;
	}

	QmFsPackage *package = PlCreatePackageHandle( qm_fs_file_get_path( file ), numFiles, NULL );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		uint32_t nameLength = qm_fs_file_read_int32( file, false, NULL );
		char *name = QM_OS_MEMORY_NEW_( char, nameLength + 1 );
		qm_file_read( file, name, sizeof( char ), nameLength );
		snprintf( package->files[ i ].name, sizeof( package->files[ i ].name ), "%s", name );
		qm_os_memory_free( name );

		package->files[ i ].offset = qm_fs_file_read_int32( file, false, NULL );
		package->files[ i ].size = qm_fs_file_read_int32( file, false, NULL );
	}

	return package;
}
