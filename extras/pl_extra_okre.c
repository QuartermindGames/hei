// SPDX-License-Identifier: MIT
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "qmos/public/qm_os_memory.h"

#include <plcore/pl_package.h>
#include "plcore/pl_image.h"

// Package format used by Argonaut's SWAT Global Strike

/////////////////////////////////////////////////////////////////////////////////////
// WAD Loader
/////////////////////////////////////////////////////////////////////////////////////

static bool ValidatePackage( PLFile *file ) {
	// There is no magic or easy identification,
	// so we need to do some really dumb checks...

	PLFileOffset origin = PlGetFileOffset( file );

	uint32_t numFiles = PL_READUINT32( file, false, NULL );
	if ( numFiles == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected number of files" );
		return false;
	}

	// This will probably be the most damning,
	// iterate over the table until we hit the end
	bool hasWad = false;
	while ( !PlIsEndOfFile( file ) ) {
		char buf[ 512 ];
		PL_ZERO_( buf );

		unsigned short stringLength = PL_READUINT16( file, false, NULL );
		if ( stringLength == 0 || stringLength >= sizeof( buf ) ) {
			PlReportErrorF( PL_RESULT_FILETYPE, "unexpected initial name size" );
			break;
		}

		if ( PlReadFile( file, buf, sizeof( char ), stringLength ) != stringLength ) {
			break;
		}

		printf( "%s\n", buf );

		if ( *buf == '\0' ) {
			PlReportErrorF( PL_RESULT_FILETYPE, "unexpected string" );
			break;
		}

		// This check is dumb...
		const char *path = PlGetFilePath( file );
		const char *c = strrchr( path, '/' );
		if ( c == NULL ) {
			c = strrchr( path, '\\' );
			if ( c == NULL ) {
				c = strrchr( path, '.' );
			}
		}

		if ( c != NULL ) {
			hasWad = true;
			break;
		}
	}

	// Go back to the point we started at
	PlFileSeek( file, origin, PL_SEEK_SET );

	return hasWad;
}

/// Seem to be exclusive to the PlayStation 2 version, for whatever reason.
PLPackage *PlParseOkreDirPackage( PLFile *file ) {
	uint32_t numSubWads = PL_READUINT32( file, false, NULL );
	if ( numSubWads == 0 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected number of files" );
		return NULL;
	}

	typedef struct FIndex {
		char name[ 64 ];
		uint32_t length;
		uint32_t offset;
	} FIndex;
	static_assert( sizeof( FIndex ) == 72, "Invalid struct size!" );
	FIndex *subWads = QM_OS_MEMORY_NEW_( FIndex, numSubWads );
	if ( PlReadFile( file, subWads, sizeof( FIndex ), numSubWads ) != numSubWads ) {
		qm_os_memory_free( subWads );
		return NULL;
	}

	// The actual data is stored in an entirely different file...
	PLPath wadPath = {};
	const char *path = PlGetFilePath( file );
	strncpy( wadPath, path, strlen( path ) - 4 );
	strcat( wadPath, ".WAD" );

	PLPackage *package = PlCreatePackageHandle( wadPath, numSubWads, NULL );
	if ( package != NULL ) {
		for ( unsigned int i = 0; i < numSubWads; ++i ) {
			strcpy( package->table[ i ].fileName, subWads[ i ].name );
			package->table[ i ].offset = subWads[ i ].offset;
			package->table[ i ].fileSize = subWads[ i ].length;
		}
	}

	qm_os_memory_free( subWads );

	return package;
}

// 10-01.ent.wad
// RIFF offset is   42674176
// Header offset is 2190155776
//                  2147481600

// 2147928064

PLPackage *PlParseOkreWadPackage( PLFile *file ) {
	if ( !ValidatePackage( file ) ) {
		return NULL;
	}

	// Alright, now parse it for real!

	uint32_t numFiles = PL_READUINT32( file, false, NULL );

	typedef struct WadEntry {
		PLPath path;        // path
		uint32_t offset;    // position in wad
		uint32_t sourceSize;// size of the original file
	} WadEntry;
	WadEntry *entries = QM_OS_MEMORY_NEW_( WadEntry, numFiles );
	for ( uint32_t i = 0; i < numFiles; ++i ) {
#if 1

		// Read in the name and normalise it, for clean extraction
		PLPath path = {};
		uint16_t size = PL_READUINT16( file, false, NULL );
		PlReadFile( file, path, sizeof( char ), size );
		PlNormalizePath( path, sizeof( entries[ i ].path ) );

		char *c = strrchr( path, '=' );
		if ( c != NULL ) {
			*c = '\0';
		}

		if ( *path == '$' ) {
			c = &path[ 4 ];
		} else if ( *( path + 1 ) == ':' ) {
			c = &path[ 3 ];
		} else {
			c = path;
		}

		strcpy( entries[ i ].path, c );

#else// or just copy?

		uint16_t size = PL_READUINT16( file, false, NULL );
		PlReadFile( file, entries[ i ].path, sizeof( char ), size );

#endif

		// The offset within the WAD
		entries[ i ].offset = PL_READUINT32( file, false, NULL );

		// Seek to the start of the file
		PLFileOffset o = PlGetFileOffset( file );
		PlFileSeek( file, entries[ i ].offset, PL_SEEK_SET );
		entries[ i ].sourceSize = PL_READUINT32( file, false, NULL );
		entries[ i ].offset += 4;

		// Wind us back to where we were
		PlFileSeek( file, o, PL_SEEK_SET );
	}

	PLPackage *package = PlCreatePackageHandle( PlGetFilePath( file ), numFiles, NULL );
	if ( package != NULL ) {
		for ( unsigned int i = 0; i < package->table_size; ++i ) {
			uint32_t csize;
			if ( i == ( package->table_size - 1 ) ) {
				csize = PlGetFileSize( file ) - entries[ i ].offset;
			} else {
				csize = entries[ i + 1 ].offset - entries[ i ].offset;
			}

			if ( entries[ i ].sourceSize == 0 ) {
				entries[ i ].sourceSize = csize;
			}

			strcpy( package->table[ i ].fileName, entries[ i ].path );
			package->table[ i ].offset = entries[ i ].offset;
			package->table[ i ].compressedSize = csize;
			package->table[ i ].fileSize = entries[ i ].sourceSize;
			package->table[ i ].compressionType = ( entries[ i ].sourceSize != csize ) ? PL_COMPRESSION_DEFLATE : PL_COMPRESSION_NONE;
		}
	}

	qm_os_memory_free( entries );

	return package;
}

/////////////////////////////////////////////////////////////////////////////////////
// Texture Loader
/////////////////////////////////////////////////////////////////////////////////////

typedef struct OkreTextureHeader {
	uint32_t format;
	uint32_t width;
	uint32_t height;
	int32_t unk0;
	int32_t unk1;
	int32_t unk2;
	uint32_t dataSize;
} OkreTextureHeader;
static_assert( sizeof( OkreTextureHeader ) == 28, "Invalid struct size!" );

PLImage *PlParseOkreTexture( PLFile *file ) {
	OkreTextureHeader header;
	if ( PlReadFile( file, &header, sizeof( OkreTextureHeader ), 1 ) != 1 ) {
		return NULL;
	}

	// sanity checks ...
	if ( header.width >= 2048 || header.height >= 2048 ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "unexpected image size" );
		return NULL;
	} else if ( header.dataSize == 0 || header.dataSize > PlGetFileSize( file ) - sizeof( OkreTextureHeader ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid data size" );
		return NULL;
	}

	uint8_t *buf = QM_OS_MEMORY_NEW_( uint8_t, header.dataSize );
	if ( PlReadFile( file, buf, sizeof( uint8_t ), header.dataSize ) != header.dataSize ) {
		qm_os_memory_free( buf );
		return NULL;
	}

	PLImage *image = NULL;
	if ( header.format == 3 ) {
		//TODO: looks like a grayscale image with palette at end??
	} else {// DXT1/DXT5
		uint8_t *dbuf = QM_OS_MEMORY_NEW_( uint8_t, header.width * header.height * 4 );
		if ( header.format == 0 ) {
			PlBlockDecompressImageDXT1( header.width, header.height, buf, dbuf );
		} else {
			PlBlockDecompressImageDXT5( header.width, header.height, buf, dbuf );
		}
		image = PlCreateImage( dbuf, header.width, header.height, 0, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );
		qm_os_memory_free( dbuf );
	}

	qm_os_memory_free( buf );

	return image;
}
