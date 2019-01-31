#include <stdio.h>
#include "kallocator.h"

int main(int argc, char* argv[]) {
    initialize_allocator(100, FIRST_FIT);
    //initialize_allocator(100, BEST_FIT);
    //initialize_allocator(100, WORST_FIT);
    printf("Using first fit algorithm on memory size 100\n");


    /* Setting up the p array using kalloc */
    printf("\n\nSetting up the p array using kalloc\n");
    int* p[100] = {NULL};
    char* cp[100] = {NULL};
    for(int i=0; i<30; ++i) {
        p[i] = kalloc(sizeof(int));
        if(p[i] == NULL) {
            printf("Allocation failed\n");
            continue;
        }

        *(p[i]) = i;
        printf("p[%d] = %p ; *p[%d] = %d\n", i, p[i], i, *(p[i]));
    }


    /* First call to print statistics: */
    printf("\n\nPrinting statistics after allocating p:\n");
    print_statistics();
    //debug_print(2);


    /* Free certain entries: */
    printf("\nFreeing entries:\n");
    for(int i=0; i<25; ++i) {
        if(i%2 == 0)
            continue;

        printf("Freeing p[%d]\n", i);
        kfree(p[i]);

        //debug_print(2);
        p[i] = NULL;
    }
    printf("Freeing p[%d]\n", 2);
    kfree(p[2]);
    printf("Freeing p[%d]\n", 14);
    kfree(p[14]);
    printf("Freeing p[%d]\n", 16);
    kfree(p[16]);

    /* Second call to print stats: */
    printf("\n\nPrinting stats after freeing some of p:\n");
    printf("available_memory %d\n", available_memory());
    print_statistics();
    for (int i = 0; i < 25; ++i){
        if (i%2 == 1 || i == 2 || i == 14 || i == 16)
            continue;
        printf("p[%d] = %p ; *p[%d] = %d\n", i, p[i], i, *(p[i]));
    }
    debug_print(0);



    /* Test with characters */
    printf("\n\nTesting with characters:\n");
    for (int i = 0; i < 55; ++i){
        cp[i] = kalloc(sizeof(char));
        if (cp[i] == NULL){
            printf("Allocation failed\n");
            continue;
        } 

        if (i == 2 || i == 5){
            debug_print(0);
        }

        *(cp[i]) = i+65;
        printf("cp[%d] = %p ; *cp[%d] = %c\n", i, cp[i], i, *(cp[i]));
    }
    debug_print(0);



    /* Compact the array: */
    printf("\nCompacting now:\n");

    void* before[100] = {NULL};
    void* after[100] = {NULL};
    int beforesize = compact_allocation(before, after);

    debug_print(0);
    print_statistics();

    debug_print(0);

    /* KENNYS EXTRA TESTING CODE: */
    for (int i = 0; i < beforesize; ++i){
        printf("before[%d] = %p\n", i, before[i]);
        printf("after[%d] = %p\n", i, after[i]);
    }

    printf("\n\n");

    for (int i = 0; i < beforesize; ++i){
        for (int j = 0; j < 25; ++j){
            if (j%2 == 0 && j != 2 && j != 14 && j != 16){
                if (p[j] == before[i]){
                    p[j] = after[i];
                }
            }
        }
    }
    for (int i = 0; i < 25; ++i){
        if (i%2 == 0 && i != 2 && i != 14 && i != 16)
            printf("p[%d] = %p ; *p[%d] = %d\n", i, p[i], i, *(p[i]));
    }

    for (int i = 0; i < beforesize; ++i){
        for (int j = 0; j < 52; ++j){
            if (cp[j] == before[i]){
                printf("CP Hit found: cp[%d] = %p, before[%d] = %p\n", j, cp[j], i, before[i]);
                cp[j] = after[i];
            }
        }
    }
    for (int i = 0; i < 52; ++i){
        printf("cp[%d] = %p ; *cp[%d] = %c\n", i, cp[i], i, *(cp[i]));
    }

    // You can assume that the destroy_allocator will always be the 
    // last funciton call of main function to avoid memory leak 
    // before exit

    printf("\n\nDestroying allocator:\n");
    destroy_allocator();

    debug_print(0);

    return 0;
}
