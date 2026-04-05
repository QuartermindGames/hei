/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include "pl_private.h"

#include <plcore/pl_image.h>

PLImage *PlParseRsbImage_( QmFsFile *file );
PLImage *PlParse3drTexImage_( QmFsFile *file );
PLImage *PlParseAngelTexImage_( QmFsFile *file );
PLImage *PlParseDtxImage_( QmFsFile *file );

bool PlWriteQoiImage( const PLImage *image, const char *path );
