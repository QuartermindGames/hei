/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl_image.h>

struct DDSPixelFormat {
	uint32_t size;
	uint32_t flags;
	uint32_t fourcc;
	uint32_t rgbbitcount;
} DDSPixelFormat;

typedef struct DDSHeader {
	uint32_t size;// Should always be 124.
	uint32_t flags;
	uint32_t height, width;
	uint32_t pitchlinear;
	uint32_t depth;
	uint32_t levels;
	uint32_t reserved1[ 11 ];

	//

	uint32_t caps, caps2, caps3, caps4;
	uint32_t reserved2;
} DDSHeader;

enum DDSFlag {
	DDS_CAPS,
};

bool PlDDSFormatCheck( PLFile *fin ) {
	PlRewindFile( fin );

	char ident[ 4 ];
	if ( PlReadFile( fin, ident, sizeof( char ), 4 ) != 4 ) {
		return false;
	}

	return ( bool ) ( strncmp( ident, "DDS", 3 ) == 0 );
}

bool PlLoadDDSImage( PLFile *fin, PLImage *out ) {
	DDSHeader header;
	memset( &header, 0, sizeof( DDSHeader ) );
	if ( PlReadFile( fin, &header, sizeof( DDSHeader ), 1 ) != 1 ) {
		return false;
	}

	memset( out, 0, sizeof( PLImage ) );

	out->width = header.width;
	out->height = header.height;

	return true;
}
