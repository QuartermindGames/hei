// SPDX-License-Identifier: MIT
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_package.h>

// WAD format as used by The Mark of Kri

PLPackage *PlParseBREWadPackage( PLFile *file ) {
	uint32_t numEntries = PlReadInt32( file, false, NULL );
	if ( numEntries == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "no entries in table" );
		return NULL;
	}

	typedef struct Index {
		uint32_t hash;
		uint32_t size;
		uint32_t unused;
		uint32_t unknown;
	} Index;
	PL_STATIC_ASSERT( sizeof( Index ) == 16, "Invalid index size!" );

	size_t tableSize = sizeof( Index ) * numEntries;
	size_t fileSize = PlGetFileSize( file );
	if ( tableSize >= fileSize ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected table size (%u >= %u)", tableSize, fileSize );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numEntries, NULL );
	for ( unsigned int i = 0; i < numEntries; ++i ) {
		// for now, just convert the hash to a string and use that as a name
		uint32_t hash = ( uint32_t ) PlReadInt32( file, false, NULL );
		pl_itoa( hash, package->table[ i ].fileName, sizeof( package->table[ i ].fileName ), 10 );

		package->table[ i ].fileSize = ( uint32_t ) PlReadInt32( file, false, NULL );
		PlReadInt32( file, false, NULL );// unused
		PlReadInt32( file, false, NULL );// unknown
		package->table[ i ].offset = ( i == 0 ) ? 2048 : package->table[ i - 1 ].offset + package->table[ i - 1 ].fileSize;
	}

	return package;
}
