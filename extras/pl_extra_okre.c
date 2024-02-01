// SPDX-License-Identifier: MIT
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include <plcore/pl_package.h>
#include "plcore/pl_image.h"

/////////////////////////////////////////////////////////////////////////////////////
// WAD Loader
/////////////////////////////////////////////////////////////////////////////////////

// Package format used by Argonaut's SWAT Global Strike (Xbox)

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

		if ( *buf == '\0' ) {
			PlReportErrorF( PL_RESULT_FILETYPE, "unexpected string" );
			break;
		}

		const char *c = strrchr( PlGetFilePath( file ), '/' ) + 1;
		if ( pl_strncasecmp( c, buf, strlen( buf ) ) != 0 ) {
			hasWad = true;
			break;
		}
	}

	// Go back to the point we started at
	PlFileSeek( file, origin, PL_SEEK_SET );

	return hasWad;
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
	WadEntry *entries = PL_NEW_( WadEntry, numFiles );
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

	PL_DELETE( entries );

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

	uint8_t *buf = PL_NEW_( uint8_t, header.dataSize );
	if ( PlReadFile( file, buf, sizeof( uint8_t ), header.dataSize ) != header.dataSize ) {
		PL_DELETE( buf );
		return NULL;
	}

	PLImage *image = NULL;
	if ( header.format == 3 ) {
		//TODO: looks like a grayscale image with palette at end??
	} else {// DXT1/DXT5
		uint8_t *dbuf = PL_NEW_( uint8_t, header.width * header.height * 4 );
		if ( header.format == 0 ) {
			PlBlockDecompressImageDXT1( header.width, header.height, buf, dbuf );
		} else {
			PlBlockDecompressImageDXT5( header.width, header.height, buf, dbuf );
		}
		image = PlCreateImage( dbuf, header.width, header.height, 0, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );
		PL_DELETE( dbuf );
	}

	PL_DELETE( buf );

	return image;
}
