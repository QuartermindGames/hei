/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
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
