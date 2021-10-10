/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include "pl_private.h"

#include <plcore/pl_filesystem.h>
#include <plcore/pl_package.h>

#define PLPACKAGE_VERSION_MAJOR     1
#define PLPACKAGE_VERSION_MINOR     0

enum {
	PLPACKAGE_LEVEL_INDEX,
	PLPACKAGE_MODEL_INDEX,
	PLPACKAGE_TEXTURE_INDEX,
	PLPACKAGE_MATERIAL_INDEX,

	PLPACKAGE_VERTICES_INDEX,
	PLPACKAGE_TRIANGLES_INDEX,

	// Level

	PLPACKAGE_LEVEL_SECTORS_INDEX, // aka 'rooms', links in with triangles?
	PLPACKAGE_LEVEL_PORTALS_INDEX, // aka 'doors', links in with triangles and sectors?
	PLPACKAGE_LEVEL_OBJECTS_INDEX, // list of objects within the level

	// Model

	// Texture

	PLPACKAGE_UNKNOWN_INDEX,

	PLPACKAGE_LAST_INDEX
};

typedef struct PLPackageHeader { // (PACKAGE)
	uint8_t identity[4];    // Descriptor/name of the data type. "PACK"
	uint8_t version[2];     // Version of this type.

	uint32_t num_indexes;    // Number of data indexes (each _should_ be a fixed size?)

	// followed by num_indexes + length
	// then followed by rest of data
} PLPackageHeader;

typedef struct PLPackageIndexHeader {
	uint16_t type;
	uint32_t length;

	// followed by type-specific index information
} PLPackageIndexHeader;

PL_EXTERN_C

inline static void WritePackageHeader( FILE* handle, uint32_t num_indexes ) {
	plAssert( handle );
	uint8_t identity[4] = { 'P', 'A', 'C', 'K' };
	fwrite( identity, sizeof( char ), sizeof( identity ), handle );
	uint8_t version[2] = {
		PLPACKAGE_VERSION_MAJOR,
		PLPACKAGE_VERSION_MINOR
	};
	fwrite( version, sizeof( uint8_t ), sizeof( version ), handle );
	fwrite( &num_indexes, sizeof( uint32_t ), 1, handle );
}

inline static void WritePackageIndexHeader( FILE* handle, uint16_t type, uint32_t length ) {
	plAssert( handle );
	fwrite( &type, sizeof( uint16_t ), 1, handle );
	fwrite( &length, sizeof( uint32_t ), 1, handle );
}

/////////////////////////////////////////////////////////////////

PLPackage *PlLoadMadPackage( const char *path );
PLPackage *PlLoadArtPackage( const char *path );
PLPackage *PlLoadLstPackage( const char *path );
PLPackage *PlLoadTabPackage( const char *path );
PLPackage *PlLoadVsrPackage( const char *path );
PLPackage *PlLoadFfPackage( const char *path );
PLPackage *PlLoadWadPackage( const char *path );
PLPackage *PlLoadRidbPackage( const char *path );
PLPackage *PlLoadApukPackage( const char *path );
PLPackage *PlLoadOPKPackage( const char *path );

PL_EXTERN_C_END
