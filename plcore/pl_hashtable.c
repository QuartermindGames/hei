/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl_hashtable.h>

#define HASH_TABLE_SIZE 1024U

typedef struct PLHashTableNode {
	void *key;
	size_t keySize;
	uint64_t hash;
	void *value;
	PLHashTable *table;
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
	if ( hashTable == NULL ) {
		return;
	}

	PlClearHashTable( hashTable );

	PL_DELETE( hashTable );
}

void PlDestroyHashTableEx( PLHashTable *hashTable, void ( *elementDeleter )( void *user ) ) {
	if ( hashTable == NULL ) {
		return;
	}

	for ( size_t i = 0; i < HASH_TABLE_SIZE; ++i ) {
		PLHashTableNode *child = hashTable->nodes[ i ];
		while ( child != NULL ) {
			PLHashTableNode *next = child->next;
			if ( elementDeleter != NULL ) {
				elementDeleter( child->value );
			}
			PL_DELETE( child->key );
			PL_DELETE( child );
			child = next;
		}
	}

	PL_DELETE( hashTable );
}

#define GET_INDEX( HASH ) ( unsigned int ) ( ( HASH ) % HASH_TABLE_SIZE )

void PlClearHashTable( PLHashTable *hashTable ) {
	if ( hashTable->numNodes == 0 ) {
		return;
	}

	for ( size_t i = 0; i < HASH_TABLE_SIZE; ++i ) {
		PLHashTableNode *child = hashTable->nodes[ i ];
		while ( child != NULL ) {
			PLHashTableNode *next = child->next;
			PL_DELETEN( child->key );
			PL_DELETEN( child );
			child = next;
		}
		hashTable->nodes[ i ] = NULL;
	}

	hashTable->numNodes = 0;
}

PLHashTableNode *PlLookupHashTableNode( const PLHashTable *hashTable, const void *key, size_t keySize ) {
	if ( key == NULL || keySize == 0 ) {
		return NULL;
	}

	uint64_t hash = PlGenerateHashFNV1( key, keySize );
	unsigned int index = GET_INDEX( hash );
	for ( PLHashTableNode *node = hashTable->nodes[ index ]; node != NULL; node = node->next ) {
		if ( keySize == node->keySize && memcmp( key, node->key, keySize ) == 0 ) {
			return node;
		}
	}

	return NULL;
}

void *PlLookupHashTableUserData( const PLHashTable *hashTable, const void *key, size_t keySize ) {
	PLHashTableNode *node = PlLookupHashTableNode( hashTable, key, keySize );
	if ( node == NULL ) {
		return NULL;
	}

	return node->value;
}

PLHashTableNode *PlInsertHashTableNode( PLHashTable *hashTable, const void *key, size_t keySize, void *value ) {
	assert( key != NULL && keySize != 0 );
	if ( key == NULL || keySize == 0 ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "invalid key provided" );
		return NULL;
	}

	if ( PlLookupHashTableUserData( hashTable, key, keySize ) != NULL ) {
		PlReportErrorF( PL_RESULT_INVALID_PARM2, "key already exists" );
		return NULL;
	}

	PLHashTableNode *node = PL_NEW( PLHashTableNode );
	node->hash = PlGenerateHashFNV1( key, keySize );
	node->value = value;
	node->table = hashTable;

	node->keySize = keySize;
	node->key = PL_NEW_( char, keySize + 1 );
	memcpy( node->key, key, node->keySize );

	unsigned int index = GET_INDEX( node->hash );
	node->next = hashTable->nodes[ index ];
	hashTable->nodes[ index ] = node;
	hashTable->numNodes++;

	return node;
}

void PlDestroyHashTableNode( PLHashTableNode *hashTableNode ) {
	if ( hashTableNode == NULL ) {
		return;
	}

	PLHashTable *hashTable = hashTableNode->table;
	unsigned int index = GET_INDEX( hashTableNode->hash );

	PLHashTableNode **pptr = &( hashTable->nodes[ index ] );
	while ( *pptr != hashTableNode ) {
		assert( *pptr != NULL );
		pptr = &( ( *pptr )->next );
	}

	*pptr = hashTableNode->next;
	hashTable->numNodes--;

	PL_DELETE( hashTableNode->key );
	PL_DELETE( hashTableNode );
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

PLHashTableNode *PlGetNextHashTableNode( PLHashTableNode *hashTableNode ) {
	PLHashTableNode *child = hashTableNode->next;
	if ( child != NULL ) {
		return child;
	}

	unsigned int index = GET_INDEX( hashTableNode->hash ) + 1;
	while ( child == NULL ) {
		if ( index >= HASH_TABLE_SIZE ) {
			break;
		}

		child = hashTableNode->table->nodes[ index++ ];
	}

	return child;
}

void *PlGetHashTableNodeUserData( PLHashTableNode *hashTableNode ) {
	return hashTableNode->value;
}

void PlSetHashTableNodeUserData( PLHashTableNode *hashTableNode, void *value ) {
	hashTableNode->value = value;
}
