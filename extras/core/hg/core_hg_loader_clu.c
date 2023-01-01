/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "../core.h"

/* Core Design, CLU (cluster) format
 *
 * Usages:
 * 	- Herdy Gerdy (2002, PS2)
 *
 * Notes:
 * 	Levels use the original level name as the description,
 * 	while the stored file uses a hashed name instead, hence
 * 	why the CLU names for levels appear somewhat random.
 *
 * 	Table of contents is sorted by hash, hence why the last
 * 	32-bit value increments (was pretty confusing at first!)
 * 	This is also why the table of contents doesn't match with
 * 	the order of the data.
 */

#define CLU_MAGIC   PL_MAGIC_TO_NUM( 'C', 'L', 'U', '\0' )
#define CLU_VERSION 2

typedef struct CLUHeader {
	uint32_t magic;   /* 'CLU ' */
	uint32_t version; /* always appears to be 2 - changed to 1 in later version */
	uint32_t headerLength;
	uint32_t numIndices;
	uint32_t hash;
	uint32_t unused;
	char description[ 60 ];
} CLUHeader;
PL_STATIC_ASSERT( sizeof( CLUHeader ) == 84U, "Invalid struct size!" );

typedef struct CLUIndex {
	uint32_t nameOffset; /* always resolves to 4 bytes before data, because it's unused */
	uint32_t size;
	uint32_t offset;
	uint32_t hash;
} CLUIndex;
PL_STATIC_ASSERT( sizeof( CLUIndex ) == 16U, "Invalid struct size!" );

/**
 * Seems to provide the correct result as far as I've been
 * able to determine...
 */
static uint32_t GenerateCoreHash( const char *string ) {
	uint32_t hash = 0;
	char c;
	while ( ( c = *string ) != '\0' ) {
		hash += ( hash << 7 ) + ( hash << 1 ) + ( uint32_t ) c;
		string++;
	}

	return hash;
}

static const char *GetStringForHash( uint32_t hash ) {
	static const char *sampleStringTable[] = {
#include "core_hg_files.h"
	};

	for ( unsigned int i = 0; i < PL_ARRAY_ELEMENTS( sampleStringTable ); ++i ) {
		uint32_t sHash = GenerateCoreHash( sampleStringTable[ i ] );
		if ( sHash != hash ) {
			continue;
		}

		return sampleStringTable[ i ];
	}

	return NULL;
}

static const char *IdentifyFileType( PLFile *file ) {
	uint32_t magic = ( uint32_t ) gInterface->ReadInt32( file, false, NULL );

	/* first check if it's a bitmap... */
	if ( ( ( uint16_t ) magic ) == 0x4d42 ) {
		return ".bmp";
	}

	/* then geo */
	if ( magic == PL_MAGIC_TO_NUM( 'H', 'E', 'D', ' ' ) ) {
		return ".hgm"; /* referenced as '.hed', but actually stored as '.hgm' */
	}

	if ( magic == PL_MAGIC_TO_NUM( 'C', 'L', 'U', '\0' ) ) {
		return ".clu";
	}

	/* and finally, check if it's a texture */
	if ( magic == PL_MAGIC_TO_NUM( 'T', 'X', 'T', 'R' ) ) {
		return ".hgt";
	} else {
		/* more typically, they'll have some crap at the start followed by 'TXTR' */
		magic = gInterface->ReadInt32( file, false, NULL );
		if ( magic == PL_MAGIC_TO_NUM( 'T', 'X', 'T', 'R' ) ) {
			return ".hgt";
		}
	}

	/* try to determine if it's a script (ugly) */
	char tmp[ 128 ];
	gInterface->ReadFile( file, tmp, sizeof( char ), sizeof( tmp ) );
	for ( unsigned int i = 0; i < sizeof( tmp ); ++i ) {
		if ( !isascii( tmp[ i ] ) ) {
			tmp[ 0 ] = '\0';
			break;
		}
	}
	if ( tmp[ 0 ] != '\0' ) {
		return ".txt";
	}

	return "";
}

static PLPackage *ParseCLUFile( PLFile *file ) {
	CLUHeader header;
	gInterface->ReadFile( file, &header, sizeof( CLUHeader ), 1 );

	if ( header.magic != CLU_MAGIC ) {
		gInterface->ReportError( PL_RESULT_FILETYPE, PL_FUNCTION, "invalid magic" );
		return NULL;
	}

	if ( header.version != CLU_VERSION ) {
		gInterface->ReportError( PL_RESULT_FILEVERSION, PL_FUNCTION, "unexpected version" );
		return NULL;
	}

	if ( header.headerLength >= gInterface->GetFileSize( file ) ) {
		gInterface->ReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid data offset provided in header" );
		return NULL;
	}

	size_t tableSize = ( sizeof( CLUIndex ) * header.numIndices );
	if ( tableSize >= header.headerLength ) {
		gInterface->ReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid relative table size to data offset" );
		return NULL;
	}

	CLUIndex *indicies = gInterface->MAlloc( tableSize, true );
	if ( gInterface->ReadFile( file, indicies, sizeof( CLUIndex ), header.numIndices ) != header.numIndices ) {
		gInterface->Free( indicies );
		return NULL;
	}

	unsigned int numMatches = 0;
	PLPackage *package = gInterface->CreatePackageHandle( gInterface->GetFilePath( file ), header.numIndices, NULL );
	for ( unsigned int i = 0; i < header.numIndices; ++i ) {
		package->table[ i ].fileSize = indicies[ i ].size;
		package->table[ i ].offset = indicies[ i ].offset;

		const char *fileName = GetStringForHash( indicies[ i ].hash );
		if ( fileName == NULL ) {
			/* let's add an extension based on a guessed type */
			const char *extension = "";
			PLFileOffset oldOffset = gInterface->GetFileOffset( file );
			if ( gInterface->FileSeek( file, package->table[ i ].offset, PL_SEEK_SET ) ) {
				extension = IdentifyFileType( file );
			}
			gInterface->FileSeek( file, oldOffset, PL_SEEK_SET );

			snprintf( package->table[ i ].fileName, sizeof( PLPath ), "%X%s", indicies[ i ].hash, extension );
		} else {
			snprintf( package->table[ i ].fileName, sizeof( PLPath ), "%s", fileName );
			numMatches++;
		}
	}

	gInterface->Free( indicies );

#if !defined( NDEBUG )
	const char *filePath = gInterface->GetFilePath( file );
	const char *fileName = strrchr( filePath, '/' );
	if ( fileName == NULL ) {
		fileName = filePath;
	}
	printf( "%.2lf%% matched (%u/%u) for CLU package \"%s\"\n", ( ( double ) numMatches / header.numIndices ) * 100.0,
	        numMatches, header.numIndices,
	        fileName );
#endif

	return package;
}

PLPackage *Core_CLU_LoadPackage( const char *path ) {
	PLFile *file = gInterface->OpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLPackage *package = ParseCLUFile( file );

	gInterface->CloseFile( file );

	return package;
}
