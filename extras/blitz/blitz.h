// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl_plugin_interface.h>
extern const PLPluginExportTable *gInterface;

extern const char *titanStrings[];
extern const unsigned int numTitanStrings;

const char *get_string_for_hash( uint32_t hash, const char **strings, unsigned int numStrings );
