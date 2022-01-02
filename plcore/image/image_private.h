/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include "pl_private.h"

#include <plcore/pl_image.h>

PLImage *PlLoad3dfImage( const char *path );
PLImage *PlLoadFtxImage( const char *path );
PLImage *PlLoadTimImage( const char *path );
PLImage *PlLoadSwlImage( const char *path );
PLImage *PlLoadQoiImage( const char *path );
PLImage *PlLoadDdsImage( const char *path );

bool PlWriteQoiImage( const PLImage *image, const char *path );
