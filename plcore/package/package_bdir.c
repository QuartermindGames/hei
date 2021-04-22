/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include "package_private.h"

/* Eradicator's BDIR package format */

typedef struct BdirIndex {
	char		name[ 12 ];
	uint32_t	offset;
	uint32_t	size;
} BdirIndex;

PLPackage *plLoadRIDBPackage( const char *path ) {
	FunctionStart();

	PLFile *filePtr = plOpenFile( path, false );
	if( filePtr == NULL ) {
		return NULL;
	}

	/* read in the header */

	char identifier[ 4 ];
	if( plReadFile( filePtr, identifier, 1, sizeof( identifier ) ) != sizeof( identifier ) ) {
		plCloseFile( filePtr );
		return NULL;
	}

	if( !(
		identifier[ 0 ] == 'R' &&
		identifier[ 1 ] == 'I' && 
		identifier[ 2 ] == 'D' && 
		identifier[ 3 ] == 'B' ) ) {
		plReportErrorF( PL_RESULT_FILETYPE, "invalid bdir header, \"%s\"", identifier );
		plCloseFile( filePtr );
		return NULL;
	}

	bool status;

	uint32_t numLumps = plReadInt32( filePtr, false, &status );
	uint32_t tableOffset = plReadInt32( filePtr, false, &status );
	size_t tableSize = sizeof( BdirIndex ) * numLumps;
	if( tableOffset + tableSize > plGetFileSize( filePtr ) ) {
		plReportErrorF( PL_RESULT_INVALID_PARM1, "invalid table offset" );
		plCloseFile( filePtr );
		return NULL;
	}

	if( !status ) {
		plCloseFile( filePtr );
		return NULL;
	}

	/* and now read in the file table */

	plFileSeek( filePtr, tableOffset, PL_SEEK_SET );

	BdirIndex *indices = pl_malloc( tableSize );
	for( unsigned int i = 0; i < numLumps; ++i ) {
#define cleanup() pl_free( indices ); plCloseFile( filePtr )
		if( plReadFile( filePtr, indices[ i ].name, 1, 12 ) != 12 ) {
			cleanup();
			return NULL;
		}
		indices[ i ].name[ 11 ] = '\0';

		indices[ i ].offset = plReadInt32( filePtr, false, &status );
		if( indices[ i ].offset >= tableOffset ) {
			plReportErrorF( PL_RESULT_INVALID_PARM1, "invalid file offset for index %d", i );
			cleanup();
			return NULL;
		}

		indices[ i ].size = plReadInt32( filePtr, false, &status );
		if( indices[ i ].size >= plGetFileSize( filePtr ) ) {
			plReportErrorF( PL_RESULT_INVALID_PARM1, "invalid file size for index %d", i );
			cleanup();
			return NULL;
		}
	}

	plCloseFile( filePtr );

	if( !status ) {
		pl_free( indices );
		return NULL;
	}

	/* yay, we're finally done - now to setup the package object */

	PLPackage *package = PlCreatePackageHandle( path, numLumps, NULL );
	for( unsigned int i = 0; i < package->table_size; ++i ) {
		PLPackageIndex *index = &package->table[ i ];
		index->offset = indices[ i ].offset;
		index->fileSize = indices[ i ].size;
		strncpy( index->fileName, indices[ i ].name, sizeof( index->fileName ) );
	}

	pl_free( indices );

	return package;
}
