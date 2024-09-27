// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "pl_private.h"

#include <plcore/pl_filesystem.h>
#include <plcore/pl_package.h>

PL_EXTERN_C

PLPackage *PlParseDFSFile( PLFile *file );
PLPackage *PlLoadDFSPackage( const char *path );

PLPackage *PlLoadIWADPackage_( const char *path );
PLPackage *PlLoadWAD2Package_( const char *path );
PLPackage *PlLoadPAKPackage_( const char *path );
PLPackage *PlLoadVPKPackage_( const char *path );
PLPackage *PlLoadMadPackage( const char *path );
PLPackage *PlLoadTabPackage( const char *path );

PLPackage *PlParseFreshBinPackage_( PLFile *file );
PLPackage *PlParseGrpPackage_( PLFile *file );
PLPackage *PlParseOpkPackage_( PLFile *file );// Outcast
PLPackage *PlParseInuPackage_( PLFile *file );// White Fear / Inuits
PLPackage *PlParseAllPackage_( PLFile *file );// The Last Job
PLPackage *PlParseAfsPackage_( PLFile *file );// Headhunter
PLPackage *PlParseAhfPackage_( PLFile *file );// Headhunter

PL_EXTERN_C_END
