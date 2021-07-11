/**
 * Hei Platform Library
 * Copyright (C) 2017-2021 Mark E Sowden <hogsy@oldtimes-software.com>
 * This software is licensed under MIT. See LICENSE for more details.
 */

#include <plcore/pl.h>

typedef struct PLLinkedListNode {
	struct PLLinkedListNode *next;
	struct PLLinkedListNode *prev;
	struct PLLinkedList *listParent;
	void *userPtr;
} PLLinkedListNode;

typedef struct PLLinkedList {
	PLLinkedListNode *root;
	PLLinkedListNode *ceiling;
	unsigned int numNodes;
} PLLinkedList;

PLLinkedList *PlCreateLinkedList( void ) {
	return pl_calloc( 1, sizeof( PLLinkedList ) );
}

PLLinkedListNode *PlInsertLinkedListNode( PLLinkedList *list, void *userPtr ) {
	PLLinkedListNode *node = pl_malloc( sizeof( PLLinkedListNode ) );
	if( list->root == NULL ) {
		list->root = node;
	}

	node->prev = list->ceiling;
	if( list->ceiling != NULL ) {
		list->ceiling->next = node;
	}
	list->ceiling = node;
	node->next = NULL;

	node->listParent = list;
	node->userPtr = userPtr;

	list->numNodes++;

	return node;
}

PLLinkedListNode *PlGetNextLinkedListNode( PLLinkedListNode *node ) {
	return node->next;
}

PLLinkedListNode *PlGetPrevLinkedListNode( PLLinkedListNode *node ) {
	return node->prev;
}

PLLinkedListNode *PlGetFirstNode( PLLinkedList *list ) {
	return list->root;
}

void *PlGetLinkedListNodeUserData( PLLinkedListNode *node ) {
	return node->userPtr;
}

/**
 * Destroys the specified node and removes it from the list.
 * Keep in mind this does not free any user data!
 */
void PlDestroyLinkedListNode( PLLinkedList *list, PLLinkedListNode *node ) {
	if( node->prev != NULL ) {
		node->prev->next = node->next;
	}

	if( node->next != NULL ) {
		node->next->prev = node->prev;
	}

	/* ensure root and ceiling are always pointing to a valid location */
	if( node == list->root ) {
		list->root = node->next;
	}
	if( node == list->ceiling ) {
		list->ceiling = node->prev;
	}

	list->numNodes--;

	pl_free( node );
}

void PlDestroyLinkedListNodes( PLLinkedList *list ) {
	while( list->root != NULL ) { PlDestroyLinkedListNode( list, list->root ); }
	list->numNodes = 0;
}

void PlDestroyLinkedList( PLLinkedList *list ) {
	if ( list == NULL ) {
		return;
	}

	PlDestroyLinkedListNodes( list );
	pl_free( list );
}

unsigned int PlGetNumLinkedListNodes( PLLinkedList *list ) {
	return list->numNodes;
}

PLLinkedList *PlGetLinkedListNodeContainer( PLLinkedListNode *node ) {
	return node->listParent;
}
