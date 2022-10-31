/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl_image.h>
#include <plcore/pl_package.h>

PLImage *Blitz_SPT_ParseImage( PLFile *file );
void Blitz_SPT_BulkExport( PLFile *file, const char *destination, const char *format );

PLPackage *Blitz_DAT_LoadPackage( const char *path );
