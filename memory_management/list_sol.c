#include "list_sol.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>

static _Bool doSinglePassOnSort(struct nodeStruct **headRef);
static void swapElements(struct nodeStruct **previous, struct nodeStruct *nodeA, struct nodeStruct *b);


/*
 * Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value size. Return a pointer to the new node.
 */
struct nodeStruct* List_createNode(int size, void *ptr)
{
	struct nodeStruct *pNode = malloc(sizeof(struct nodeStruct));
	if (pNode != NULL) {
		pNode->size = size;
        pNode->ptr = ptr;
	}
	return pNode;
}

/*
 * Insert node at the head of the list.
 */
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node)
{
	node->next = *headRef;
	*headRef = node;
}

/*
 * Insert node after the tail of the list.
 */
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node)
{
	node->next = NULL;

	// Handle empty list
	if (*headRef == NULL) {
		*headRef = node;
	}
	else {
		// Find the tail and insert node
		struct nodeStruct *current = *headRef;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = node;
	}
}

/*
 * Count number of nodes in the list.
 * Return 0 if the list is empty, i.e., head == NULL
 */
int List_countNodes (struct nodeStruct *head)
{
	int count = 0;
	struct nodeStruct *current = head;
	while (current != NULL) {
		current = current->next;
		count++;
	}
	return count;
}

/*
 * Return the first node holding the correct pointer, return NULL if none found
 */
struct nodeStruct* List_findNode(struct nodeStruct *head, void *ptr)
{
	struct nodeStruct *current = head;
	while (current != NULL) {
		if (current->ptr == ptr) {
			return current;
		}
		current = current->next;
	}
	return NULL;
}

/*
 * Delete node from the list and free memory allocated to it.
 * This function assumes that node has been properly set (by for example
 * calling List_findNode()) to a valid node in the list. If the list contains
 * only node, the head of the list should be set to NULL.
 */
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node)
{
	assert(headRef != NULL);
	assert(*headRef != NULL);

	// Is it the first element?
	if (*headRef == node) {
		*headRef = node->next;
	}
	else {
		// Find the previous node:
		struct nodeStruct *previous = *headRef;
		while (previous->next != node) {
			previous = previous->next;
			assert(previous != NULL);
		}

		// Unlink node:
		assert(previous->next == node);
		previous->next = node->next;
	}

	// Free memory:
	free(node);
}


/* KENNY: ADDED THIS ONE MYSELF!
 * This function is just to remove clutter, because
 * I continually used this same chunk of code over and over. 
 */
void* allocate_node(struct nodeStruct **freeBlocks, struct nodeStruct **allocatedBlocks, struct nodeStruct *freeNode, int _size){
    void* ptr = freeNode->ptr;
    struct nodeStruct *allocatedNode = List_createNode(_size, ptr);

    /* Decrease the free node's size accordingly */
    freeNode->size -= _size;
    freeNode->ptr = (void*)((char*)freeNode->ptr + _size); 
    if (freeNode->size == 0){ 
        List_deleteNode(freeBlocks, freeNode);
    }   

    /* Add the new allocated node to the allocatedBlocks */
    List_insertHead(allocatedBlocks, allocatedNode);
    
    return ptr;
}



/* KENNY: ADDED THIS ONE MYSELF!
 * Delete all nodes in the linked list.
 */
void List_deleteAll(struct nodeStruct **headRef){
    assert(headRef != NULL);
    assert(*headRef != NULL);

    struct nodeStruct* node = *headRef;

    while(node != NULL){
        *headRef = node->next;
        free(node);
    
        node = *headRef;
    }
    *headRef = NULL;
} 

/* KENNY: ADDED THIS ONE MYSELF!
 * Used when aalgorithm is FIRST_FIT. Finds the first block that
 * is larger than or equal to minsize. Returns NULL if none found. */
struct nodeStruct* List_findFirstFit (struct nodeStruct *head, int minSize){
    struct nodeStruct *current = head;  

    while (current != NULL){
        if (current->size >= minSize){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* KENNY: ADDED THIS ONE MYSELF!
 * Used when aalgorithm is BEST_FIT. Finds the first block that is
 * larger than or equal to minsize, by a minimized amount. 
 * Returns NULL If no block has size > minsize. */
struct nodeStruct* List_findBestFit (struct nodeStruct *head, int minSize){
    struct nodeStruct *current = head;
    struct nodeStruct *ret = NULL;
    /* Initialize minDiff to an unrealistically large value */
    int minDiff = INT_MAX;

    while (current != NULL){
        int curDiff = current->size - minSize;

        if (curDiff < minDiff && curDiff >= 0){
            minDiff = curDiff;
            ret = current;
        } 

        current = current->next;
    }

    return ret;
}

/* KENNY: ADDED THIS ONE MYSELF!
 * Used when aalgorithm is WORST_FIT. Finds a block that is
 * larger than or equal to minsize, by the maximum amount.
 * Returns NULL if no block has size > minSize.
 */
struct nodeStruct* List_findWorstFit (struct nodeStruct *head, int minSize){
    struct nodeStruct *current = head;
    struct nodeStruct *ret = NULL;
    int maxDiff = -1;

    while (current != NULL){
        int curDiff = current->size - minSize;
    
        if (curDiff >= 0 && curDiff > maxDiff){
            maxDiff = curDiff;
            ret = current;
        }

        current = current->next;
    }

    return ret;
}

/* KENNY: ADDED THIS ONE MYSELF!
 * Used when kfree() is called. Looks for two things:
    1). Free nodes w/ pointer = coalescepoint.ptr + coalescepoint.size 
        (i.e. a pointer right after our memory block)
    2). Free nodes such that freeNode.ptr + freeNode.size = coalescepoint.ptr
        (i.e. OUR pointer is right after another person's memory block)
 */
void List_coalesceNodes(struct nodeStruct **headRef, struct nodeStruct *coalescepoint){
    struct nodeStruct *current = *headRef;
    
    while (current != NULL){
        /* Checking the first case (above) */
        if ((void*)((char*)coalescepoint->ptr + coalescepoint->size) == current->ptr){
            void* ptr = coalescepoint->ptr;
            int size = coalescepoint->size + current->size;
            struct nodeStruct *coalescedNode = List_createNode(size, ptr);
            //printf("DEBUG: CoalesceNodes | for coalescedNode, ptr = %p, size = %d\n", ptr, size);
            //printf("DEBUG: CoalesceNodes | for coalescePoint, ptr = %p, size = %d\n", coalescepoint->ptr, coalescepoint->size);
            //printf("DEBUG: CoalesceNodes | for current, ptr = %p, size = %d\n", current->ptr, current->size);
            List_deleteNode(headRef, current);
            List_deleteNode(headRef, coalescepoint);
            List_insertTail(headRef, coalescedNode);
            coalescepoint = coalescedNode;

            /* Set the coalescepoint to be this new merged node, in case
             * there is also a case 2 node. */
            coalescepoint = coalescedNode;
            current = *headRef;
        }
        /* Checking the second case (above) */
        if ((void*)((char*)current->ptr + current->size) == coalescepoint->ptr){
            void* ptr = current->ptr;
            int size = coalescepoint->size + current->size;
            struct nodeStruct *coalescedNode = List_createNode(size, ptr);
            List_deleteNode(headRef, current);
            List_deleteNode(headRef, coalescepoint);
            List_insertTail(headRef, coalescedNode);

            /* Set the coalescepoint to be this new merged node, in case
             * there is also a case 1 node. */
            coalescepoint = coalescedNode;
            current = *headRef;
        }   

        current = current->next;
    }
}


/*
 * Sort the list in ascending order based on the size field.
 * Any sorting algorithm is fine.
 */
void List_sort (struct nodeStruct **headRef)
{
	while (doSinglePassOnSort(headRef)) {
		// Do nothing: work done in loop condition.
	}
}
static _Bool doSinglePassOnSort(struct nodeStruct **headRef)
{
	_Bool didSwap = false;
	while (*headRef != NULL) {
		struct nodeStruct *nodeA = *headRef;
		// If we don't have 2 remaining elements, nothing to swap.
		if (nodeA->next == NULL) {
			break;
		}
		struct nodeStruct *nodeB = nodeA->next;

		// Swap needed?
		if (nodeA->ptr > nodeB->ptr){
			swapElements(headRef, nodeA, nodeB);
			didSwap = true;
		}

		// Advance to next elements
		headRef = &((*headRef)->next);
	}
	return didSwap;
}
static void swapElements(struct nodeStruct **previous,
		struct nodeStruct *nodeA,
		struct nodeStruct *nodeB)
{
	*previous = nodeB;
	nodeA->next = nodeB->next;
	nodeB->next = nodeA;
}
