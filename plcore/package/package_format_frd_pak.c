// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2025 Mark E Sowden <hogsy@oldtimes-software.com>

#include "package_private.h"

#include <plcore/pl_hashtable.h>
#include <plcore/pl_parse.h>

// Loader for Free Radical Design's PAK format.

#define PAK5_MAGIC PL_MAGIC_TO_NUM( 'P', '5', 'C', 'K' )

typedef struct Pak5Header {
	uint32_t magic;
	uint32_t tocOffset;
	uint32_t tocSize;
} Pak5Header;

typedef struct Pak5Index {
	uint32_t checksum;
	uint32_t offset;
	uint32_t size;
	uint32_t compressedSize;//?
} Pak5Index;

static PLHashTable *populate_name_table( const PLFile *file ) {
	PLPath path;
	PlSetupPath( path, true, PlGetFilePath( file ) );
	char *c = strrchr( path, '.' );
	if ( c == NULL ) {
		return NULL;
	}

	*c = '\0';
	if ( PlAppendPath( path, ".c2n", false ) == NULL ) {
		return NULL;
	}

	PLFile *nameFile = PlOpenFile( path, false );
	if ( nameFile == NULL ) {
		return NULL;
	}

	PLHashTable *nameTable = PlCreateHashTable();
	while ( true ) {
		char line[ 1024 ];
		const char *p = PlReadString( nameFile, line, sizeof( line ) );
		if ( p == NULL ) {
			break;
		}

		char token[ 256 ];

		// checksum
		if ( PlParseToken( &p, token, sizeof( token ) ) == NULL ) {
			break;
		}
		uint32_t checksum = strtoul( token, NULL, 16 );

		// name
		if ( PlParseToken( &p, token, sizeof( token ) ) == NULL ) {
			break;
		}

		char *name = PL_NEW_( char, strlen( token ) + 1 );
		strcpy( name, token );

		PlInsertHashTableNode( nameTable, &checksum, sizeof( checksum ), name );
	}

	return nameTable;
}

PLPackage *PlParseFrdPakPackage_( PLFile *file ) {
	Pak5Header header;
	if ( PlReadFile( file, &header, sizeof( Pak5Header ), 1 ) != 1 ) {
		return NULL;
	}

	if ( header.magic != PAK5_MAGIC ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid magic number" );
		return NULL;
	}

	if ( header.tocOffset == 0 || header.tocOffset >= PlGetFileSize( file ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid TOC offset" );
		return NULL;
	}

	if ( !PlFileSeek( file, header.tocOffset, SEEK_SET ) ) {
		return NULL;
	}

	unsigned int numFiles = header.tocSize / sizeof( Pak5Index );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid number of files" );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );

	// see if there's a checksum to name file
	PLHashTable *nameTable = populate_name_table( file );

	unsigned int i;
	for ( i = 0; i < numFiles; ++i ) {
		Pak5Index index;
		if ( PlReadFile( file, &index, sizeof( Pak5Index ), 1 ) != 1 ) {
			break;
		}
		if ( index.offset == 0 || index.offset >= PlGetFileSize( file ) ) {
			PlReportErrorF( PL_RESULT_FILETYPE, "invalid file (%u) offset (%u)", i, index.offset );
			break;
		}

		package->table[ i ].offset = index.offset;
		package->table[ i ].fileSize = index.size;

		if ( nameTable != NULL ) {
			const char *name = PlLookupHashTableUserData( nameTable, &index.checksum, sizeof( index.checksum ) );
			if ( name != NULL ) {
				snprintf( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ), "%s", name );
			}
		}
	}

	PlDestroyHashTableEx( nameTable, PlFree );

	if ( i != numFiles ) {
		PlDestroyPackage( package );
		package = NULL;
	}

	return package;
}
