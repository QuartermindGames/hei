// SPDX-License-Identifier: MIT
// Copyright © 2017-2023 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

typedef struct PLPackage PLPackage;
typedef struct PLFile PLFile;

PLPackage *PlParseKriPackage( PLFile *file );
