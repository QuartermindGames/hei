/*
MIT License

Copyright (c) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "package_private.h"

/*  MAD/MTD Format Specification    */
/* The MAD/MTD format is the package format used by
 * Hogs of War to store and index content used by
 * the game.
 *
 * Files within these packages are expected to be in
 * a specific order, as both the game and other assets
 * within the game rely on this order so that they, for
 * example, will know which textures to load in / use.
 *
 * Because of this, any package that's recreated will need
 * to be done so in a way that preserves the original file
 * order.
 *
 * Thanks to solemnwarning for his help on this one!
 */

PL_PACKED_STRUCT_START( MADIndex )
char file[ 16 ];

uint32_t offset;
uint32_t length;
PL_PACKED_STRUCT_END( MADIndex )

PLPackage *PlLoadMadPackage( const char *path ) {
	FunctionStart();

	PLFile *fh = PlOpenFile( path, false );
	if ( fh == NULL ) {
		return NULL;
	}

	PLPackage *package = NULL;

	size_t file_size = PlGetLocalFileSize( path );
	if ( PlGetFunctionResult() != PL_RESULT_SUCCESS ) {
		goto FAILED;
	}

	/* Figure out the number of headers in the MAD file by reading them in until we cross into the data region of one
	 * we've previously loaded. Checks each header is valid.
	 */

	size_t data_begin = file_size;
	unsigned int num_indices = 0;

	while ( ( num_indices + 1 ) * sizeof( MADIndex ) <= data_begin ) {
		MADIndex index;
		if ( PlReadFile( fh, &index, sizeof( MADIndex ), 1 ) != 1 ) {
			/* EOF, or read error */
			goto FAILED;
		}

		// ensure the file name is valid...
		for ( unsigned int i = 0; i < 16; ++i ) {
			if ( isprint( index.file[ i ] ) == 0 && index.file[ i ] != '\0' ) {
				PlReportErrorF( PL_RESULT_FILEREAD, "received invalid filename for index" );
				goto FAILED;
			}
		}

		if ( index.offset >= file_size || ( uint64_t ) ( index.offset ) + ( uint64_t ) ( index.length ) > file_size ) {
			/* File offset/length falls beyond end of file */
			PlReportErrorF( PL_RESULT_FILEREAD, "file offset/length falls beyond end of file" );
			goto FAILED;
		}

		if ( index.offset < data_begin ) {
			data_begin = index.offset;
		}

		++num_indices;
	}

	/* Allocate the basic package structure now we know how many files are in the archive. */
	package = PlCreatePackageHandle( path, num_indices, NULL );

	/* Rewind the file handle and populate package->table with the metadata from the headers. */

	PlRewindFile( fh );

	for ( unsigned int i = 0; i < num_indices; ++i ) {
		MADIndex index;
		if ( PlReadFile( fh, &index, sizeof( MADIndex ), 1 ) != 1 ) {
			/* EOF, or read error */
			PlReportErrorF( PL_RESULT_FILEREAD, "failed to read MAD index %d", i );
			goto FAILED;
		}

		strncpy( package->table[ i ].fileName, index.file, sizeof( package->table[ i ].fileName ) );
		package->table[ i ].fileName[ sizeof( index.file ) - 1 ] = '\0';
		package->table[ i ].fileSize = index.length;
		package->table[ i ].offset = index.offset;
	}

	PlCloseFile( fh );

	return package;

FAILED:

	PlDestroyPackage( package );

	PlCloseFile( fh );

	return NULL;
}
