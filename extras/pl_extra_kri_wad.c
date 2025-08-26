// SPDX-License-Identifier: MIT
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_package.h>

// WAD format as used by The Mark of Kri

static const unsigned int BLOCK_SIZE = 2048U;
static const unsigned int DIR_MAGIC = PL_MAGIC_TO_NUM( 'D', 'I', 'R', '0' );

typedef struct KRIIndex {
	int32_t hash;
	uint32_t size;
	int32_t unused;
	int32_t unknown;
} KRIIndex;
PL_STATIC_ASSERT( sizeof( KRIIndex ) == 16, "needs to be 16 bytes" );

static int HashFilename( const char *filename, unsigned int length ) {
	int hash = 0;
	for ( unsigned int i = 0; i < length; ++i ) {
		if ( filename[ i ] == '\0' ) {
			break;
		}

		hash = hash * 0x83 + toupper( filename[ i ] );
	}

	return hash;
}

static size_t CalculateStreamLength( size_t dataSize ) {
	return ceil( ( double ) dataSize / BLOCK_SIZE ) * BLOCK_SIZE;
}

typedef struct DirTable {
	char *stream;
	unsigned int *offsets;
	bool isInitialized;
} DirTable;

static DirTable *ParseDirTable( PLFile *file, unsigned int numEntries, DirTable *table ) {
	table->isInitialized = false;

	unsigned int magic = PL_READUINT32( file, false, NULL );
	if ( magic != DIR_MAGIC ) {
		return NULL;
	}

	unsigned int blockSize = PL_READUINT32( file, false, NULL );
	table->offsets = QM_OS_MEMORY_NEW_( unsigned int, ( numEntries - 1 ) );
	for ( unsigned int i = 0; i < ( numEntries - 1 ); ++i ) {
		table->offsets[ i ] = PL_READUINT32( file, false, NULL );
	}

	blockSize = ( blockSize - numEntries * 4 ) + 4;
	table->stream = QM_OS_MEMORY_NEW_( char, blockSize );
	PlReadFile( file, table->stream, sizeof( char ), blockSize );

	table->isInitialized = true;

	return table;
}

static void FreeDirTable( DirTable *table ) {
	if ( !table->isInitialized ) {
		return;
	}

	PL_DELETEN( table->offsets );
	PL_DELETEN( table->stream );
}

PLPackage *PlParseKriPackage( PLFile *file ) {
	unsigned int numEntries = PlReadInt32( file, false, NULL );
	if ( numEntries == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "no entries in table" );
		return NULL;
	}

	size_t tableSize = CalculateStreamLength( sizeof( KRIIndex ) * numEntries );
	if ( tableSize >= PlGetFileSize( file ) ) {
		PlReportErrorF( PL_RESULT_FILESIZE, "unexpected table size" );
		return NULL;
	}

	uint8_t *stream = QM_OS_MEMORY_NEW_( uint8_t, tableSize );
	PlReadFile( file, stream, sizeof( uint8_t ), tableSize );

	PLFileOffset offset = PlGetFileOffset( file );

	// Check if there's a directory listing we can use to ID the files
	DirTable dirTable;
	if ( ParseDirTable( file, numEntries, &dirTable ) ) {
		offset += ( CalculateStreamLength( ( ( KRIIndex * ) stream )->size ) - 4 );
		numEntries--;
	}

	KRIIndex *index = !dirTable.isInitialized ? &( ( KRIIndex * ) stream )[ 0 ] : &( ( KRIIndex * ) stream )[ 1 ];
	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numEntries, NULL );
	for ( unsigned int i = 0; i < numEntries; ++i, ++index ) {
		assert( index->hash != 0 );

		if ( dirTable.isInitialized ) {
			// Perform some basic validation...
			int hash = HashFilename( &dirTable.stream[ dirTable.offsets[ i ] ], strlen( &dirTable.stream[ dirTable.offsets[ i ] ] ) );
			assert( hash == index->hash );
			if ( hash != index->hash ) {
				PlReportErrorF( PL_RESULT_FILEERR, "hash mismatch in table, maybe hashing/dir format is different" );
				PlDestroyPackage( package );
				package = NULL;
				break;
			}

			snprintf( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ), "%s", &dirTable.stream[ dirTable.offsets[ i ] ] );
		} else {
			snprintf( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ), "%u", ( unsigned int ) index->hash );
		}

		if ( i > 0 ) {
			offset += CalculateStreamLength( package->table[ i - 1 ].fileSize );
		}

		package->table[ i ].offset = offset;
		package->table[ i ].fileSize = index->size;
	}

	FreeDirTable( &dirTable );

	qm_os_memory_free( stream );

	return package;
}
