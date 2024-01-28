/* SPDX-License-Identifier: MIT */
/* Copyright © 2017-2022 Mark E Sowden <hogsy@oldtimes-software.com> */

#pragma once

#include <plcore/pl.h>

PL_EXTERN_C

typedef struct PLLinkedList PLLinkedList;
typedef struct PLLinkedListNode PLLinkedListNode;

PLLinkedList *PlCreateLinkedList( void );

PLLinkedListNode *PlInsertLinkedListNode( PLLinkedList *list, void *userPtr );
PLLinkedListNode *PlInsertFrontLinkedListNode( PLLinkedList *list, void *userPtr );

void PlDestroyLinkedList( PLLinkedList *list );
void PlDestroyLinkedListEx( PLLinkedList *list, void ( *elementDeleter )( void *user ) );
void PlDestroyLinkedListNode( PLLinkedListNode *node );
void PlDestroyLinkedListNodes( PLLinkedList *list );
void PlDestroyLinkedListNodesEx( PLLinkedList *list, void ( *elementDeleter )( void *user ) );

PLLinkedListNode *PlGetNextLinkedListNode( PLLinkedListNode *node );
PLLinkedListNode *PlGetPrevLinkedListNode( PLLinkedListNode *node );
PLLinkedListNode *PlGetFirstNode( PLLinkedList *list );
PLLinkedListNode *PlGetLastNode( PLLinkedList *list );
void *PlGetLinkedListNodeUserData( PLLinkedListNode *node );
void PlSetLinkedListNodeUserData( PLLinkedListNode *node, void *userPtr );

unsigned int PlGetNumLinkedListNodes( PLLinkedList *list );

PLLinkedList *PlGetLinkedListNodeContainer( PLLinkedListNode *node );

typedef void ( *PLLinkedListIteratorCallback )( void *userData, bool *breakEarly );
void PlIterateLinkedList( PLLinkedList *linkedList, PLLinkedListIteratorCallback callbackHandler, bool forward );

void PlMoveLinkedListNodeToFront( PLLinkedListNode *node );
void PlMoveLinkedListNodeToBack( PLLinkedListNode *node );

void **PlArrayFromLinkedList( PLLinkedList *list, unsigned int *numElements );

PL_EXTERN_C_END
