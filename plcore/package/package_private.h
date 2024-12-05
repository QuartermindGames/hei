// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "pl_private.h"

#include <plcore/pl_filesystem.h>
#include <plcore/pl_package.h>

PL_EXTERN_C

PLPackage *PlParseDfsPackage_( PLFile *file );

PLPackage *PlParseWadPackage_( PLFile *file );
PLPackage *PlParseQWadPackage_( PLFile *file );
PLPackage *PlParsePakPackage_( PLFile *file );
PLPackage *PlParseVpkPackage_( PLFile *file );
PLPackage *PlParseMadPackage_( PLFile *file );
PLPackage *PlLoadTabPackage( const char *path );

PLPackage *PlParseFreshBinPackage_( PLFile *file );
PLPackage *PlParseGrpPackage_( PLFile *file );
PLPackage *PlParseOpkPackage_( PLFile *file );     // Outcast
PLPackage *PlParseInuPackage_( PLFile *file );     // White Fear / Inuits
PLPackage *PlParseAllPackage_( PLFile *file );     // The Last Job
PLPackage *PlParseAfsPackage_( PLFile *file );     // Headhunter
PLPackage *PlParseAhfPackage_( PLFile *file );     // Headhunter
PLPackage *PlParseAngelDatPackage_( PLFile *file );// Oni 2, Red Dead Revolver
PLPackage *PlParseHalPackage_( PLFile *file );     // Mortyr
PLPackage *PlParseIce3DDatPackage_( PLFile *file );// Bioshock 3D

#if ( RAR_SUPPORTED == 1 )

PLPackage *PlParseRarPackage_( PLFile *file );

#endif

PL_EXTERN_C_END
