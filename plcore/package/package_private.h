// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "pl_private.h"

#include <plcore/pl_filesystem.h>
#include <plcore/pl_package.h>

PL_EXTERN_C

QmFsPackage *PlParseDfsPackage_( QmFsFile *file );

QmFsPackage *PlParseWadPackage_( QmFsFile *file );
QmFsPackage *PlParseQWadPackage_( QmFsFile *file );
QmFsPackage *PlParsePakPackage_( QmFsFile *file );
QmFsPackage *PlParseVpkPackage_( QmFsFile *file );
QmFsPackage *PlParseMadPackage_( QmFsFile *file );
QmFsPackage *PlLoadTabPackage( const char *path );

QmFsPackage *PlParseFreshBinPackage_( QmFsFile *file );
QmFsPackage *PlParseGrpPackage_( QmFsFile *file );
QmFsPackage *PlParseOpkPackage_( QmFsFile *file );     // Outcast
QmFsPackage *PlParseInuPackage_( QmFsFile *file );     // White Fear / Inuits
QmFsPackage *PlParseAllPackage_( QmFsFile *file );     // The Last Job
QmFsPackage *PlParseAfsPackage_( QmFsFile *file );     // Headhunter
QmFsPackage *PlParseAhfPackage_( QmFsFile *file );     // Headhunter
QmFsPackage *PlParseAngelDatPackage_( QmFsFile *file );// Oni 2, Red Dead Revolver
QmFsPackage *PlParseHalPackage_( QmFsFile *file );     // Mortyr
QmFsPackage *PlParseIce3DDatPackage_( QmFsFile *file );// Bioshock 3D
QmFsPackage *PlParseFrdPakPackage_( QmFsFile *file );  // TimeSplitters

#if ( RAR_SUPPORTED == 1 )

QmFsPackage *PlParseRarPackage_( QmFsFile *file );

#endif

PL_EXTERN_C_END
