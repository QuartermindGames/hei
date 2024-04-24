/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl.h>

PL_EXTERN_C

typedef struct PLHashTable PLHashTable;
typedef struct PLHashTableNode PLHashTableNode;

PLHashTable *PlCreateHashTable( void );
void PlDestroyHashTable( PLHashTable *hashTable );
void PlDestroyHashTableEx( PLHashTable *hashTable, void ( *elementDeleter )( void *user ) );
void PlClearHashTable( PLHashTable *hashTable );
void *PlLookupHashTableUserData( const PLHashTable *hashTable, const void *key, size_t keySize );
PLHashTableNode *PlInsertHashTableNode( PLHashTable *hashTable, const void *key, size_t keySize, void *value );
unsigned int PlGetNumHashTableNodes( const PLHashTable *hashTable );

PLHashTableNode *PlGetFirstHashTableNode( PLHashTable *hashTable );
PLHashTableNode *PlGetNextHashTableNode( PLHashTable *hashTable, PLHashTableNode *hashTableNode );
void *PlGetHashTableNodeUserData( PLHashTableNode *hashTableNode );
void PlSetHashTableNodeUserData( PLHashTableNode *hashTableNode, void *value );

PL_EXTERN_C_END
