// SPDX-License-Identifier: MIT
// Copyright © 2023 Mark E Sowden <hogsy@oldtimes-software.com>

#include "../core.h"

#define HG_MAX_VERSION 2016

static int ValidateHeader( PLFile *file ) {
	if ( gInterface->ReadInt32( file, false, NULL ) != PL_MAGIC_TO_NUM( 'H', 'E', 'D', ' ' ) ) {
		gInterface->ReportError( PL_RESULT_FILETYPE, PL_FUNCTION, "invalid magic" );
		return 0;
	}
	int size = gInterface->ReadInt32( file, false, NULL );
	if ( size != 40 ) {
		gInterface->ReportError( PL_RESULT_FILEVERSION, PL_FUNCTION, "invalid header size" );
		return 0;
	}
	int version = gInterface->ReadInt32( file, false, NULL );
	if ( version <= 0 || version > HG_MAX_VERSION ) {
		gInterface->ReportError( PL_RESULT_FILEVERSION, PL_FUNCTION, "invalid version" );
		return 0;
	}
	return size;
}

// "Object Header"

static int ValidateObjectHeader( PLFile *file ) {
	if ( gInterface->ReadInt32( file, false, NULL ) != PL_MAGIC_TO_NUM( 'H', 'O', 'B', 'J' ) ) {
		gInterface->ReportError( PL_RESULT_FILETYPE, PL_FUNCTION, "invalid magic" );
		return 0;
	}
	int size = gInterface->ReadInt32( file, false, NULL );
	if ( size != 16 ) {
		gInterface->ReportError( PL_RESULT_FILEVERSION, PL_FUNCTION, "invalid header size" );
		return 0;
	}
	int version = gInterface->ReadInt32( file, false, NULL );
	if ( version <= 0 || version > HG_MAX_VERSION ) {
		gInterface->ReportError( PL_RESULT_FILEVERSION, PL_FUNCTION, "invalid object header version" );
		return 0;
	}
	return size;
}

// "Object"

static int ValidateObject( PLFile *file ) {
	if ( gInterface->ReadInt32( file, false, NULL ) != PL_MAGIC_TO_NUM( 'O', 'B', 'J', ' ' ) ) {
		gInterface->ReportError( PL_RESULT_FILETYPE, PL_FUNCTION, "invalid magic" );
		return 0;
	}
	int version = gInterface->ReadInt32( file, false, NULL );
	if ( version <= 0 || version > HG_MAX_VERSION ) {
		gInterface->ReportError( PL_RESULT_FILEVERSION, PL_FUNCTION, "invalid object version" );
		return 0;
	}
	return size;
}

#define MESH_MAGIC PL_MAGIC_TO_NUM( 'M', 'E', 'S', 'H' )
#define PRIM_MAGIC PL_MAGIC_TO_NUM( 'P', 'R', 'I', 'M' )
#define NODE_MAGIC PL_MAGIC_TO_NUM( 'N', 'O', 'D', 'E' )
#define TXTR_MAGIC PL_MAGIC_TO_NUM( 'T', 'X', 'T', 'R' )

typedef struct HGMHeader {
	uint32_t magic;
	uint32_t headerSize;
	uint32_t version;// expected to be less than 2017
} HGMHeader;
static HGMHeader *ParseHeader( PLFile *file, HGMHeader *out ) {
	PL_ZERO_( *out );
	int32_t magic = gInterface->ReadInt32( file, false, NULL );
	if ( magic != HED_MAGIC ) {
	}

	int32_t headerSize = gInterface->ReadInt32( file, false, NULL );


	int32_t version = gInterface->ReadInt32( file, false, NULL );


	return out;
}

void HG_HGM_ParseFile( PLFile *file ) {
	HGMHeader header;
	if ( ParseHeader( file, &header ) == NULL ) {
		return;
	}
}

void HG_HGM_LoadFile( const char *path ) {
}
