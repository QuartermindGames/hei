// SPDX-License-Identifier: MIT
// Hei Platform Library
// Copyright © 2017-2024 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include <plcore/pl.h>

PL_EXTERN_C

typedef struct PLLinkedList     PLLinkedList;
typedef struct PLLinkedListNode PLLinkedListNode;

/**
 * Creates an instance of a linked list container.
 *
 * @return	Pointer to the new linked list container.
 */
PL_DEPRECATED( PLLinkedList *PlCreateLinkedList( void ) );

/**
 * Inserts a new node into the specified linked list.
 *
 * @param list		List to add the node to.
 * @param userPtr 	A pointer to the data you want attached to the node.
 * @return 			A pointer to the new node in the list.
 */
PL_DEPRECATED( PLLinkedListNode *PlInsertLinkedListNode( PLLinkedList *list, void *userPtr ) );

/**
 * Destroys the specified linked list. Doesn't free user data.
 *
 * @param list	List to destroy.
 */
PL_DEPRECATED( void PlDestroyLinkedList( PLLinkedList *list ) );

/**
 * An extended variation of the destroy function, allowing for deletion of user data.
 *
 * @param list 				List to destroy.
 * @param elementDeleter 	A callback for your destructor for user data.
 */
PL_DEPRECATED( void PlDestroyLinkedListEx( PLLinkedList *list, void ( *elementDeleter )( void *user ) ) );

/**
 * Destroys an individual node and automatically removes it from the parent list.
 *
 * @param node	Node to destroy.
 */
PL_DEPRECATED( void PlDestroyLinkedListNode( PLLinkedListNode *node ) );

/**
 * Destroys all the nodes under the linked list. Doesn't free user data.
 *
 * @param list	List to clear.
 */
PL_DEPRECATED( void PlDestroyLinkedListNodes( PLLinkedList *list ) );

/**
 * Destroys all the nodes under the linked list, allowing for deletion of user data.
 *
 * @param list 				List to clear.
 * @param elementDeleter 	A callback for your destructor for user data.
 */
PL_DEPRECATED( void PlDestroyLinkedListNodesEx( PLLinkedList *list, void ( *elementDeleter )( void *user ) ) );

PL_DEPRECATED( PLLinkedListNode *PlGetNextLinkedListNode( PLLinkedListNode *node ) );
PL_DEPRECATED( PLLinkedListNode *PlGetPrevLinkedListNode( PLLinkedListNode *node ) );
PL_DEPRECATED( PLLinkedListNode *PlGetFirstNode( PLLinkedList *list ) );
PL_DEPRECATED( PLLinkedListNode *PlGetLastNode( PLLinkedList *list ) );

/**
 * Returns the user data for the given node.
 *
 * @param node	Node to fetch the user data from.
 * @return 		A pointer to the user data.
 */
PL_DEPRECATED( void *PlGetLinkedListNodeUserData( PLLinkedListNode *node ) );

/**
 * Returns the number of nodes under the list.
 *
 * @param list 	The list to query.
 * @return 		The number of nodes under the given list.
 */
PL_DEPRECATED( unsigned int PlGetNumLinkedListNodes( PLLinkedList *list ) );

/**
 * Returns the parent list for a given node.
 *
 * @param node 	A pointer to the specific node you want the parent of.
 * @return 		The parent list of the given node.
 */
PL_DEPRECATED( PLLinkedList *PlGetLinkedListNodeContainer( PLLinkedListNode *node ) );

typedef void ( *PLLinkedListIteratorCallback )( void *userData, bool *breakEarly );
PL_DEPRECATED( void PlIterateLinkedList( PLLinkedList *linkedList, PLLinkedListIteratorCallback callbackHandler, bool forward ) );

/**
 * @brief Checks whether the linked list is empty.
 *
 * @param list 	The linked list to check.
 * @return 		true if the linked list is empty, false otherwise.
 */
PL_DEPRECATED( bool PlIsLinkedListEmpty( const PLLinkedList *list ) );

/**
 * A simple iterator macro, can be used like so...
 * 	MyType myVar;
 * 	PL_ITERATE_LINKED_LIST( myVar, MyType, myList ) { logic goes here }
 */
#define PL_ITERATE_LINKED_LIST( VAR, TYPE, LIST, ITR )                                                                                                     \
	for ( PLLinkedListNode * ( ITR##_ ), *( ITR ) = PlGetFirstNode( LIST );                                                                                \
	      ( ITR ) != NULL && ( ( ITR##_ ) = PlGetNextLinkedListNode( ( ITR ) ), ( ( ( VAR ) = ( TYPE * ) PlGetLinkedListNodeUserData( ITR ) ) != NULL ) ); \
	      ( ITR ) = ( ITR##_ ) )

PL_EXTERN_C_END
