/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#include <plcore/pl.h>
#include <plcore/pl_linkedlist.h>

typedef struct PLLinkedListNode {
	struct PLLinkedListNode *next;
	struct PLLinkedListNode *prev;
	struct PLLinkedList *listParent;
	void *userPtr;
} PLLinkedListNode;

typedef struct PLLinkedList {
	PLLinkedListNode *front;
	PLLinkedListNode *back;
	unsigned int numNodes;
} PLLinkedList;

PLLinkedList *PlCreateLinkedList( void ) {
	return PL_NEW( PLLinkedList );
}

PLLinkedListNode *PlInsertLinkedListNode( PLLinkedList *list, void *userPtr ) {
	PLLinkedListNode *node = PL_NEW( PLLinkedListNode );
	if ( list->front == NULL ) {
		list->front = node;
	}

	node->prev = list->back;
	if ( list->back != NULL ) {
		list->back->next = node;
	}
	list->back = node;
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
	return list->front;
}

PLLinkedListNode *PlGetLastNode( PLLinkedList *list ) {
	return list->back;
}

void *PlGetLinkedListNodeUserData( PLLinkedListNode *node ) {
	return node->userPtr;
}

void PlSetLinkedListNodeUserData( PLLinkedListNode *node, void *userPtr ) {
	node->userPtr = userPtr;
}

/**
 * Destroys the specified node and removes it from the list.
 * Keep in mind this does not free any user data!
 */
void PlDestroyLinkedListNode( PLLinkedListNode *node ) {
	if ( node->prev != NULL ) {
		node->prev->next = node->next;
	}
	if ( node->next != NULL ) {
		node->next->prev = node->prev;
	}

	/* ensure root and back are always pointing to a valid location */
	PLLinkedList *list = node->listParent;
	if ( node == list->front ) {
		list->front = node->next;
	}
	if ( node == list->back ) {
		list->back = node->prev;
	}

	list->numNodes--;

	PL_DELETE( node );
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
	PL_DELETE( list );
}

unsigned int PlGetNumLinkedListNodes( PLLinkedList *list ) {
	return list->numNodes;
}

PLLinkedList *PlGetLinkedListNodeContainer( PLLinkedListNode *node ) {
	return node->listParent;
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

/**
 * Move the given node to the head/start of the list.
 */
void PlMoveLinkedListNodeToFront( PLLinkedListNode *node ) {
	PLLinkedList *list = node->listParent;
	if ( list->front == node ) {
		return;
	}

	if ( list->back == node ) {
		list->back = node->prev;
	}

	if ( node->prev != NULL ) {
		node->prev->next = node->next;
		node->prev = NULL;
	}
	if ( node->next != NULL ) {
		node->next->prev = node->prev;
	}

	node->next = list->front;

	list->front->prev = node;
	list->front = node;
}

/**
 * Move the given node to the tail/end of the list.
 */
void PlMoveLinkedListNodeToBack( PLLinkedListNode *node ) {
	PLLinkedList *list = node->listParent;
	if ( list->back == node ) {
		return;
	}

	if ( list->front == node ) {
		list->front = node->next;
	}

	if ( node->prev != NULL ) {
		node->prev->next = node->next;
	}
	if ( node->next != NULL ) {
		node->next->prev = node->prev;
		node->next = NULL;
	}

	node->prev = list->back;

	list->back->next = node;
	list->back = node;
}

void **PlArrayFromLinkedList( PLLinkedList *list, unsigned int *numElements ) {
	unsigned int i = 0;

	// Allocate container for all the elements
	*numElements = PlGetNumLinkedListNodes( list );
	void **elements = PL_NEW_( void *, *numElements );

	// Now fillerup
	PLLinkedListNode *node = PlGetFirstNode( list );
	while ( node != NULL ) {
		void *data = PlGetLinkedListNodeUserData( node );
		elements[ i++ ] = data;
		node = PlGetNextLinkedListNode( node );
	}
	return elements;
}
