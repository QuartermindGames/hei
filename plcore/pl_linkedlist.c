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

PLLinkedList *plCreateLinkedList( void ) {
	return pl_calloc( 1, sizeof( PLLinkedList ) );
}

PLLinkedListNode *plInsertLinkedListNode( PLLinkedList *list, void *userPtr ) {
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

PLLinkedListNode *plGetNextLinkedListNode( PLLinkedListNode *node ) {
	return node->next;
}

PLLinkedListNode *plGetPrevLinkedListNode( PLLinkedListNode *node ) {
	return node->prev;
}

PLLinkedListNode *plGetFirstNode( PLLinkedList *list ) {
	return list->root;
}

void *plGetLinkedListNodeUserData( PLLinkedListNode *node ) {
	return node->userPtr;
}

/**
 * Destroys the specified node and removes it from the list.
 * Keep in mind this does not free any user data!
 */
void plDestroyLinkedListNode( PLLinkedList *list, PLLinkedListNode *node ) {
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

void plDestroyLinkedListNodes( PLLinkedList *list ) {
	while( list->root != NULL ) { plDestroyLinkedListNode( list, list->root ); }
	list->numNodes = 0;
}

void plDestroyLinkedList( PLLinkedList *list ) {
	if ( list == NULL ) {
		return;
	}

	plDestroyLinkedListNodes( list );
	pl_free( list );
}

unsigned int plGetNumLinkedListNodes( PLLinkedList *list ) {
	return list->numNodes;
}

PLLinkedList *plGetLinkedListNodeContainer( PLLinkedListNode *node ) {
	return node->listParent;
}
