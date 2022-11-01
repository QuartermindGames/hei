/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl_image.h>

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
	SPT_FLAGS_END = 1,
	SPT_FLAGS_8BPP = 2,
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

static bool ParseHeader( PLFile *file, SPTHeader *out ) {
	PL_ZERO( out, sizeof( SPTHeader ) );

	out->imageOffset = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( out->imageOffset == 0 || out->imageOffset >= PlGetFileSize( file ) ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid image offset" );
		return false;
	}

	out->paletteOffset = ( uint32_t ) PlReadInt32( file, false, NULL );
	if ( out->paletteOffset == 0 || out->paletteOffset >= out->imageOffset ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid palette offset" );
		return false;
	}

	out->width = ( uint8_t ) PlReadInt8( file, NULL );
	if ( out->width == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid width" );
		return false;
	}

	out->height = ( uint8_t ) PlReadInt8( file, NULL );
	if ( out->height == 0 ) {
		PlReportErrorF( PL_RESULT_FILEERR, "invalid height" );
		return false;
	}

	PlReadInt32( file, false, NULL );

	out->flags = ( uint16_t ) PlReadInt16( file, false, NULL );
	out->hash = ( uint32_t ) PlReadInt32( file, false, NULL );

	return true;
}

static PLImage *SPTToImage( const SPTHeader *header, const uint16_t *pal, const uint8_t *img, unsigned int size, SPTFormat format ) {
	PLImage *image = PlCreateImage( NULL, header->width, header->height, 0, PL_COLOURFORMAT_RGBA, PL_IMAGEFORMAT_RGBA8 );
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

static PLImage *ParseImage( PLFile *file, PLMemoryGroup *group, const SPTHeader *header ) {
	/* fetch image data */
	if ( !PlFileSeek( file, header->imageOffset, PL_SEEK_SET ) ) {
		return NULL;
	}

	unsigned int size = PlReadInt32( file, false, NULL );
	if ( size == 0 || size < ( header->width * header->height ) ) {
		PlReportErrorF( PL_RESULT_FILEREAD, "invalid size for index" );
		return NULL;
	}

	SPTFormat format = ( header->flags & SPT_FLAGS_8BPP ) ? SPT_FORMAT_8BPP : SPT_FORMAT_4BPP;
	//assert( format != SPT_FORMAT_8BPP );
	if ( format == SPT_FORMAT_4BPP ) {
		size /= 2;
	}

	uint8_t *img = PL_GNEW_( group, uint8_t, size );
	if ( PlReadFile( file, img, sizeof( uint8_t ), size ) != size ) {
		return NULL;
	}

	/* fetch palette */
	uint16_t pal[ 16 ];
	if ( !PlFileSeek( file, header->paletteOffset, PL_SEEK_SET ) ) {
		return NULL;
	}
	if ( PlReadFile( file, pal, sizeof( uint16_t ), 16 ) != 16 ) {
		return NULL;
	}

	return SPTToImage( header, pal, img, size, format );
}

PLImage *Blitz_SPT_ParseImage( PLFile *file ) {
	SPTHeader header;
	if ( !ParseHeader( file, &header ) ) {
		return NULL;
	}

	PLMemoryGroup *group = PlCreateMemoryGroup();
	PLImage *image = ParseImage( file, group, &header );
	PlDestroyMemoryGroup( group );

	return image;
}

void Blitz_SPT_BulkExport( PLFile *file, const char *destination, const char *format ) {
	unsigned int i = 0;
	SPTHeader header;
	do {
		if ( !ParseHeader( file, &header ) ) {
			break;
		}

		/* save this so we can restore our position after */
		PLFileOffset offset = PlGetFileOffset( file );

		PLMemoryGroup *group = PlCreateMemoryGroup();
		PLImage *image = ParseImage( file, group, &header );
		PlDestroyMemoryGroup( group );

		if ( image != NULL ) {
			PLPath path;
			snprintf( path, sizeof( path ), "%s/%X.%s", destination, header.hash, format );
			PlWriteImage( image, path );
			PlDestroyImage( image );
		}

		PlFileSeek( file, offset, PL_SEEK_SET );

		i++;
	} while ( !( header.flags & SPT_FLAGS_END ) );
}
