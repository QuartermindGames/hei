// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "blitz.h"

static PLPluginDescription pluginDesc = {
        .description = "Blitz file format support.",
        .pluginVersion = { 0, 0, 1 },
        .interfaceVersion = { PL_PLUGIN_INTERFACE_VERSION_MAJOR, PL_PLUGIN_INTERFACE_VERSION_MINOR }
};
PL_EXPORT const PLPluginDescription *PLQueryPlugin( void ) {
	return &pluginDesc;
}

/***********************************************************/

static unsigned int hash_string( const char *string ) {
	static const unsigned int crcTable[] = {
#include "blitz_hash_table.h"
	};

	unsigned int crc = 0;
	char c;
	while ( ( c = *string ) != '\0' ) {
		crc = crc << 8 ^ crcTable[ crc >> 24 ^ ( unsigned int ) c ];
		string++;
	}
	return crc;
}

const char *titanStrings[] = {
#include "blitz_titan_files.h"
};
const unsigned int numTitanStrings = PL_ARRAY_ELEMENTS( titanStrings );

const char *get_string_for_hash( uint32_t hash, const char **strings, unsigned int numStrings ) {
	if ( strings == NULL ) {
		return NULL;
	}

	for ( unsigned int i = 0; i < numStrings; ++i ) {
		unsigned int sHash = hash_string( strings[ i ] );
		if ( sHash != hash ) {
			continue;
		}

		return strings[ i ];
	}

	return NULL;
}

PLImage *Blitz_SPT_ParseImage( PLFile *file );
void Blitz_SPT_BulkExport( PLFile *file, const char *destination, const char *format );
PLPackage *Blitz_DAT_LoadPackage( const char *path );

static void dump_spt_file( const char *path ) {
	PLFile *file = gInterface->OpenFile( path, false );
	if ( file == NULL ) {
		printf( "Failed to open file: %s\n", gInterface->GetError() );
		return;
	}

	/* gross - but attempt to load the first index just to verify it */
	PLImage *image = Blitz_SPT_ParseImage( file );
	if ( image == NULL ) {
		printf( "Probably not an SPT file: %s\n", path );
		gInterface->CloseFile( file );
		return;
	}
	gInterface->DestroyImage( image );

	const char *fileName = strrchr( path, '/' );
	if ( fileName == NULL ) {
		fileName = path;
	}

	char tmp[ 64 ];
	snprintf( tmp, sizeof( tmp ), "%s", fileName );
	const char *extension = strrchr( path + 1, '.' );
	if ( extension != NULL ) {
		tmp[ strlen( tmp ) - 4 ] = '\0';
		gInterface->strntolower( tmp, sizeof( tmp ) );
	}

	char outPath[ 64 ];
	snprintf( outPath, sizeof( outPath ), "./dump_spts/%s", tmp );
	if ( !gInterface->CreatePath( outPath ) ) {
		printf( "Failed to create destination folder: %s\n", gInterface->GetError() );
		gInterface->CloseFile( file );
		return;
	}

	gInterface->RewindFile( file );

	Blitz_SPT_BulkExport( file, outPath, "tga" );

	gInterface->CloseFile( file );
}

static void command_blitz_spt_dump( unsigned int argc, char **argv ) {
	dump_spt_file( argv[ 1 ] );
}

/***********************************************************/
/** command_blitz_spt_dump_all **/

static void dump_spt_callback( const char *path, void *user ) {
	const char *extension = strrchr( path + 1, '.' );
	if ( extension != NULL ) {
		if ( gInterface->strcasecmp( extension, ".spt" ) != 0 ) {
			return;
		}

		dump_spt_file( path );
		return;
	}

	/* no extension, need to do some horrible nasty crap... */
	dump_spt_file( path );
}

static void command_blitz_spt_dump_all( unsigned int argc, char **argv ) {
	const char *path = argv[ 1 ];
	gInterface->ScanDirectory( path, NULL, dump_spt_callback, true, NULL );
}

/***********************************************************/

static void index_file_callback( const char *path, void *user ) {
	PLFile *in = gInterface->OpenFile( path, false );
	if ( in == NULL ) {
		printf( "Failed to open file: %s\n", gInterface->GetError() );
		return;
	}

	static const int32_t PSI_MAGIC = PL_MAGIC_TO_NUM( 'P', 'S', 'I', '\0' );
	int32_t magic = gInterface->ReadInt32( in, false, NULL );
	if ( magic != PSI_MAGIC ) {
		return;
	}

	printf( "Found PSI: %s\n", path );

	gInterface->FileSeek( in, 72, PL_SEEK_SET );
	int32_t numTextures = gInterface->ReadInt32( in, false, NULL );
	int32_t textureOffset = gInterface->ReadInt32( in, false, NULL );

	gInterface->FileSeek( in, textureOffset, PL_SEEK_SET );

	FILE *out = ( FILE * ) user;
	for ( int i = 0; i < numTextures; ++i ) {
		char name[ 32 ];
		gInterface->ReadFile( in, name, sizeof( char ), 32 );
		fprintf( out, "%s\n", name );
	}

	gInterface->CloseFile( in );
}

static void command_blitz_dump_strings( unsigned int argc, char **argv ) {
	FILE *file = fopen( "./blitz_strings.txt", "w" );
	if ( file == NULL ) {
		printf( "Failed to open for writing!\n" );
		return;
	}

	gInterface->ScanDirectory( argv[ 1 ], NULL, index_file_callback, true, file );
}

/***********************************************************/
/** Convert RAW images **/

static PLImage *RawToImage( const uint16_t *buf, size_t size ) {
	// for now, I can only assume they're all this size...
	static const unsigned int RAW_WIDTH = 512;
	static const unsigned int RAW_HEIGHT = 256;
	static const unsigned int RAW_SIZE = RAW_WIDTH * RAW_HEIGHT * 2;
	if ( size != RAW_SIZE ) {
		printf( "Unexpected raw image size (%zu != %u)!\n", size, RAW_SIZE );
		return NULL;
	}

	PLImage *image = gInterface->CreateImage( NULL, RAW_WIDTH, RAW_HEIGHT, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );
	if ( image == NULL ) {
		printf( "Failed to create raw image container: %s\n", gInterface->GetError() );
		return NULL;
	}

	const unsigned int shiftR = 0;
	const unsigned int shiftG = 5;
	const unsigned int shiftB = 10;

	PLColour *colour = ( PLColour * ) &image->data[ 0 ][ 0 ];
	for ( unsigned int i = 0; i < size / 2; ++i ) {
		uint16_t c = htole16( buf[ i ] );
		colour->r = ( ( c & ( 31 << shiftR ) ) >> shiftR ) << 3;
		colour->g = ( ( c & ( 31 << shiftG ) ) >> shiftG ) << 3;
		colour->b = ( ( c & ( 31 << shiftB ) ) >> shiftB ) << 3;
		colour->r |= colour->r >> 5;
		colour->g |= colour->g >> 5;
		colour->b |= colour->b >> 5;
		colour->a = 255;
		colour++;
	}

	return image;
}

static void ConvertRawImage( const char *path ) {
	PLFile *file = gInterface->OpenFile( path, true );
	if ( file == NULL ) {
		printf( "Failed to open raw image (%s): %s\n", path, gInterface->GetError() );
		return;
	}

	size_t size = gInterface->GetFileSize( file );
	const uint16_t *buf = gInterface->GetFileData( file );
	PLImage *out = RawToImage( buf, size );
	if ( out != NULL ) {
		const char *filename = strrchr( path, '/' );
		if ( filename == NULL ) {
			filename = path;
		} else {
			filename = filename + 1;
		}

		PLPath writePath;
		gInterface->SetupPath( writePath, true, "%s.png", filename );
		gInterface->WriteImage( out, writePath );
	}
	gInterface->CloseFile( file );
}

static void ConvertRawCommand( PL_UNUSED unsigned int argc, char **argv ) {
	ConvertRawImage( argv[ 1 ] );
}

static void BulkConvertCallback( const char *path, PL_UNUSED void *user ) {
	ConvertRawImage( path );
}

static void BulkConvertRawCommand( PL_UNUSED unsigned int argc, PL_UNUSED char **argv ) {
	gInterface->ScanDirectory( argv[ 1 ], "raw", BulkConvertCallback, true, NULL );
}

const PLPluginExportTable *gInterface = NULL;
PL_EXPORT void PLInitializePlugin( const PLPluginExportTable *functionTable ) {
	gInterface = functionTable;

	gInterface->RegisterImageLoader( "spt", Blitz_SPT_ParseImage );
	gInterface->RegisterPackageLoader( "dat", Blitz_DAT_LoadPackage );

	gInterface->RegisterConsoleCommand( "blitz_spt_dump",
	                                    "Dump images contained within an SPT file. 'blitz_spt_dump ./path [./outpath]'",
	                                    -1, command_blitz_spt_dump );
	gInterface->RegisterConsoleCommand( "blitz_spt_dump_all",
	                                    "Dump all possible SPT files in specified directory.",
	                                    1, command_blitz_spt_dump_all );
	gInterface->RegisterConsoleCommand( "blitz_dump_strings",
	                                    "Scan through and dump all identifiable strings from PSI files. "
	                                    "You'll need to specify the input path.",
	                                    1, command_blitz_dump_strings );

	gInterface->RegisterConsoleCommand( "blitz_convert_raw",
	                                    "Convert .raw images.",
	                                    1, ConvertRawCommand );
	gInterface->RegisterConsoleCommand( "blitz_convert_raw_bulk",
	                                    "Converts all .raw images in a given directory.",
	                                    1, BulkConvertRawCommand );

	void Blitz_Format_Psi_ConvertModelCommand( unsigned int argc, char **argv );
	gInterface->RegisterConsoleCommand( "blitz_convert_model",
	                                    "Converts the given model to SMD/OBJ.",
	                                    1, Blitz_Format_Psi_ConvertModelCommand );
}
