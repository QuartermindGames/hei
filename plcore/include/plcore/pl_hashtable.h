/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl.h>

PL_EXTERN_C

typedef struct PLHashTable PLHashTable;
typedef struct PLHashTableNode PLHashTableNode;

PLHashTable *PlCreateHashTable( void );
void PlDestroyHashTable( PLHashTable *hashTable );
void *PlLookupHashTableUserData( const PLHashTable *hashTable, const void *key, size_t keySize );
bool PlInsertHashTableNode( PLHashTable *hashTable, const void *key, size_t keySize, void *value );
unsigned int PlGetNumHashTableNodes( const PLHashTable *hashTable );

PLHashTableNode *PlGetFirstHashTableNode( PLHashTable *hashTable );
PLHashTableNode *PlGetNextHashTableNode( PLHashTable *hashTable, PLHashTableNode *hashTableNode );
void *PlGetHashTableNodeUserData( PLHashTableNode *hashTableNode );

PL_EXTERN_C_END
