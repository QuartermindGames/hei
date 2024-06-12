// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#define PL_COMPILE_PLUGIN 1
#include <plgraphics/plg_driver_interface.h>

extern const PLGDriverExportTable *gInterface;
extern PLGDriverImportTable graphicsInterface;

extern int glLogLevel;
