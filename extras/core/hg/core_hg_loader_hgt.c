/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "../core.h"

/* Core Design, HGT format
 *
 * Usages:
 * 	- Herdy Gerdy (2002, PS2)
 *
 * Notes:
 *  It's just a BMP with an extra header at the start.
 */

#define HGT_HEADER_MAGIC PL_MAGIC_TO_NUM( 'T', 'X', 'T', 'R' )
#define HGT_HEADER_SIZE  44

PLImage *Core_HGT_ParseImage( PLFile *file ) {
	/* validate it first */
	gInterface->FileSeek( file, 4, PL_SEEK_CUR );
	uint32_t magic = gInterface->ReadInt32( file, false, NULL );
	if ( magic != HGT_HEADER_MAGIC ) {
		gInterface->ReportError( PL_RESULT_FILETYPE, PL_FUNCTION, "invalid magic" );
		return NULL;
	}

	/* now seek to the bitmap */
	if ( !gInterface->FileSeek( file, HGT_HEADER_SIZE, PL_SEEK_SET ) ) {
		return NULL;
	}

	/* load the file into a memory, as it's just a bmp */
	const void *buf = gInterface->CacheFile( file );
	if ( buf == NULL ) {
		return NULL;
	}

	/* create a temporary file handle, so we can pass it back to our ParseImage function */
	PLPath tmpPath;
	const char *path = gInterface->GetFilePath( file );
	snprintf( tmpPath, sizeof( PLPath ), "%s.bmp", ( path != NULL ) ? path : "tmp" );
	PLFile *tmp = gInterface->CreateFileFromMemory( tmpPath, ( void * ) buf, ( gInterface->GetFileSize( file ) - HGT_HEADER_SIZE ), PL_FILE_MEMORYBUFFERTYPE_UNMANAGED );

	PLImage *image = gInterface->ParseImage( tmp );

	gInterface->CloseFile( tmp );

	return image;
}
