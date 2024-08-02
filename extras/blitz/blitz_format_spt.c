// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#include "blitz.h"

/* Blitz, SPT format
 *
 * Usages:
 * 	- Titan A.E. (Cancelled, PSX)
 *
 * Notes:
 * 	An SPT can hold a collection of textures, rather than just 1.
 * 	Additionally, these use a 16-bit palette of 16 colours?
 */

typedef enum SPTFormat {
	SPT_FORMAT_4BPP,
	SPT_FORMAT_8BPP,

	MAX_SPT_FORMATS
} SPTFormat;

typedef enum SPTFlags {
	SPT_FLAGS_END = 1, // indicates that it's the last image
	SPT_FLAGS_8BPP = 2,// if this flag isn't set, it seems to be 4bpp
	SPT_FLAGS_ALPHA = 16,
} SPTFlags;

PL_PACKED_STRUCT_START( SPTHeader )
uint32_t imageOffset;
uint32_t paletteOffset;
uint8_t width;
uint8_t height;
int32_t unknown;
uint16_t flags;
uint32_t hash;
PL_PACKED_STRUCT_END( SPTHeader )
PL_STATIC_ASSERT( sizeof( SPTHeader ) == 20, "invalid struct size" );

static bool parse_header( PLFile *file, SPTHeader *out ) {
	PL_ZERO( out, sizeof( SPTHeader ) );

	out->imageOffset = ( uint32_t ) gInterface->ReadInt32( file, false, NULL );
	if ( out->imageOffset == 0 || out->imageOffset >= gInterface->GetFileSize( file ) ) {
		gInterface->ReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid image offset" );
		return false;
	}

	out->paletteOffset = ( uint32_t ) gInterface->ReadInt32( file, false, NULL );
	if ( out->paletteOffset == 0 || out->paletteOffset >= out->imageOffset ) {
		gInterface->ReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid palette offset" );
		return false;
	}

	out->width = ( uint8_t ) gInterface->ReadInt8( file, NULL );
	if ( out->width == 0 ) {
		gInterface->ReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid width" );
		return false;
	}

	out->height = ( uint8_t ) gInterface->ReadInt8( file, NULL );
	if ( out->height == 0 ) {
		gInterface->ReportError( PL_RESULT_FILEERR, PL_FUNCTION, "invalid height" );
		return false;
	}

	gInterface->ReadInt32( file, false, NULL );

	out->flags = ( uint16_t ) gInterface->ReadInt16( file, false, NULL );
	out->hash = ( uint32_t ) gInterface->ReadInt32( file, false, NULL );

	return true;
}

static PLImage *spt_to_image( const SPTHeader *header, const uint16_t *pal, const uint8_t *img, unsigned int size, SPTFormat format ) {
	PLImage *image = gInterface->CreateImage( NULL, header->width, header->height, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );
	if ( image == NULL ) {
		return NULL;
	}

#define AAA5( x ) ( ( ( x ) << 3 ) | ( ( x ) >> 1 ) )
#define AAA6( x ) ( ( ( x ) << 2 ) | ( ( x ) >> 4 ) )
	PLColour *pColour = ( PLColour * ) &image->data[ 0 ][ 0 ];
	if ( format == SPT_FORMAT_8BPP ) {
		for ( unsigned int i = 0; i < size; ++i ) {
			uint16_t c = pal[ img[ i ] & 0x0F ];
			pColour->r = AAA5( c & 0x1F );
			pColour->g = AAA6( ( c >> 4 ) & 0x3E );
			pColour->b = AAA5( c >> 10 );
			pColour->a = 255;
			++pColour;
		}
	} else {
		for ( unsigned int i = 0; i < size; ++i ) {
			uint16_t c = pal[ img[ i ] & 0x0F ];
			pColour->r = AAA5( c & 0x1F );
			pColour->g = AAA6( ( c >> 4 ) & 0x3E );
			pColour->b = AAA5( c >> 10 );
			pColour->a = 255;
			++pColour;

			c = pal[ img[ i ] >> 4 ];
			pColour->r = AAA5( c & 0x1F );
			pColour->g = AAA6( ( c >> 4 ) & 0x3E );
			pColour->b = AAA5( c >> 10 );
			pColour->a = 255;
			++pColour;
		}
	}

	return image;
}

static PLImage *ParseImage( PLFile *file, const SPTHeader *header ) {
	/* fetch image data */
	if ( !gInterface->FileSeek( file, header->imageOffset, PL_SEEK_SET ) ) {
		return NULL;
	}

	unsigned int size = gInterface->ReadInt32( file, false, NULL );
	if ( size == 0 || size < ( header->width * header->height ) ) {
		gInterface->ReportError( PL_RESULT_FILEREAD, PL_FUNCTION, "invalid size for index" );
		return NULL;
	}

	SPTFormat format = ( header->flags & SPT_FLAGS_8BPP ) ? SPT_FORMAT_8BPP : SPT_FORMAT_4BPP;
	//assert( format != SPT_FORMAT_8BPP );
	if ( format == SPT_FORMAT_4BPP ) {
		size /= 2;
	}

	uint8_t *img = gInterface->MAlloc( size, true );
	if ( gInterface->ReadFile( file, img, sizeof( uint8_t ), size ) != size ) {
		return NULL;
	}

	/* fetch palette */
	uint16_t pal[ 16 ];
	if ( !gInterface->FileSeek( file, header->paletteOffset, PL_SEEK_SET ) ) {
		gInterface->Free( img );
		return NULL;
	}
	if ( gInterface->ReadFile( file, pal, sizeof( uint16_t ), 16 ) != 16 ) {
		gInterface->Free( img );
		return NULL;
	}

	PLImage *image = spt_to_image( header, pal, img, size, format );

	gInterface->Free( img );

	return image;
}

PLImage *Blitz_SPT_ParseImage( PLFile *file ) {
	SPTHeader header;
	if ( !parse_header( file, &header ) ) {
		return NULL;
	}

	return ParseImage( file, &header );
}

void Blitz_SPT_BulkExport( PLFile *file, const char *destination, const char *format ) {
	/* originally went with this method, but as great as it works, there are still
	 * cases where SPTs contain textures that the PSI doesn't actually use/reference,
	 * so instead we'll just use our giant global list - there are of course also
	 * cases where we still don't have the PSI file matched up, so that will be missing... */
#if 0
	strcpy( path, gInterface->GetFilePath( file ) );

	unsigned int numStrings = 0;
	char **strings = NULL;

	/* check to see if there's a psi file we can pull names from */
	path[ strlen( path ) - 3 ] = '\0';
	strcat( path, "PSI" );
	PLFile *referenceFile;
	if ( ( referenceFile = gInterface->OpenFile( path, false ) ) != NULL ) {
		gInterface->FileSeek( referenceFile, 72, PL_SEEK_SET );

		numStrings = gInterface->ReadInt32( referenceFile, false, NULL );
		strings = gInterface->MAlloc( sizeof( char * ) * numStrings, true );

		int32_t textureOffset = gInterface->ReadInt32( referenceFile, false, NULL );
		gInterface->FileSeek( referenceFile, textureOffset, PL_SEEK_SET );

		for ( int i = 0; i < numStrings; ++i ) {
			strings[ i ] = gInterface->MAlloc( sizeof( char ) * ( 32 + 1 ), true );
			gInterface->ReadFile( referenceFile, strings[ i ], sizeof( char ), 32 );
		}

		gInterface->CloseFile( referenceFile );
	}
#endif

	SPTHeader header;
	do {
		if ( !parse_header( file, &header ) ) {
			break;
		}

		/* save this so we can restore our position after */
		PLFileOffset offset = gInterface->GetFileOffset( file );

		PLImage *image = ParseImage( file, &header );
		if ( image != NULL ) {
			PLPath path;
			const char *fileName = get_string_for_hash( header.hash, titanStrings, numTitanStrings );
			if ( fileName != NULL ) {
				char tmp[ 64 ];
				snprintf( tmp, sizeof( tmp ), "%s", fileName );
				gInterface->strntolower( tmp, sizeof( tmp ) );
				snprintf( path, sizeof( path ), "%s/%s.%s", destination, tmp, format );
			} else {
				snprintf( path, sizeof( path ), "%s/%X.%s", destination, header.hash, format );
			}
			gInterface->WriteImage( image, path );
			gInterface->DestroyImage( image );
		}

		gInterface->FileSeek( file, offset, PL_SEEK_SET );
	} while ( !( header.flags & SPT_FLAGS_END ) );

#if 0
	if ( strings != NULL ) {
		for ( unsigned int i = 0; i < numStrings; ++i ) {
			gInterface->Free( strings[ i ] );
		}
		gInterface->Free( strings );
	}
#endif
}
