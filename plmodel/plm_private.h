// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>
#include <plcore/pl_console.h>

#include <plmodel/plm.h>

extern int LOG_LEVEL_MODEL;
#define ModelLog( ... ) PlLogMessage( LOG_LEVEL_MODEL, __VA_ARGS__ )

PL_EXTERN_C

PLMModel *PlmParsePlyModel( PLFile *file );
PLMModel *PlmParseU3dModel( PLFile *file );
PLMModel *PlmParseObjModel( PLFile *file );
PLMModel *PlmLoadSmdModel( const char *path );

PL_EXTERN_C_END
