// Copyright © 2017-2025 Quartermind Games, Mark E. Sowden <hogsy@snortysoft.net>
// Purpose: Linked list implementation.
// Author:  Mark E. Sowden

#include "qmos/public/qm_os_linked_list.h"
#include "qmos/public/qm_os_memory.h"

#include <assert.h>

typedef struct QmOsLinkedListNode
{
	QmOsLinkedListNode *next;
	QmOsLinkedListNode *prev;
	void               *data;

	QmOsLinkedList *parent;
} QmOsLinkedListNode;

typedef struct QmOsLinkedList
{
	QmOsLinkedListNode *front;
	QmOsLinkedListNode *back;
	size_t              size;
} QmOsLinkedList;

QmOsLinkedList *qm_os_linked_list_create()
{
	return QM_OS_MEMORY_NEW( QmOsLinkedList );
}

void qm_os_linked_list_destroy( QmOsLinkedList *self )
{
}

size_t qm_os_linked_list_get_size( const QmOsLinkedList *self )
{
	assert( self != nullptr );
	return self->size;
}

QmOsLinkedListNode *qm_os_linked_list_push_back( QmOsLinkedList *self, void *data )
{
	assert( self != nullptr );

	QmOsLinkedListNode *node = QM_OS_MEMORY_NEW( QmOsLinkedListNode );
	if ( node == nullptr )
	{
		return nullptr;
	}

	node->data   = data;
	node->parent = self;

	if ( self->front == nullptr )
	{
		self->front = node;
	}

	node->prev = self->back;
	if ( self->back != nullptr )
	{
		self->back->next = node;
	}

	self->back = node;
	self->size++;

	return node;
}

QmOsLinkedListNode *qm_os_linked_list_push_front( QmOsLinkedList *self, void *data )
{
	assert( self != nullptr );

	QmOsLinkedListNode *node = QM_OS_MEMORY_NEW( QmOsLinkedListNode );
	if ( node == nullptr )
	{
		return nullptr;
	}

	node->data   = data;
	node->parent = self;

	if ( self->front == nullptr )
	{
		self->front = node;
		self->back  = node;
	}
	else
	{
		node->next        = self->front;
		self->front->prev = node;
		self->front       = node;
	}

	self->size++;

	return node;
}

QmOsLinkedListNode *qm_os_linked_list_get_front( const QmOsLinkedList *self )
{
	assert( self != nullptr );
	return self->front;
}

QmOsLinkedListNode *qm_os_linked_list_get_back( const QmOsLinkedList *self )
{
	assert( self != nullptr );
	return self->back;
}

QmOsLinkedListNode *qm_os_linked_list_node_get_next( const QmOsLinkedListNode *self )
{
	assert( self != nullptr );
	return self->next;
}

QmOsLinkedListNode *qm_os_linked_list_node_get_prev( const QmOsLinkedListNode *self )
{
	assert( self != nullptr );
	return self->prev;
}

void qm_os_linked_list_node_set_data( QmOsLinkedListNode *self, void *data )
{
	assert( self != nullptr );
	self->data = data;
}

void *qm_os_linked_list_node_get_data( const QmOsLinkedListNode *self )
{
	assert( self != nullptr );
	return self->data;
}
