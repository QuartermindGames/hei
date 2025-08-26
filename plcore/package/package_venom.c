// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2025 Mark E Sowden <hogsy@oldtimes-software.com>
// Purpose: Venom PAK loader

#include "package_private.h"
#include "plcore/pl_parse.h"
#include "qmos/public/qm_os_memory.h"

// big-endian

static const char VENOM_PAK_IDENTIFIER[] = "VENOMBINPAK1.0\n";

PLPackage *PlParseVenomPakPackage( PLFile *file ) {
	char ident[ 15 ];
	PlReadFile( file, ident, sizeof( char ), sizeof( ident ) );
	if ( strncmp( VENOM_PAK_IDENTIFIER, ident, sizeof( ident ) ) != 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "file is not a valid venom package" );
		return NULL;
	}

	uint32_t headerSize = PL_READUINT32( file, true, NULL );
	if ( headerSize == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid header size (%u)", headerSize );
		return NULL;
	}

	uint32_t numFiles = PL_READUINT32( file, true, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid number of files" );
		return NULL;
	}

	PlReadInt32( file, true, NULL );// unk0

	size_t bufSize = headerSize - PlGetFileOffset( file );
	char *buf = QM_OS_MEMORY_NEW_( char, bufSize + 1 );
	if ( PlReadFile( file, buf, sizeof( char ), bufSize ) != bufSize ) {
		qm_os_memory_free( buf );
		return NULL;
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );

	const char *p = buf;
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		if ( *p == '\0' ) {
			break;
		}

		char line[ 512 ] = {};

		// name
		if ( PlParseLine( &p, line, sizeof( line ) ) == NULL ) {
			PlReportErrorF( PL_RESULT_FILEREAD, "failed to pull name %u", i );
			break;
		}
		PlSetupPath( package->table[ i ].fileName, true, "%s", ( *line == 'd' && *( line + 1 ) == ':' ) ? line + 3 : line );

		// filesize
		if ( PlParseLine( &p, line, sizeof( line ) ) == NULL ) {
			break;
		}
		package->table[ i ].fileSize = strtoul( line, NULL, 10 );

		// offset
		if ( PlParseLine( &p, line, sizeof( line ) ) == NULL ) {
			break;
		}
		package->table[ i ].offset = headerSize + ( PLFileOffset ) strtoul( line, NULL, 10 );

		// unknown...
		if ( PlParseLine( &p, line, sizeof( line ) ) == NULL ) {
			break;
		}
	}

	qm_os_memory_free( buf );

	return package;
}
