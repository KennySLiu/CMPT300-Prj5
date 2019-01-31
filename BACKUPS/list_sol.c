#include "list_sol.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

static _Bool doSinglePassOnSort(struct nodeStruct **headRef);
static void swapElements(struct nodeStruct **previous, struct nodeStruct *nodeA, struct nodeStruct *b);


/*
 * Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value id. Return a pointer to the new node.
 */
struct nodeStruct* List_createNode(int id)
{
	struct nodeStruct *pNode = malloc(sizeof(struct nodeStruct));
	if (pNode != NULL) {
		pNode->id = id;
        pNode->edge_to = NULL;
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
 * Return the first node holding the value id, return NULL if none found
 */
struct nodeStruct* List_findNode(struct nodeStruct *head, int id)
{
	struct nodeStruct *current = head;
	while (current != NULL) {
		if (current->id == id) {
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


/*
 * Sort the list in ascending order based on the id field.
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
		if (nodeA->id > nodeB->id){
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






/* KENNYS STUFF */
void List_setEdge(struct nodeStruct *node, struct nodeStruct *edge_to){
    node->edge_to = edge_to;
}

int List_findCycle(struct nodeStruct *node){
    struct nodeStruct *curr = node->edge_to;
    int debug_infiniteLoop = 0;

    while (curr->edge_to != NULL){
        ++debug_infiniteLoop;
        if (curr == node){  
            /*printf("DEBUG: List_findCycle | found a cycle.\n"); */
            return 1;   
        }
        curr = curr->edge_to;
        
        if (debug_infiniteLoop >= 5000 && debug_infiniteLoop%100000 == 5000){
            printf("There's probably an infinite loop in List_findCycle.");
        }
    }

    return 0;
}

void List_deleteAll(struct nodeStruct **headRef){
    if (headRef == NULL){
        printf("List_deleteAll called with a NULL headRef\n");
        return;
    } if (*headRef == NULL){
        printf("List_deleteAll called with a NULL *headRef\n");
        return;
    }

    struct nodeStruct *node = *headRef;

    while (node != NULL){
        *headRef = node->next;
        free(node);

        node = *headRef;
    }
    *headRef = NULL;
}

