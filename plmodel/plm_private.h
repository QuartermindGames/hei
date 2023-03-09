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

PLMModel *PlmDeserializePly( PLFile *file );

// old api design, this is being deprecated
PLMModel *PlmLoadHdvModel( const char *path );
PLMModel *PlmLoadU3DModel( const char *path );
PLMModel *PlmLoadObjModel( const char *path );
PLMModel *PlmLoadRequiemModel( const char *path );
PLMModel *PlmLoadSmdModel( const char *path );

bool PlmWriteSmdModel( PLMModel *model, const char *path );
bool PlmWriteObjModel( PLMModel *model, const char *path );

PL_EXTERN_C_END
