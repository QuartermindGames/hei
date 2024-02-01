// SPDX-License-Identifier: MIT
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

typedef struct PLPackage PLPackage;
typedef struct PLImage PLImage;
typedef struct PLFile PLFile;

PLPackage *PlParseOkreWadPackage( PLFile *file );
PLImage *PlParseOkreTexture( PLFile *file );
