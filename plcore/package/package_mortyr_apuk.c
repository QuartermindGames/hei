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

/* Mortyr's APUK package format */

PLPackage *plLoadAPUKPackage( const char *path ) {
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
		identifier[ 0 ] == 'A' &&
		identifier[ 1 ] == 'P' &&
		identifier[ 2 ] == 'U' &&
		identifier[ 3 ] == 'K'
		) ) {
		plReportErrorF( PL_RESULT_FILETYPE, "invalid package identifier, \"%s\"", identifier );
		plCloseFile( filePtr );
		return NULL;
	}

	typedef struct FileIndex {
		uint32_t size;
		uint32_t offset;
		int32_t unknown[ 2 ];
		char name[ 16 ];
	} FileIndex;

	bool status;
	uint32_t numFiles = plReadInt32( filePtr, false, &status );
	if( !status ) {
		plCloseFile( filePtr );
		return NULL;
	}

	/* make sure the file table is valid */
	size_t tableSize = sizeof( FileIndex ) * numFiles;
	if( tableSize + 32 > plGetFileSize( filePtr ) ) {
		plReportErrorF( PL_RESULT_INVALID_PARM1, "invalid file table" );
		plCloseFile( filePtr );
		return NULL;
	}

	/* 24 bytes of nothing, can't think what this was intended for... */
	if( !plFileSeek( filePtr, 24, PL_SEEK_CUR ) ) {
		plCloseFile( filePtr );
		return NULL;
	}

	FileIndex *indices = pl_malloc( sizeof( FileIndex ) * numFiles );
	for( unsigned int i = 0; i < numFiles; ++i ) {
		indices[ i ].size = plReadInt32( filePtr, false, &status );
		indices[ i ].offset = plReadInt32( filePtr, false, &status );
		indices[ i ].unknown[ 0 ] = plReadInt32( filePtr, false, &status );
		indices[ i ].unknown[ 1 ] = plReadInt32( filePtr, false, &status );
		
		if( plReadFile( filePtr, indices[ i ].name, 1, 16 ) != 16 ) {
			status = false;
		}
	}

	plCloseFile( filePtr );

	if( !status ) {
		pl_free( indices );
		return NULL;
	}

	/* yay, we're finally done - now to setup the package object */

	PLPackage *package = PlCreatePackageHandle( path, numFiles, NULL );
	for( unsigned int i = 0; i < package->table_size; ++i ) {
		PLPackageIndex *index = &package->table[ i ];
		index->offset = indices[ i ].offset;
		index->fileSize = indices[ i ].size;
		strncpy( index->fileName, indices[ i ].name, sizeof( index->fileName ) );
	}

	pl_free( indices );

	return package;
}
