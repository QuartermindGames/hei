/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl.h>
#include <plcore/pl_image.h>

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
	PlFileSeek( file, 4, PL_SEEK_CUR );
	uint32_t magic = PlReadInt32( file, false, NULL );
	if ( magic != HGT_HEADER_MAGIC ) {
		PlReportBasicError( PL_RESULT_FILETYPE );
		return NULL;
	}

	/* now seek to the bitmap */
	if ( !PlFileSeek( file, HGT_HEADER_SIZE, PL_SEEK_SET ) ) {
		return NULL;
	}

	/* load the rest of the file into a temporary buffer, as it's just a bmp */
	unsigned int bufSize = PlGetFileSize( file ) - HGT_HEADER_SIZE;
	void *buf = PlMAllocA( bufSize );
	if ( PlReadFile( file, buf, sizeof( char ), bufSize ) != bufSize ) {
		return NULL;
	}

	const char *path = PlGetFilePath( file );
	if ( path == NULL ) {
		path = "tmp";
	}

	/* provide an alt path so we can basically hint to the
	 * ParseImage function that it's a bmp and then create
	 * a temporary file handle for it... in future we should
	 * really just pass the current file's buffer instead
	 * of allocating another, but this will require some
	 * work... */

	PLPath tmpPath;
	snprintf( tmpPath, sizeof( PLPath ), "%s.bmp", path );
	PLFile *tmp = PlCreateFileFromMemory( tmpPath, buf, bufSize, PL_FILE_MEMORYBUFFERTYPE_OWNER );

	PLImage *image = PlParseImage( tmp );

	/* as the file has ownership, this will free the buffer */
	PlCloseFile( tmp );

	return image;
}
