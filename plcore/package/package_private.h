/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#pragma once

#include "pl_private.h"

#include <plcore/pl_filesystem.h>
#include <plcore/pl_package.h>

PL_EXTERN_C

PLPackage *PlLoadMadPackage( const char *path );
PLPackage *PlLoadArtPackage( const char *path );
PLPackage *PlLoadLstPackage( const char *path );
PLPackage *PlLoadTabPackage( const char *path );
PLPackage *PlLoadVsrPackage( const char *path );
PLPackage *PlLoadFfPackage( const char *path );
PLPackage *PlLoadRidbPackage( const char *path );
PLPackage *PlLoadApukPackage( const char *path );
PLPackage *PlLoadOPKPackage( const char *path );

PL_EXTERN_C_END
