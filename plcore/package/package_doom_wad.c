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

/* id Software's Doom WAD package format */

PL_PACKED_STRUCT_START( WadIndex )
uint32_t offset;
uint32_t size;
char name[ 8 ];
PL_PACKED_STRUCT_END( WadIndex )

PLPackage *PlLoadWadPackage( const char *path ) {
	FunctionStart();

	PLFile *filePtr = PlOpenFile( path, false );
	if ( filePtr == NULL ) {
		return NULL;
	}

	/* read in the header */

	char identifier[ 4 ];
	if ( PlReadFile( filePtr, identifier, 1, sizeof( identifier ) ) != sizeof( identifier ) ) {
		PlCloseFile( filePtr );
		return NULL;
	}

	/* first part of the ident varys between I (initial) and P (patch) */
	if ( ( identifier[ 0 ] != 'I' && identifier[ 0 ] != 'P' ) ||
	     !( identifier[ 1 ] == 'W' && identifier[ 2 ] == 'A' && identifier[ 3 ] == 'D' ) ) {
		PlReportErrorF( PL_RESULT_FILETYPE, "invalid wad header, \"%s\"", identifier );
		PlCloseFile( filePtr );
		return NULL;
	}

	bool status;

	uint32_t numLumps = PlReadInt32( filePtr, false, &status );
	uint32_t tableOffset = PlReadInt32( filePtr, false, &status );
	size_t tableSize = sizeof( WadIndex ) * numLumps;
	if ( tableOffset + tableSize > PlGetFileSize( filePtr ) ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "invalid table offset" );
		PlCloseFile( filePtr );
		return NULL;
	}

	if ( !status ) {
		PlCloseFile( filePtr );
		return NULL;
	}

	/* and now read in the file table */

	PlFileSeek( filePtr, tableOffset, PL_SEEK_SET );

	WadIndex *indices = pl_malloc( tableSize );
	for ( unsigned int i = 0; i < numLumps; ++i ) {
#define cleanup()       \
	pl_free( indices ); \
	PlCloseFile( filePtr )
		indices[ i ].offset = PlReadInt32( filePtr, false, &status );
		if ( indices[ i ].offset >= tableOffset ) {
			PlReportErrorF( PL_RESULT_INVALID_PARM1, "invalid file offset for index %d", i );
			cleanup();
			return NULL;
		}

		indices[ i ].size = PlReadInt32( filePtr, false, &status );
		if ( indices[ i ].size >= PlGetFileSize( filePtr ) ) {
			PlReportErrorF( PL_RESULT_INVALID_PARM1, "invalid file size for index %d", i );
			cleanup();
			return NULL;
		}

		if ( PlReadFile( filePtr, indices[ i ].name, 1, 8 ) != 8 ) {
			cleanup();
			return NULL;
		}
	}

	PlCloseFile( filePtr );

	if ( !status ) {
		pl_free( indices );
		return NULL;
	}

	/* yay, we're finally done - now to setup the package object */

	PLPackage *package = PlCreatePackageHandle( path, numLumps, NULL );
	for ( unsigned int i = 0; i < package->table_size; ++i ) {
		PLPackageIndex *index = &package->table[ i ];
		index->offset = indices[ i ].offset;
		index->fileSize = indices[ i ].size;
		strncpy( index->fileName, indices[ i ].name, sizeof( index->fileName ) );
		index->fileName[ 8 ] = '\0';
	}

	pl_free( indices );

	return package;
}
