/* Copyright (C) 2021 Mark E Sowden <markelswo@gmail.com> */

#include "plugin.h"

/* This outline is probably specific to Future Tactics,
 * which is based on RenderWare but appears to use it's
 * own package format. */

typedef struct PakIndex {
	char fileName[ 48 ];
	uint32_t offset;
	uint32_t lflag;
} PakIndex;

static PLPackage *PAK_ReadFile( PLFile *file ) {
	bool status;

	/* first we're provided with an indication of how many files are in the package */
	unsigned int numFiles = gInterface->ReadInt32( file, false, &status );
	if ( !status ) {
		return NULL;
	}

	PakIndex *indices = gInterface->CAlloc( numFiles, sizeof( PakIndex ) );
	if ( indices == NULL ) {
		return NULL;
	}

	gInterface->ReadFile( file, indices, sizeof( PakIndex ), numFiles );

	const char *path = gInterface->GetFilePath( file );
	PLPackage *package = gInterface->CreatePackageHandle( path, numFiles, NULL );
	for ( unsigned int i = 0; i < numFiles; ++i ) {
		snprintf( package->table[ i ].fileName, sizeof( package->table[ i ].fileName ), "%s", indices[ i ].fileName );

#if 0 /* this appears to be wrong, sadly, so for now just dump the compressed file */
		/* extract the flag from the end of the index */
		uint8_t flag = ( indices[ i ].lflag & 0xFF000000 ) >> 24;
		indices[ i ].lflag &= 0xFFFFFF;

		if ( flag == 0x80 ) { /* indicates the file is compressed */
			package->table[ i ].compressedSize = indices[ i ].lflag;
			package->table[ i ].compressionType = PL_COMPRESSION_ZLIB;

			if ( !gInterface->FileSeek( file, indices[ i ].offset, PL_SEEK_SET ) ) {
				continue;
			}

			package->table[ i ].fileSize = gInterface->ReadInt32( file, false, &status );
			package->table[ i ].offset += 4;
			continue;
		}
#endif

		package->table[ i ].compressedSize = 0;
		package->table[ i ].compressionType = PL_COMPRESSION_NONE;
		package->table[ i ].fileSize = indices[ i ].lflag;
		package->table[ i ].offset = indices[ i ].offset;
	}

	gInterface->Free( indices );

	return package;
}

PLPackage *PAK_LoadFile( const char *path ) {
	PLFile *file = gInterface->OpenFile( path, false );
	if ( file == NULL ) {
		return NULL;
	}

	PLPackage *package = PAK_ReadFile( file );

	gInterface->CloseFile( file );

	return package;
}
