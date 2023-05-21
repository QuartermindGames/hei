/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl_hashtable.h>

#define HASH_TABLE_SIZE 1024U

typedef struct PLHashTableNode {
	void *key;
	size_t keySize;
	uint64_t hash;
	void *value;
	struct PLHashTableNode *next;
} PLHashTableNode;

typedef struct PLHashTable {
	PLHashTableNode *nodes[ HASH_TABLE_SIZE ];
	unsigned int numNodes;
} PLHashTable;

PLHashTable *PlCreateHashTable( void ) {
	return PL_NEW( PLHashTable );
}

void PlDestroyHashTable( PLHashTable *hashTable ) {
	for ( size_t i = 0; i < HASH_TABLE_SIZE; ++i ) {
		PLHashTableNode *child = hashTable->nodes[ i ];
		while ( child != NULL ) {
			PLHashTableNode *next = child->next;
			PL_DELETE( child->key );
			PL_DELETE( child );
			child = next;
		}
	}
	PL_DELETE( hashTable );
}

#define GET_INDEX( HASH ) ( unsigned int ) ( ( HASH ) % HASH_TABLE_SIZE )

void *PlLookupHashTableUserData( const PLHashTable *hashTable, const void *key, size_t keySize ) {
	assert( key != NULL && keySize != 0 );
	if ( key == NULL || keySize == 0 ) {
		return NULL;
	}

	uint64_t hash = PlGenerateHashFNV1( key, keySize );
	unsigned int index = GET_INDEX( hash );
	for ( PLHashTableNode *node = hashTable->nodes[ index ]; node != NULL; node = node->next ) {
		if ( keySize == node->keySize && memcmp( key, node->key, keySize ) == 0 ) {
			return node->value;
		}
	}

	return NULL;
}

bool PlInsertHashTableNode( PLHashTable *hashTable, const void *key, size_t keySize, void *value ) {
	assert( key != NULL && keySize != 0 );
	if ( key == NULL || keySize == 0 ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "invalid key provided" );
		return false;
	}

	if ( PlLookupHashTableUserData( hashTable, key, keySize ) != NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "key already exists" );
		return false;
	}

	PLHashTableNode *node = PL_NEW( PLHashTableNode );
	node->hash = PlGenerateHashFNV1( key, keySize );
	node->keySize = keySize;
	node->key = PL_NEW_( char, keySize + 1 );
	node->value = value;

	memcpy( node->key, key, node->keySize );

	unsigned int index = GET_INDEX( node->hash );
	node->next = hashTable->nodes[ index ];
	hashTable->nodes[ index ] = node;
	hashTable->numNodes++;

	return true;
}

unsigned int PlGetNumHashTableNodes( const PLHashTable *hashTable ) {
	return hashTable->numNodes;
}

/* iterator */

PLHashTableNode *PlGetFirstHashTableNode( PLHashTable *hashTable ) {
	for ( size_t i = 0; i < HASH_TABLE_SIZE; ++i ) {
		PLHashTableNode *child = hashTable->nodes[ i ];
		if ( child != NULL ) {
			return child;
		}
	}

	return NULL;
}

PLHashTableNode *PlGetNextHashTableNode( PLHashTable *hashTable, PLHashTableNode *hashTableNode ) {
	PLHashTableNode *child = hashTableNode->next;
	if ( child != NULL ) {
		return child;
	}

	unsigned int index = GET_INDEX( hashTableNode->hash ) + 1;
	while ( child == NULL ) {
		if ( index >= HASH_TABLE_SIZE ) {
			break;
		}

		child = hashTable->nodes[ index++ ];
	}

	return child;
}

void *PlGetHashTableNodeUserData( PLHashTableNode *hashTableNode ) {
	return hashTableNode->value;
}
