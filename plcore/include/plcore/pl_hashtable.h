/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl.h>

PL_EXTERN_C

typedef struct PLHashTable PLHashTable;
typedef struct PLHashTableNode PLHashTableNode;

PLHashTable *PlCreateHashTable( void );
void PlDestroyHashTable( PLHashTable *hashTable );
void *PlLookupHashTableUserData( const PLHashTable *hashTable, const char *key );
const PLHashTableNode *PlInsertHashTableNode( PLHashTable *hashTable, const char *key, void *value );
unsigned int PlGetNumHashTableNodes( const PLHashTable *hashTable );
void *PlGetHashTableNodeUserData( const PLHashTableNode *hashTableNode );
const PLHashTableNode *PlGetHashTableNodeByIndex( PLHashTable *hashTable, unsigned int index );

PL_EXTERN_C_END
