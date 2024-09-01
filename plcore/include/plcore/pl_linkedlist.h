// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>

PL_EXTERN_C

typedef struct PLLinkedList PLLinkedList;
typedef struct PLLinkedListNode PLLinkedListNode;

/**
 * Creates an instance of a linked list container.
 *
 * @return	Pointer to the new linked list container.
 */
PLLinkedList *PlCreateLinkedList( void );

/**
 * Inserts a new node into the specified linked list.
 *
 * @param list		List to add the node to.
 * @param userPtr 	A pointer to the data you want attached to the node.
 * @return 			A pointer to the new node in the list.
 */
PLLinkedListNode *PlInsertLinkedListNode( PLLinkedList *list, void *userPtr );

/**
 * Inserts a new node into the start of the specified linked list.
 *
 * @param list 		List to add the node to.
 * @param userPtr 	A pointer to the data you want attached to the node.
 * @return 			A pointer to the new node in the list.
 */
PLLinkedListNode *PlInsertFrontLinkedListNode( PLLinkedList *list, void *userPtr );

/**
 * Destroys the specified linked list. Doesn't free user data.
 *
 * @param list	List to destroy.
 */
void PlDestroyLinkedList( PLLinkedList *list );

/**
 * An extended variation of the destroy function, allowing for deletion of user data.
 *
 * @param list 				List to destroy.
 * @param elementDeleter 	A callback for your destructor for user data.
 */
void PlDestroyLinkedListEx( PLLinkedList *list, void ( *elementDeleter )( void *user ) );

/**
 * Destroys an individual node and automatically removes it from the parent list.
 *
 * @param node	Node to destroy.
 */
void PlDestroyLinkedListNode( PLLinkedListNode *node );

/**
 * Destroys all the nodes under the linked list. Doesn't free user data.
 *
 * @param list	List to clear.
 */
void PlDestroyLinkedListNodes( PLLinkedList *list );

/**
 * Destroys all the nodes under the linked list, allowing for deletion of user data.
 *
 * @param list 				List to clear.
 * @param elementDeleter 	A callback for your destructor for user data.
 */
void PlDestroyLinkedListNodesEx( PLLinkedList *list, void ( *elementDeleter )( void *user ) );

PLLinkedListNode *PlGetNextLinkedListNode( PLLinkedListNode *node );
PLLinkedListNode *PlGetPrevLinkedListNode( PLLinkedListNode *node );
PLLinkedListNode *PlGetFirstNode( PLLinkedList *list );
PLLinkedListNode *PlGetLastNode( PLLinkedList *list );

/**
 * Returns the user data for the given node.
 *
 * @param node	Node to fetch the user data from.
 * @return 		A pointer to the user data.
 */
void *PlGetLinkedListNodeUserData( PLLinkedListNode *node );

/**
 * Set the user data for the given node to something else.
 *
 * @param node 		Node to set user data.
 * @param userPtr 	A pointer to your user data to attach to the node.
 */
void PlSetLinkedListNodeUserData( PLLinkedListNode *node, void *userPtr );

/**
 * Returns the number of nodes under the list.
 *
 * @param list 	The list to query.
 * @return 		The number of nodes under the given list.
 */
unsigned int PlGetNumLinkedListNodes( PLLinkedList *list );

/**
 * Returns the parent list for a given node.
 *
 * @param node 	A pointer to the specific node you want the parent of.
 * @return 		The parent list of the given node.
 */
PLLinkedList *PlGetLinkedListNodeContainer( PLLinkedListNode *node );

typedef void ( *PLLinkedListIteratorCallback )( void *userData, bool *breakEarly );
void PlIterateLinkedList( PLLinkedList *linkedList, PLLinkedListIteratorCallback callbackHandler, bool forward );

/**
 * Moves the given node to the front of the list.
 *
 * @param node	The node to move.
 */
void PlMoveLinkedListNodeToFront( PLLinkedListNode *node );

/**
 * Moves the given node to the back of the list.
 *
 * @param node	The node to move.
 */
void PlMoveLinkedListNodeToBack( PLLinkedListNode *node );

void **PlArrayFromLinkedList( PLLinkedList *list, unsigned int *numElements );

/**
 * A simple iterator macro, can be used like so...
 * 	MyType myVar;
 * 	PL_ITERATE_LINKED_LIST( myVar, MyType, myList ) { logic goes here }
 */
#define PL_ITERATE_LINKED_LIST( VAR, TYPE, LIST )                                             \
	for ( PLLinkedListNode *i = PlGetFirstNode( LIST );                                       \
	      i != NULL && ( ( ( VAR ) = ( TYPE * ) PlGetLinkedListNodeUserData( i ) ) != NULL ); \
	      i = PlGetNextLinkedListNode( i ) )

PL_EXTERN_C_END
