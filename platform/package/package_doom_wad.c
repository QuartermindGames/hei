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

/* id Software's Doom WAD package format */

PLPackage *plLoadDoomWadPackage( const char *path ) {
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

	/* first part of the ident varys between I (initial) and P (patch) */
	if( ( identifier[ 0 ] == 'I' || identifier[ 0 ] == 'P' ) &&
		identifier[ 1 ] != 'W' &&
		identifier[ 2 ] != 'A' &&
		identifier[ 3 ] != 'D' ) {
		ReportError( PL_RESULT_FILETYPE, "invalid wad header, \"%s\"", identifier );
		plCloseFile( filePtr );
		return NULL;
	}

	bool status;

	uint32_t numLumps = plReadInt32( filePtr, false, &status );
	if( numLumps == 0 ) {
		ReportError( PL_RESULT_INVALID_PARM1, "no lumps in wad" );
		plCloseFile( filePtr );
		return NULL;
	}

	uint32_t tableOffset = plReadInt32( filePtr, false, &status );
	if( tableOffset >= plGetFileSize( filePtr ) ) {
		ReportError( PL_RESULT_INVALID_PARM1, "invalid table offset" );
		plCloseFile( filePtr );
		return NULL;
	}

	if( !status ) {
		plCloseFile( filePtr );
		return NULL;
	}

	/* and now read in the file table */

	typedef struct WadIndex {
		uint32_t offset;
		uint32_t size;
		char     name[ 8 ];
	} WadIndex;
	WadIndex *indices = pl_malloc( sizeof( WadIndex ) * numLumps );
	for( unsigned int i = 0; i < numLumps; ++i ) {
		indices[ i ].offset = plReadInt32( filePtr, false, &status );
		indices[ i ].size = plReadInt32( filePtr, false, &status );
		if( plReadString( filePtr, indices[ i ].name, sizeof( indices[ i ].name ) ) == NULL ) {
			pl_free( indices );
			plCloseFile( filePtr );
			return NULL;
		}
	}

	plCloseFile( filePtr );

	if( !status ) {
		pl_free( indices );
		return NULL;
	}

	/* yay, we're finally done - now to setup the package object */

	PLPackage *package = pl_malloc( sizeof( PLPackage ) );
	package->internal.LoadFile = _plLoadGenericPackageFile;

	/* setup the file table */
	package->table_size = numLumps;
	package->table = pl_malloc( sizeof( PLPackageIndex ) * package->table_size );
	for( unsigned int i = 0; i < package->table_size; ++i ) {
		PLPackageIndex *index = &package->table[ i ];
		index->offset = indices->offset;
		index->fileSize = indices->size;
		strncpy( index->fileName, indices->name, sizeof( index->fileName ) );
	}

	pl_free( indices );

	return package;
}
