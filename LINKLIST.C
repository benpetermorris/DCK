/*
 * LINKLIST.C: Functions for implementing a linked-list array.
 *
 * All code is (c) Copyright 1993 Ben Morris and SpeedSOFT Development.
 * Ben Morris Retains all intellectual rights and copyrights to this
 * code.  Under no circumstances is the author's name to be removed from
 * the header file, source file or object code created by compiling the
 * source code.
 *
 * Other than the above restriction, this code may be used freely with
 * any software, commercial or otherwise.
 *
 * Ben Morris also disclaims all warranties, express or implied, with
 * regards to the fitness of this code for any purpose.  Under no
 * circumstances shall Ben Morris or SpeedSOFT Development be held liable
 * for any damages, losses, or claim for loss of profit, resulting from
 * the use or inability to use this software and all accompanying
 * documentation.
 *
 */

#include "linklist.h"

static char *copyright = "Linked Lists by Ben Morris, SSD";

/*---------------------------------------------------------------------------*/

NODEPTR InitList( void )
/*
 * Initializes the linked list.  Returns NULL or a pointer to the
 * head node.
 *
 */
{
    NODEPTR ndp;

    /* Allocate space */
    ndp = malloc( sizeof( NODE ) );

    if( ndp == NULL )
        return NULL;

    /* Now setup the head node */
    ndp->next = ndp;
    ndp->prev = ndp;
    ndp->data = NULL;

    return ndp;
}

/*---------------------------------------------------------------------------*/

NODEPTR NodeInsert( NODEPTR prev_ndp, size_t size )
/*
 * Inserts a node in the linked list AFTER prev_ndp, and allocates its
 * 'data' member with 'size'.
 *
 * RETURNS NULL on fail, or NODEPTR on success.
 *
 */
{
    NODEPTR ndp, tmp = prev_ndp->next;

    /* Allocate space */
    ndp = malloc( sizeof( NODE ) );

    if( ndp == NULL )   /* No space for NODE! */
        return NULL;

    if( size )
    {
        ndp->data = malloc( size );
        if( ndp->data == NULL ) /* No space for DATA! */
        {
            free( ndp );
            return NULL;
        }
    }
    else
        ndp->data = NULL;

    /*
     * Since we're inserting AFTER prev_ndp, we have to set
     * ndp's 'next' to prev_ndp's 'next'.
     *
     * ndp's 'prev' becomes 'prev_ndp'.
     *
     */

    /* If true, prev_ndp is the last node. */
    if( tmp->prev == prev_ndp )
        tmp->prev = ndp;

    ndp->next = prev_ndp->next;
    ndp->prev = prev_ndp;
    prev_ndp->next = ndp;

    return ndp;

}

/*---------------------------------------------------------------------------*/

void NodeDelete( NODEPTR ndp )
/*
 * Deletes 'ndp' from the linked list and frees its storage, if any.
 *
 */
{
    NODEPTR prev = ndp->prev;
    NODEPTR next = ndp->next;

    prev->next = next;
    next->prev = prev;

    if( ndp->data )
        free( ndp->data );
    free( ndp );
}

/*---------------------------------------------------------------------------*/

NODEPTR NodeNumtoPtr( int num, NODEPTR head_ndp )
/*
 * Converts num to the corresponding node in the list, starting at 0.
 *
 */
{
    int     i;
    NODEPTR last = NodeLast( head_ndp ), ndp;

    ndp = NodeNext( head_ndp );

    /*
     * Loop until we hit 'num', or until the current pointer is 'last', in
     * which case we return 0.
     *
     */
    for( i = 0; i <= num; i++ )
    {
        if( i == num )
            return ndp;
        if( ndp == last )
            return NULL;
        ndp = NodeNext( ndp );
    }

    return NULL;

}

/*---------------------------------------------------------------------------*/

int NodePtrtoNum( NODEPTR sndp, NODEPTR head_ndp )
/*
 * Returns ndp's number in the linked list.
 *
 */
{
    int     i;
    NODEPTR last = NodeLast( head_ndp ), ndp;

    ndp = NodeNext( head_ndp );

    /*
     * Loop until ndp == sndp, in which case we return 'i'.  If not found,
     * return -1.
     *
     */
    for( i = 0; ; i++ )
    {
        if( ndp == sndp )
            return i;
        if( ndp == last )
            return -1;
        ndp = NodeNext( ndp );
    }

}

/*---------------------------------------------------------------------------*/

void DestroyList( NODEPTR head_ndp )
/*
 * Destroys the linked list by freeing all allocated memory in every node
 * (including the head node!)
 *
 */
{
    NODEPTR ndp = NodeLast( head_ndp ), prev;
    int     breakit = 0;

    if( ndp == head_ndp )
    {
        free( head_ndp );
        return;
    }

    /*
     * Loop until we hit 'num', or until the current pointer is 'last', in
     * which case we return 0.
     *
     */
    for( ;; )
    {
        /* Get previous node */
        prev = ndp->prev;
        /* If the previous is the head, we break after freeing it. */
        if( prev == head_ndp )
            breakit = 1;
        if( ndp->data )
            free( ndp->data );
        free( ndp );
        if( breakit )
            break;
        ndp = prev;
    }

    free( head_ndp );

}

