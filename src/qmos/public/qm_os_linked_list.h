// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>

#pragma once

#include "qm_os.h"

/////////////////////////////////////////////////////////////////////////////////////
// Linked List
/////////////////////////////////////////////////////////////////////////////////////

#if defined( __cplusplus )
extern "C"
{
#endif

	/**
	 * Linked list instance.
	 */
	typedef struct QmOsLinkedList QmOsLinkedList;

	/**
	 * Item under the linked list.
	 */
	typedef struct QmOsLinkedListNode QmOsLinkedListNode;

	/**
	 * Allocate a new linked list.
	 * Call memory_free to destroy (will auto-destroy nodes).
	 * @return A new linked list instance.
	 */
	QmOsLinkedList *qm_os_linked_list_create();

	/**
	 * Clears all elements under the linked list.
	 * Size will be zero after calling.
	 * @param self Linked list instance.
	 */
	void qm_os_linked_list_clear( QmOsLinkedList *self );

	/**
	 * Fetch the number of elements under a linked list.
	 * @param self Linked list instance.
	 * @return Number of elements in the linked list.
	 */
	size_t qm_os_linked_list_get_size( const QmOsLinkedList *self );

	/**
	 * Pushes the given data to the back of the list, via a node.
	 * Call memory_free to destroy the given node (note this doesn't free the data under the node).
	 * @param self Linked list instance.
	 * @param data Data to attach.
	 * @return Node inside of the list.
	 */
	QmOsLinkedListNode *qm_os_linked_list_push_back( QmOsLinkedList *self, void *data );

	/**
	 * Pushes the given data to the front of the list, via a node.
	 * Call memory_free to destroy the given node (note this doesn't free the data under the node).
	 * @param self Linked list instance.
	 * @param data Data to attach.
	 * @return Node inside of the list.
	 */
	QmOsLinkedListNode *qm_os_linked_list_push_front( QmOsLinkedList *self, void *data );

	QmOsLinkedListNode *qm_os_linked_list_get_front( const QmOsLinkedList *self );
	QmOsLinkedListNode *qm_os_linked_list_get_back( const QmOsLinkedList *self );

	QmOsLinkedListNode *qm_os_linked_list_node_get_next( const QmOsLinkedListNode *self );
	QmOsLinkedListNode *qm_os_linked_list_node_get_prev( const QmOsLinkedListNode *self );
	void                qm_os_linked_list_node_set_data( QmOsLinkedListNode *self, void *data );
	void               *qm_os_linked_list_node_get_data( const QmOsLinkedListNode *self );

#define QM_OS_LINKED_LIST_ITERATE( VAR, LIST, ITR )                                                                                                                           \
	for ( QmOsLinkedListNode * ( ITR##_ ), *( ITR ) = qm_os_linked_list_get_front( LIST );                                                                                    \
	      ( ITR ) != NULL && ( ( ITR##_ ) = qm_os_linked_list_node_get_next( ( ITR ) ), ( ( ( VAR ) = ( typeof( VAR ) ) qm_os_linked_list_node_get_data( ITR ) ) != NULL ) ); \
	      ( ITR ) = ( ITR##_ ) )

#if defined( __cplusplus )
};
#endif
