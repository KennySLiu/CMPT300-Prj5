#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "kallocator.h"
#include "list_sol.h"

struct KAllocator {
    enum allocation_algorithm aalgorithm;
    int size;
    void* memory;
    // Some other data members you want, 
    // such as lists to record allocated/free memory

    struct nodeStruct *freeBlocks;
    struct nodeStruct *allocatedBlocks;

};

struct KAllocator kallocator;

void initialize_allocator(int _size, enum allocation_algorithm _aalgorithm) {
    assert(_size > 0);
    kallocator.aalgorithm = _aalgorithm;
    kallocator.size = _size;
    kallocator.memory = malloc((size_t)kallocator.size);

    // Add some other initialization 

    kallocator.freeBlocks = List_createNode(_size, kallocator.memory);
    kallocator.allocatedBlocks = NULL;

}

void destroy_allocator() {
    free(kallocator.memory);

    // free other dynamic allocated memory to avoid memory leak
    /* deleteAll sets the HEAD pointers to NULL for me, so I don't need to do that. 
     * I need to make sure they aren't NULL, though, because otherwise the deleteAll function fails. */
    if (kallocator.freeBlocks != NULL){
        List_deleteAll(&kallocator.freeBlocks);
    }
    if (kallocator.allocatedBlocks != NULL){
        List_deleteAll(&kallocator.allocatedBlocks);
    }
}

void* kalloc(int _size) {
    void* ptr = NULL;
    //printf("DEBUG: KALLOC WAS CALLED!\n");

    // Allocate memory from kallocator.memory 
    // ptr = address of allocated memory

    if (kallocator.aalgorithm == FIRST_FIT){
        /* If we use FIRST_FIT, we traverse the freeBlocks to 
         * find the first block that is large enough. */
        struct nodeStruct *freeNode = List_findFirstFit(kallocator.freeBlocks, _size );

        if (freeNode != NULL){
            //printf("DEBUG: FIRST Fit returned okay\n");
            ptr = allocate_node(&kallocator.freeBlocks, &kallocator.allocatedBlocks, freeNode, _size);
        } else { 
            //printf("DEBUG: FIRST Fit returned NULL\n");
            ptr = NULL;
        }
    }
    else if (kallocator.aalgorithm == BEST_FIT){
        /* If we use BEST_FIT, we need to traverse freeBlocks so that
         * we can find the node with size >= _size, but minimally greater. */
        struct nodeStruct *freeNode = List_findBestFit(kallocator.freeBlocks, _size );

        if (freeNode != NULL){
            //printf("DEBUG: BEST Fit returned okay\n");
            ptr = allocate_node(&kallocator.freeBlocks, &kallocator.allocatedBlocks, freeNode, _size);
        } else {
            //printf("DEBUG: BEST Fit returned NULL\n");
            ptr = NULL;
        }
    }
    else if (kallocator.aalgorithm == WORST_FIT){
        /* If we use WORST_FIT, we need to traverse freeBlocks so that 
         * we can find the node with size >= _size, but maximally greater. */
        struct nodeStruct *freeNode = List_findWorstFit(kallocator.freeBlocks, _size );

        if (freeNode != NULL){ 
            //printf("DEBUG: WORST Fit returned okay\n");
            ptr = allocate_node(&kallocator.freeBlocks, &kallocator.allocatedBlocks, freeNode, _size);
        } else {
            //printf("DEBUG: WORST Fit returned NULL\n");
            ptr = NULL;
        }
    }

    List_sort(&kallocator.freeBlocks);

    return ptr;
}

void kfree(void* _ptr) {
    assert(_ptr != NULL);

    /* My code: */
    
    /* Get the node with poiter _ptr */
    struct nodeStruct* nodeToKill = List_findNode(kallocator.allocatedBlocks, _ptr);

    /* Add this metadata to the freeBlocks list */
    int size = nodeToKill->size;
    struct nodeStruct* freeNode = List_createNode(size, _ptr);
    List_insertTail(&kallocator.freeBlocks, freeNode);

    /* Remove the nodeToKill from the allocatedBlocks list: */
    List_deleteNode(&kallocator.allocatedBlocks, nodeToKill);

    /* Coalesce the free block, if possible */
    List_coalesceNodes(&kallocator.freeBlocks, freeNode);

    List_sort(&kallocator.freeBlocks);
}

int compact_allocation(void** _before, void** _after) {
    int compacted_size = 0;

    // compact allocated memory
    // update _before, _after and compacted_size
    
    /* KENNY: What is the necessary logic, here?
     * We can simply shove everything in the memory array over to the left, and 
     * not care about the freeBlocks, because the end result of freeBlocks is 
     * that it will contain one node with large size.
     */
    
    /* Initialization: */
    List_sort(&kallocator.allocatedBlocks);
    struct nodeStruct* current = kallocator.allocatedBlocks;
    void *endOfMemory = kallocator.memory;
    void *curptr = NULL;
    int cursize = 0;
    int totalsize = 0;
    int i = 0;
    
    /* Above, we have sorted the allocatedBlocks by increasing pointer values. This is so that
     * when we write data, we write from the RIGHT side of the array to the LEFT side, so we 
     * never overwrite data. */

    /*printf("\n\nDEBUG: compact_allocation | Calling debug print before the while loop!\n");
    debug_print(0);*/

    while (current != NULL){
        ++compacted_size;

        cursize = current->size;
        curptr = current->ptr;
        totalsize += cursize;

        /* Copy the addresses into the before & after arrays: */
        _before[i] = curptr;
        _after[i] = memcpy(endOfMemory, curptr, (size_t) cursize);

        /* Update the metadata, too: */
        current->ptr = _after[i];

        /* Increment endOfMemory so that we don't overwrite our data */
        endOfMemory = (void*)((char*)endOfMemory + cursize);

        current = current->next;
        ++i;
    }


    /* Now we need to re-do the freeBlocks array. Delete it all, and make a new big node.*/
    if (kallocator.freeBlocks != NULL){
        /* If the free blocks is NULL, this line would error out. */
        List_deleteAll(&kallocator.freeBlocks);
    }
    if (kallocator.size - totalsize > 0){
        /* If the size would be 0, then we don't really need a free node to represent that. */
        struct nodeStruct* freeNode = List_createNode(kallocator.size - totalsize, endOfMemory);
        List_insertTail(&kallocator.freeBlocks, freeNode);
    }

    return compacted_size;
}

int available_memory() {
    int available_memory_size = 0;
    // Calculate available memory size

    struct nodeStruct *current = kallocator.freeBlocks;
    while (current != NULL){
        available_memory_size += current->size;
        current = current->next;
    }
    return available_memory_size;
}

void print_statistics() {
    int allocated_size = 0;
    int allocated_chunks = 0;
    int free_size = 0;
    int free_chunks = 0;
    int smallest_free_chunk_size = kallocator.size;
    int largest_free_chunk_size = 0;

    // Calculate the statistics
    struct nodeStruct* current = kallocator.allocatedBlocks;
    while (current != NULL){
        allocated_size += current->size;
        ++allocated_chunks;
        current = current->next;
    }

    current = kallocator.freeBlocks;
    while (current != NULL){
        int curSize = current->size;

        free_size += curSize;
        ++free_chunks;

        largest_free_chunk_size = (curSize > largest_free_chunk_size) ? curSize : largest_free_chunk_size;
        smallest_free_chunk_size = (curSize < smallest_free_chunk_size) ? curSize : smallest_free_chunk_size;

        current = current->next;
    }

    printf("Allocated size = %d\n", allocated_size);
    printf("Allocated chunks = %d\n", allocated_chunks);
    printf("Free size = %d\n", free_size);
    printf("Free chunks = %d\n", free_chunks);
    printf("Largest free chunk size = %d\n", largest_free_chunk_size);
    printf("Smallest free chunk size = %d\n", smallest_free_chunk_size);

    //printf("DEBUG: print_statistics | \n");
}



/* KENNYS STUFF: */
int get_free_size(){
    struct nodeStruct *current = kallocator.freeBlocks;
    int size = 0;

    while (current != NULL){
        size += current->size;
    }
    return size;
}


void debug_print(int selector){
    struct nodeStruct *freeBlocks = kallocator.freeBlocks;
    struct nodeStruct *allocBlocks = kallocator.allocatedBlocks;

    struct nodeStruct *current = freeBlocks;
    int i = 0;

    if (selector == 0 || selector == 2){
        printf("\n\nDEBUG: debug_print | printing freeBlocks\n");
        while (current != NULL){
            printf("Node %d: size = %d, ptr = %p\n", i, current->size, current->ptr);

            current = current->next;
        }
    }
    
    if (selector == 0 || selector == 1){
        i = 0;
        current = allocBlocks;
        printf("\n\nDEBUG: debug_print | printing allocBlocks\n");
        while (current != NULL){
            printf("Node %d: size = %d, ptr = %p\n", i, current->size, current->ptr);

            current = current->next;
        }
    }
    
    printf("\n\n");
}
