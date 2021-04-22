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

/* 4X Technologies Package Format
 * These come in both an IBF and LST format;
 * the LST file lists all of the contents
 * of the IBF.
 *
 * There doesn't appear to be any form of
 * compression used on these packages. */

PLPackage* plLoadLSTPackage( const char* path ) {
	PLFile* fh = plOpenFile( path, false );
	if ( fh == NULL ) {
		return NULL;
	}

	PLPackage* package = NULL;

	/* any files we're going to load will be from the IBF, not the LST */
	char ibf_path[PL_SYSTEM_MAX_PATH + 1];
	strncpy( ibf_path, path, strlen( path ) - 3 );
	strncat( ibf_path, "ibf", PL_SYSTEM_MAX_PATH );
	if ( !plFileExists( ibf_path ) ) {
		plReportErrorF( PL_RESULT_FILEPATH, "failed to open ibf package at \"%s\", aborting", ibf_path );
		goto ABORT;
	}

	//DebugPrint("LST %s\n", path);
	//DebugPrint("IBF %s\n", ibf_path);

	/* grab the IBF size so we can do some sanity checking later */
	size_t ibf_size = plGetLocalFileSize( ibf_path );
	if ( ibf_size == 0 ) {
		plReportErrorF( PL_RESULT_FILESIZE, "invalid ibf \"%s\" size of 0, aborting", ibf_path );
		goto ABORT;
	}

	/* read in the ident */
	char ident[8];
	if ( plReadFile( fh, ident, sizeof( char ), 8 ) != 8 ) {
		plReportErrorF( PL_RESULT_FILETYPE, "failed to read identification, aborting" );
		goto ABORT;
	}

	if ( strncmp( ident, "_TSL1.0V", 8 ) != 0 ) {
		plReportErrorF( PL_RESULT_FILETYPE, "invalid file ident, \"%s\", aborting", ident );
		goto ABORT;
	}

	/* sanity checking */
	uint32_t num_indices;
	if ( plReadFile( fh, &num_indices, sizeof( unsigned int ), 1 ) != 1 ) {
		plReportErrorF( PL_RESULT_FILEREAD, "failed to read in indices count from lst, aborting" );
		goto ABORT;
	}

	if ( num_indices > 4096 ) {
		plReportErrorF( PL_RESULT_FILESIZE, "larger than expected package, aborting" );
		goto ABORT;
	}

	//DebugPrint("LST INDICES %u\n", num_indices);

	struct {
		char name[64];
		uint32_t data_offset;
		uint32_t data_length;
	} index;

	package = PlCreatePackageHandle( path, num_indices, NULL );
	for ( unsigned int i = 0; i < num_indices; ++i ) {
		if ( plIsEndOfFile( fh ) != 0 ) {
			printf( "Unexpected end of package in %s, ignoring!\n", path );
			break;
		}

		if ( plReadFile( fh, &index, sizeof( index ), 1 ) != 1 ) {
			plReportErrorF( PL_RESULT_FILEREAD, "failed to read index at %d, aborting", plGetFileOffset( fh ) );
			goto ABORT;
		}

		//DebugPrint("LST INDEX %s\n", index.name);

		if ( index.data_offset >= ibf_size
			|| ( uint64_t ) ( index.data_offset ) + ( uint64_t ) ( index.data_length ) > ibf_size ) {
			plReportErrorF( PL_RESULT_FILESIZE, "offset/length falls beyond IBF size, aborting" );
			goto ABORT;
		}

		strncpy( package->table[ i ].fileName, index.name, sizeof( package->table[ i ].fileName ) );
		package->table[ i ].fileName[ sizeof( package->table[ i ].fileName ) - 1 ] = '\0';
		package->table[ i ].fileSize = index.data_length;
		package->table[ i ].offset = index.data_offset;
	}

	plCloseFile( fh );

	return package;

	ABORT:

	if ( package != NULL ) {
		PlDestroyPackage( package );
	}

	plCloseFile( fh );

	return NULL;
}
