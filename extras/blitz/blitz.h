/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl_plugin_interface.h>
extern const PLPluginExportTable *gInterface;

extern const char *titanStrings[];
extern const unsigned int numTitanStrings;

const char *get_string_for_hash( uint32_t hash, const char **strings, unsigned int numStrings );
