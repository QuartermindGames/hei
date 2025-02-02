/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include "pl_private.h"

#include <plcore/pl_image.h>

PLImage *PlParseRsbImage_( PLFile *file );
PLImage *PlParse3drTexImage_( PLFile *file );
PLImage *PlParseAngelTexImage_( PLFile *file );
PLImage *PlParseDtxImage_( PLFile *file );

bool PlWriteQoiImage( const PLImage *image, const char *path );
