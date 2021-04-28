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

#include "image_private.h"

/* Sentient's TXTR format */

static bool SentientTxtrFormatCheck( PLFile *ptr ) {
	PlRewindFile( ptr );

	struct {
		char ident[ 4 ];    // RTXT
		unsigned int length;// Length of chunk
	} header;
	if ( PlReadFile( ptr, &header, sizeof( header ), 1 ) != 1 ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to read image header" );
		return false;
	}

	if ( strncmp( "RTXT", header.ident, 4 ) != 0 ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "invalid identifer, %s vs RTXT", header.ident );
		return false;
	}

	if ( header.length > PlGetFileSize( ptr ) ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "invalid size indicated by header" );
		return false;
	}

	return true;
}

bool PlLoadSentientTxtrImage( PLFile *ptr, PLImage *out ) {
	PlRewindFile( ptr );
	PlFileSeek( ptr, 4, PL_SEEK_CUR );

	bool status;
	unsigned int length = PlReadInt32( ptr, false, &status );
	if ( !status ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM1, "failed to read length from header" );
		return false;
	}

	// HACK HACK HACK...
	unsigned int wh = sqrt( length / 3 );//plRoundUp(sqrt(length / 3), 2);
	if ( !PlIsPowerOfTwo( wh ) ) {
		if ( wh < 100 ) {
			wh = PlRoundUp( sqrt( length / 3 ), 2 );
		}
	}

	out->width = out->height = wh;
	out->colour_format = PL_COLOURFORMAT_RGB;
	out->format = PL_IMAGEFORMAT_RGB8;
	out->size = PlGetImageSize( out->format, out->width, out->height );
	out->levels = 1;
	out->data = pl_calloc( out->levels, sizeof( uint8_t * ) );
	out->data[ 0 ] = pl_calloc( out->size, sizeof( uint8_t ) );
	PlReadFile( ptr, out->data[ 0 ], 1, out->size );
	return true;
}
