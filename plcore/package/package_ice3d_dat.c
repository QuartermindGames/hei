// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"
#include "qmos/public/qm_os_memory.h"

QmFsPackage *PlParseIce3DDatPackage_( QmFsFile *file ) {
	uint16_t versionMajor = PL_READUINT16( file, false, NULL );
	uint16_t versionMinor = PL_READUINT16( file, false, NULL );
	if ( versionMajor != 1 || versionMinor != 4 ) {
		PlReportErrorF( PL_RESULT_FILEVERSION, "invalid version (%u != 1 || %u != 4 )", versionMajor, versionMinor );
		return NULL;
	}

	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "unexpected file count" );
		return NULL;
	}

	QmFsPackage *package = PlCreatePackageHandle( qm_fs_file_get_path( file ), numFiles, NULL );
	for ( uint32_t i = 0; i < numFiles; ++i ) {
		PL_READUINT32( file, false, NULL );// index
		PL_READUINT16( file, false, NULL );// ??

		package->files[ i ].size = PL_READUINT32( file, false, NULL );

		uint32_t stringSize = PL_READUINT32( file, false, NULL );
		char *buf = QM_OS_MEMORY_NEW_( char, stringSize + 1 );
		qm_file_read( file, buf, sizeof( char ), stringSize );
		snprintf( package->files[ i ].name, sizeof( package->files[ i ].name ), "%s", buf );
		qm_os_memory_free( buf );

		package->files[ i ].offset = PL_READUINT32( file, false, NULL ) + 8;
	}

	return package;
}
