/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include "qmos/public/qm_os_memory.h"


#include <plcore/pl.h>
#include <plcore/pl_linkedlist.h>

typedef struct PLLinkedListNode {
	struct PLLinkedListNode *next;
	struct PLLinkedListNode *prev;
	struct PLLinkedList *list;
	void *userPtr;
} PLLinkedListNode;

typedef struct PLLinkedList {
	PLLinkedListNode *front;
	PLLinkedListNode *back;
	unsigned int numNodes;
} PLLinkedList;

PLLinkedList *PlCreateLinkedList( void ) {
	return QM_OS_MEMORY_NEW( PLLinkedList );
}

PLLinkedListNode *PlInsertLinkedListNode( PLLinkedList *list, void *userPtr ) {
	PLLinkedListNode *node = QM_OS_MEMORY_NEW( PLLinkedListNode );
	if ( list->front == NULL ) {
		list->front = node;
	}

	node->prev = list->back;
	if ( list->back != NULL ) {
		list->back->next = node;
	}
	list->back = node;
	node->next = NULL;

	node->list = list;
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
	return list->front;
}

PLLinkedListNode *PlGetLastNode( PLLinkedList *list ) {
	return list->back;
}

void *PlGetLinkedListNodeUserData( PLLinkedListNode *node ) {
	return node->userPtr;
}

/**
 * Destroys the specified node and removes it from the list.
 * Keep in mind this does not free any user data!
 */
void PlDestroyLinkedListNode( PLLinkedListNode *node ) {
	if ( node == NULL ) {
		return;
	}

	if ( node->prev != NULL ) {
		node->prev->next = node->next;
	}
	if ( node->next != NULL ) {
		node->next->prev = node->prev;
	}

	/* ensure root and back are always pointing to a valid location */
	PLLinkedList *list = node->list;
	if ( node == list->front ) {
		list->front = node->next;
	}
	if ( node == list->back ) {
		list->back = node->prev;
	}

	list->numNodes--;

	qm_os_memory_free( node );
}

void PlDestroyLinkedListNodes( PLLinkedList *list ) {
	while ( list->front != NULL ) {
		PlDestroyLinkedListNode( list->front );
	}
	list->numNodes = 0;
}

void PlDestroyLinkedList( PLLinkedList *list ) {
	if ( list == NULL ) {
		return;
	}

	PlDestroyLinkedListNodes( list );
	qm_os_memory_free( list );
}

void PlDestroyLinkedListNodesEx( PLLinkedList *list, void ( *elementDeleter )( void *user ) ) {
	while ( list->front != NULL ) {
		elementDeleter( list->front->userPtr );
		PlDestroyLinkedListNode( list->front );
	}
	list->numNodes = 0;
}

void PlDestroyLinkedListEx( PLLinkedList *list, void ( *elementDeleter )( void *user ) ) {
	if ( list == NULL ) {
		return;
	}

	PlDestroyLinkedListNodesEx( list, elementDeleter );
	PlDestroyLinkedList( list );
}

unsigned int PlGetNumLinkedListNodes( PLLinkedList *list ) {
	return list->numNodes;
}

PLLinkedList *PlGetLinkedListNodeContainer( PLLinkedListNode *node ) {
	return node->list;
}

/**
 * Helper function for iterating through a linked list.
 */
void PlIterateLinkedList( PLLinkedList *linkedList, PLLinkedListIteratorCallback callbackHandler, bool forward ) {
	PLLinkedListNode *listNode = forward ? linkedList->front : linkedList->back;
	while ( listNode != NULL ) {
		void *userData = PlGetLinkedListNodeUserData( listNode );
		listNode = forward ? listNode->next : listNode->prev;

		bool breakEarly = false;
		callbackHandler( userData, &breakEarly );
		if ( breakEarly ) {
			break;
		}
	}
}

bool PlIsLinkedListEmpty( const PLLinkedList *list ) {
	return list->front == NULL;
}
