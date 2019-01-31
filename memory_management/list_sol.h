// Linked list module.

#ifndef LIST_H_
#define LIST_H_

struct nodeStruct {
    int size;
    void* ptr;
    struct nodeStruct *next;
};

/*
 * Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value size. Return a pointer to the new node.
 */
struct nodeStruct* List_createNode(int size, void *ptr);

/*
 * Insert node at the head of the list.
 */
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node);

/*
 * Insert node after the tail of the list.
 */
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node);

/*
 * Count number of nodes in the list.
 * Return 0 if the list is empty, i.e., head == NULL
 */
int List_countNodes (struct nodeStruct *head);

/*
 * Return the first node holding the pointer ptr, return NULL if none found
 */
struct nodeStruct* List_findNode(struct nodeStruct *head, void *ptr);

/*
 * Delete node from the list and free memory allocated to it.
 * This function assumes that node has been properly set (by for example
 * calling List_findNode()) to a valid node in the list. If the list contains
 * only node, the head of the list should be set to NULL.
 */
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node);



/*
 * Sort the list in ascending order based on the size field.
 * Any sorting algorithm is fine.
 */
void List_sort (struct nodeStruct **headRef);




/* KENNYS STUFF FOR MEMORY MANAGEMENT PRJ5: */
/* KENNY: ADDED THIS ONE MYSELF!
 * Delete all nodes in the linked list pointed to by *headRef.
 */
void List_deleteAll (struct nodeStruct **headRef);

/* KENNY: ADDED THIS ONE MYSELF!
 * Used when aalgorithm is FIRST_FIT. Finds the first block that
 * is larger than or equal to size. Returns NULL if none found. */
struct nodeStruct* List_findFirstFit (struct nodeStruct *head, int minSize);

/* KENNY: ADDED THIS ONE MYSELF!
 * Used when aalgorithm is BEST_FIT. Finds a block that is
 * larger than or equal to minsize, by a minimized amount. 
 * Returns NULL if no block has size > minsize. */
struct nodeStruct* List_findBestFit (struct nodeStruct *head, int minSize);

/* KENNY: ADDED THIS ONE MYSELF!
 * Used when aalgorithm is WORST_FIT. Finds a block that is
 * larger than or equal to minsize, by the maximum amount.
 * Returns NULL if no block has size > minSize.
*/
struct nodeStruct* List_findWorstFit (struct nodeStruct *head, int minSize);

/* KENNY: ADDED THIS ONE MYSELF!
 * Used when kfree() is called. Looks for two things:
 *  1). Free nodes w/ pointer = newNode.ptr + newNode.size 
 *      (i.e. a pointer right after our memory block)
 *  2). Free nodes such that freeNode.ptr + freeNode.size = newNode.ptr
 *      (i.e. OUR pointer is right after another person's memory block)
 */
void List_coalesceNodes(struct nodeStruct **headRef, struct nodeStruct *coalescepoint);

/* KENNY: ADDED THIS ONE MYSELF!
 * This function is just to remove clutter, because
 * I continually used this same chunk of code over and over.
 */
void* allocate_node(struct nodeStruct **freeBlocks, struct nodeStruct **allocatedBlocks, struct nodeStruct *freeNode, int _size);



#endif
