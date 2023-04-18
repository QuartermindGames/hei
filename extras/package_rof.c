// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>
// Purpose: Loader for Oddworld's A.L.I.V.E. 2 ROF packages

#include <plcore/pl_package.h>

PLPackage *ROF_ParseFile( PLFile *file ) {
	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "unexpected file count for ROF" );
		return NULL;
	}

	size_t fileSize = PlGetFileSize( file );
	uint32_t nameTableSize = PL_READUINT32( file, false, NULL );
	if ( nameTableSize == 0 || nameTableSize >= fileSize ) {
		PlReportErrorF( PL_RESULT_FILEERR, "unexpected name table size for ROF" );
		return NULL;
	}

	typedef struct FileIndex {
		uint32_t offset;
		uint32_t size;
		uint32_t unused;// ?
		uint32_t nameSize;
		uint32_t nameOffset;
	} FileIndex;

	FileIndex *indices = PL_NEW_( FileIndex, numFiles );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		indices[ i ].offset = PL_READUINT32( file, false, NULL );
		indices[ i ].size = PL_READUINT32( file, false, NULL );
		indices[ i ].unused = PL_READUINT32( file, false, NULL );
		indices[ i ].nameSize = PL_READUINT32( file, false, NULL );
		indices[ i ].nameOffset = PL_READUINT32( file, false, NULL );
	}

	char *nameBuffer = PL_NEW_( char, nameTableSize );
	PlReadFile( file, nameBuffer, sizeof( char ), nameTableSize );

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	for ( unsigned int i = 0; i < package->table_size; ++i ) {
		// sort out the name, which we just need to yank out of the name buffer

		strncpy_s( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ), &nameBuffer[ indices[ i ].nameOffset ], indices[ i ].nameSize );
		PlNormalizePath( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ) );

		/* each file here uses deflate compression, but that information
		 * isn't provided in the TOC for whatever reason but at the file
		 * header instead! so we'll jump to the header, parse it in and
		 * then store the offset post-header :) */

		PlFileSeek( file, indices[ i ].offset, PL_SEEK_SET );

		static const unsigned int magic = PL_MAGIC_TO_NUM( 'd', 'e', 'f', 'T' );
		unsigned int rm = PL_READUINT32( file, false, NULL );
		if ( rm == magic ) {
			// probably compressed, or we'll assume it is anyway!
			PlFileSeek( file, 4, PL_SEEK_CUR );// no idea...always the same

			package->table[ i ].fileSize = PL_READUINT32( file, false, NULL );
			package->table[ i ].compressedSize = PL_READUINT32( file, false, NULL );
			package->table[ i ].compressionType = PL_COMPRESSION_DEFLATE;
			package->table[ i ].offset = PlGetFileOffset( file );
		} else {
			// not compressed - not seen anything like this, but eh!
			package->table[ i ].fileSize = indices[ i ].size;
			package->table[ i ].offset = indices[ i ].offset;
		}
	}

	PL_DELETE( nameBuffer );
	PL_DELETE( indices );

	return package;
}
