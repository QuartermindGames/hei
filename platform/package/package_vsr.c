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

/* Loader for Sentient's VSR package format */

typedef struct VSRChunkHeader {
	char identifier[4];
	uint32_t length;
} VSRChunkHeader;

typedef struct VSRDirectoryIndex {
	uint32_t offset;         /* start offset for data */
	uint32_t length;         /* length of the file in bytes */
	uint32_t unknown0[8];
} VSRDirectoryIndex;
PL_STATIC_ASSERT( sizeof( VSRDirectoryIndex ) == 40, "needs to be 40 bytes" );

typedef struct VSRDirectoryChunk {
	VSRChunkHeader header;         /* DIRC */
	uint32_t num_indices;    /* number of indices in this chunk */
} VSRDirectoryChunk;
PL_STATIC_ASSERT( sizeof( VSRDirectoryChunk ) == 12, "needs to be 12 bytes" );

typedef struct VSRStringIndex {
	char file_name[256];
} VSRStringIndex;

typedef struct VSRStringChunk {
	VSRChunkHeader header;         /* STRT */
	uint32_t num_indices;
} VSRStringChunk;

typedef struct VSRHeader {
	VSRChunkHeader header;         /* VSR1 */
	uint32_t unknown0;
	uint32_t unknown1;       /* always 4 */
	uint32_t unknown2;
	uint32_t length_data;    /* length of file, minus header */
	uint32_t num_indices;    /* number of files in the package */
	uint32_t num_indices2;   /* ditto to the above? */
} VSRHeader;
PL_STATIC_ASSERT( sizeof( VSRHeader ) == 32, "needs to be 32 bytes" );

PLPackage* plLoadVSRPackage( const char* path ) {
	FunctionStart();

	PLFile* fp = plOpenFile( path, false );
	if ( fp == NULL ) {
		return NULL;
	}

	VSRHeader chunk_header;
	VSRDirectoryChunk chunk_directory;
	VSRStringChunk chunk_strings;

	VSRDirectoryIndex* directories = NULL;
	VSRStringIndex* strings = NULL;

	/* load in all the file data first */

	plReadFile( fp, &chunk_header, sizeof( VSRHeader ), 1 );
	if ( strncmp( chunk_header.header.identifier, "1RSV", 4 ) == 0 ) {
		plReadFile( fp, &chunk_directory, sizeof( VSRDirectoryChunk ), 1 );
		if ( strncmp( chunk_directory.header.identifier, "CRID", 4 ) == 0 ) {
			directories = pl_malloc( sizeof( VSRDirectoryIndex ) * chunk_directory.num_indices );
			plReadFile( fp, directories, sizeof( VSRDirectoryIndex ), chunk_directory.num_indices );

			/* skip VSRN chunk, seems to be unused? */
			plFileSeek( fp, 12, PL_SEEK_CUR );

			plReadFile( fp, &chunk_strings, sizeof( VSRStringChunk ), 1 );
			if ( strncmp( chunk_strings.header.identifier, "TRTS", 4 ) == 0 ) {
				/* read in all the string offsets, which in turn helps us determine
				 * the length of each string */

				/* fuck this, let's do this the lazy way */
				plFileSeek( fp, sizeof( uint32_t ) * chunk_strings.num_indices, PL_SEEK_CUR );
				strings = pl_calloc( sizeof( VSRStringIndex ), chunk_strings.num_indices );
				for ( unsigned int i = 0; i < chunk_strings.num_indices; ++i ) {
					for ( unsigned int j = 0; j < 256; ++j ) {
						strings[ i ].file_name[ j ] = plReadInt8( fp, NULL );
						if ( strings[ i ].file_name[ j ] == '\0' ) {
							break;
						}
					}
				}
			} else {
				ReportError( PL_RESULT_FILETYPE, "failed to read STRS header" );
			}
		} else {
			ReportError( PL_RESULT_FILETYPE, "failed to read DIRC header" );
		}
	} else {
		ReportError( PL_RESULT_FILETYPE, "failed to read VSR1 header" );
	}

	plCloseFile( fp );

	if ( plGetFunctionResult() != PL_RESULT_SUCCESS ) {
		pl_free( directories );
		pl_free( strings );
		return NULL;
	}

	/* now create our package handle */

	PLPackage* package = plCreatePackageHandle( path, chunk_directory.num_indices, NULL );
	for ( unsigned int i = 0; i < package->table_size; ++i ) {
		PLPackageIndex* index = &package->table[ i ];
		index->offset = directories[ i ].offset;
		index->fileSize = directories[ i ].length;
		strncpy( index->fileName, strings[ i ].file_name, sizeof( index->fileName ) );
	}

	pl_free( directories );
	pl_free( strings );

	if ( plGetFunctionResult() != PL_RESULT_SUCCESS ) {
		plDestroyPackage( package );
		package = NULL;
	}

	return package;
}
