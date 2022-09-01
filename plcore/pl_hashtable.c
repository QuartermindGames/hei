/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl_hashtable.h>

typedef struct PLHashTableNode {
	char *key;
	uint64_t hash;
	void *value;
} PLHashTableNode;

typedef struct PLHashTable {
	PLHashTableNode *nodes;
	unsigned int numNodes;
	unsigned int maxNodes;
} PLHashTable;

PLHashTable *PlCreateHashTable( void ) {
	static const unsigned int initNodes = 8;
	PLHashTable *hashTable = PL_NEW( PLHashTable );
	hashTable->maxNodes = initNodes;
	hashTable->nodes = PL_NEW_( PLHashTableNode, hashTable->maxNodes );
	return hashTable;
}

void PlDestroyHashTable( PLHashTable *hashTable ) {
	for ( size_t i = 0; i < hashTable->maxNodes; ++i ) {
		PL_DELETE( hashTable->nodes[ i ].key );
	}
	PL_DELETE( hashTable->nodes );
	PL_DELETE( hashTable );
}

#define GET_INDEX( HASH, TABLE ) ( unsigned int ) ( ( HASH ) & ( uint64_t ) ( ( TABLE )->maxNodes - 1 ) )

void *PlLookupHashTableUserData( const PLHashTable *hashTable, const char *key ) {
	uint64_t hash = PlGenerateHashFNV1( key );
	unsigned int index = GET_INDEX( hash, hashTable );

	while ( hashTable->nodes[ index ].key != NULL ) {
		if ( strcmp( key, hashTable->nodes[ index ].key ) == 0 ) {
			return hashTable->nodes[ index ].value;
		}

		index++;
		if ( index >= hashTable->maxNodes ) {
			index = 0;
		}
	}

	return NULL;
}

const PLHashTableNode *PlInsertHashTableNode( PLHashTable *hashTable, const char *key, void *value ) {
	assert( key != NULL );
	if ( key == NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "invalid key provided" );
		return NULL;
	}

	if ( hashTable->numNodes >= hashTable->maxNodes / 2 ) {
		// Expand
	}

	uint64_t hash = PlGenerateHashFNV1( key );
	unsigned int index = GET_INDEX( hash, hashTable );

	while ( hashTable->nodes[ index ].key != NULL ) {
		if ( strcmp( key, hashTable->nodes[ index ].key ) == 0 ) {
			PlReportErrorF( PL_RESULT_INVALID_PARM2, "key already exists in table" );
			return NULL;
		}

		index++;
		if ( index >= hashTable->maxNodes ) {
			index = 0;
		}
	}

	size_t keyLength = strlen( key );
	hashTable->nodes[ index ].key = PL_NEW_( char, keyLength + 1 );
	snprintf( hashTable->nodes[ index ].key, keyLength, "%s", key );
	hashTable->nodes[ index ].value = value;
}

unsigned int PlGetNumHashTableNodes( const PLHashTable *hashTable ) {
	return hashTable->numNodes;
}

void *PlGetHashTableNodeUserData( const PLHashTableNode *hashTableNode ) {
	return hashTableNode->value;
}

const PLHashTableNode *PlGetHashTableNodeByIndex( PLHashTable *hashTable, unsigned int index ) {
	if ( index >= hashTable->numNodes ) {
		return NULL;
	}

	return &hashTable->nodes[ index ];
}
