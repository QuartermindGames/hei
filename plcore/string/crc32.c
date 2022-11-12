/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <stdint.h>

#define MINIZ_NO_ARCHIVE_APIS
#include "../3rdparty/miniz/miniz.h"

/**
 * This just wraps around to miniz's crc32 function.
 */
unsigned long pl_crc32( const void *data, size_t length, unsigned long crc ) {
	return mz_crc32( crc, data, length );
}
