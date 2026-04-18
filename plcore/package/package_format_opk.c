// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"

/* Outcast OPK format */

QmFsPackage *PlParseOpkPackage_( QmFsFile *file ) {
	static const int32_t opkMagic = 0x6e71;
	int32_t magic = qm_fs_file_read_int32( file, false, NULL );
	if ( magic != opkMagic ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected magic: %d", magic );
		return NULL;
	}

	/* these seem to provide some sort of metadata for
	 * identifying the file type? noticed other files
	 * feature the same magic, but then these same bytes
	 * are different depending on the type. interesting. */
	if ( !qm_fs_file_seek( file, 12, QM_FS_SEEK_CUR ) ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "failed to seek to table" );
		return NULL;
	}

	int32_t numFiles = qm_fs_file_read_int32( file, false, NULL );
	if ( numFiles <= 0 ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "no files in package" );
		return NULL;
	}

	/* read in toc */
	QmFsPackage *package = PlCreatePackageHandle( qm_fs_file_get_path( file ), numFiles, NULL );
	for ( int32_t i = 0; i < numFiles; ++i ) {
		QmFsPackageFile *index = &package->files[ i ];
		int32_t nameLength = qm_fs_file_read_int32( file, false, NULL );
		if ( nameLength >= sizeof( index->name ) || nameLength <= 0 ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "invalid index name length, %d", i );
			PlDestroyPackage( package );
			return NULL;
		}

		if ( qm_file_read( file, index->name, sizeof( char ), nameLength ) != nameLength ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "failed to read in file name for index %d", i );
			PlDestroyPackage( package );
			return NULL;
		}

		bool status;
		index->offset = qm_fs_file_read_int32( file, false, &status );
		index->compressedSize = qm_fs_file_read_int32( file, false, &status );
		index->size = qm_fs_file_read_int32( file, false, &status );
		index->compressionType = PL_COMPRESSION_IMPLODE;
	}

	return package;
}
