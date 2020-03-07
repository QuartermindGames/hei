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

/* Outwars FF package format */

PLPackage* plLoadFFPackage( const char* path ) {
	FunctionStart();

	PLFile* fp = plOpenFile( path, false );
	if ( fp == NULL ) {
		return NULL;
	}

	uint32_t num_indices;
	if ( plReadFile( fp, &num_indices, sizeof( uint32_t ), 1 ) != 1 ) {
		plCloseFile( fp );
		return NULL;
	}

	typedef struct OW_FFIndex {
		uint32_t offset;
		char name[40];
	} OW_FFIndex;
	OW_FFIndex* indices = pl_malloc( sizeof( OW_FFIndex ) * num_indices );
	unsigned int* sizes = pl_malloc( sizeof( unsigned int ) * num_indices ); /* aren't stored in index data, so we'll calc these */
	if ( num_indices > 0 ) {
		if ( plReadFile( fp, indices, sizeof( OW_FFIndex ), num_indices ) == num_indices ) {
			for ( unsigned int i = 0; i < ( num_indices - 1 ); ++i ) {
				sizes[ i ] = indices[ i + 1 ].offset - indices[ i ].offset;
			}
		} else {
			ReportError( PL_RESULT_FILEREAD, "failed to read indices" );
		}
	} else {
		ReportError( PL_RESULT_FILEREAD, "invalid number of indices in package" );
	}

	plCloseFile( fp );

	if ( plGetFunctionResult() != PL_RESULT_SUCCESS ) {
		pl_free( indices );
		pl_free( sizes );
		return NULL;
	}

	PLPackage* package = pl_malloc( sizeof( PLPackage ) );
	package->internal.LoadFile = _plLoadGenericPackageFile;
	package->table_size = ( num_indices - 1 );
	strncpy( package->path, path, sizeof( package->path ) );
	if( ( package->table = pl_calloc( package->table_size, sizeof( struct PLPackageIndex ) ) ) != NULL ) {
		for( unsigned int i = 0; i < package->table_size; ++i ) {
			PLPackageIndex *index = &package->table[ i ];
			index->offset = indices[ i ].offset;
			index->fileSize = sizes[ i ];
			strncpy( index->fileName, indices[ i ].name, sizeof( index->fileName ) );
		}
	}
	else {
		plDestroyPackage( package );
		package = NULL;
	}
	
	pl_free( indices );
	pl_free( sizes );

	return package;
}
