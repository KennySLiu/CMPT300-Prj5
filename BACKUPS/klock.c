#include "klock.h"


/* ragMutex is a mutex for the resoruce graph variable, since it will be accessed
 * from a bunch of different threads */
static pthread_mutex_t ragMutex;
bool ragMutexIsInit = false;

/* To store the RAG, we'll use two adjacency lists. Note that since each node can have 
    a maximum out-degree of 1 (as discussed below), these adjacency lists will have 0 or 1 entries,
    so, quite sparse. Thus, we will condense them into a specialized linked list. */
static struct nodeStruct* resources = NULL;
static struct nodeStruct* threads = NULL;
/* This will will act as global trackers for which lock we're working with */
static int num_smartlocks = 0;

/* The structure of the resource graph can probably be taken advantage of.
 * We know that the RAG is bipartite, because edges only go from T->R or R->T, not the other way around.
 * We know that the RAG is directed.
 * We need to ensure that the RAG stays acyclic. So it'll be a bipartite DAG.
 * Each thread will only ever have one request edge, because creating a request edge is blocking.
 * Each resource will only have as many assignment edges as there are copies of that resource.
 * Since we're using MUTEXES, that means each resource is assumed to have ONE instance. */
/* Maybe we can use adjacency lists? One adjacency list for each thread,
    or one for each resource.
 * Maybe we can use redundant adjacency lists, keeping one for each thread AND one for each resource.
 * I don't think a matrix would work too well, because it wouldn't save us space and would have a similar
    runtime as redundant adjacency lists, because the graph is expected to be quite sparse. */





void init_lock(SmartLock* lock) {

    pthread_mutex_init(&(lock->mutex), NULL); 

    pthread_mutex_lock(&ragMutex);
    {
        /* Critical section */
        /* Keep the data for the thread that called us. */
        int threadID = (int) pthread_self();
        ++num_smartlocks;

        lock->id = num_smartlocks;
        
        ////printf("DEBUG: init | init lock with id = %d\n", num_smartlocks);

        struct nodeStruct *resourceNode = List_createNode(num_smartlocks);
        if (ragMutexIsInit == false){
            pthread_mutex_init(&ragMutex, NULL);
        }

        List_insertTail(&resources, resourceNode);

        if (List_findNode(threads, threadID) == NULL){
            /* If the node DOESN'T exist, create it. (the resource node is guaranteed
             * not to exist, because we just incremented num_smartlocks) */
            struct nodeStruct *threadNode = List_createNode(threadID);
            List_insertTail(&threads, threadNode);
        } 
    }
    pthread_mutex_unlock(&ragMutex);
    
}

int lock(SmartLock* lock) {
	/*printf("%lu locking\n", pthread_self());
	pthread_mutex_lock(&(lock->mutex));
	return 1;*/

    /* KENNYS CODE BELOW */    
    int retval;
    int threadID = (int) pthread_self();
    struct nodeStruct* threadNode = NULL;
    struct nodeStruct* resourceNode = NULL;
    //printf("DEBUG: lock | Lock was called, with thread = %d and id = %d\n", threadID, lock->id);

    pthread_mutex_lock( &ragMutex );
    {
        /* Critical section */
        threadNode = List_findNode( threads, threadID );
        if (threadNode == NULL){
            /* If the node DOESN'T exist, create it. */
            threadNode = List_createNode( threadID );
            List_insertTail( &threads, threadNode );
        } 
    }
    pthread_mutex_unlock( &ragMutex );

    int resourceID = lock->id;
    
    /* Now what we need to do is check: if our threadNode DOES have an edge, is there a cycle? */
    pthread_mutex_lock( &ragMutex );
    {
        /* Critical section */
        /* The resourceNode will necessarily exist, because we assume lock_init was called. */
        resourceNode = List_findNode(resources, resourceID);

        List_setEdge(threadNode, resourceNode);
        bool cycleExists = (bool) List_findCycle(threadNode);

        if (cycleExists){
            //printf("DEBUG: Lock | cycle would've existed! lock_id = %d, thread_id = %d\n", lock->id, threadID);
            List_setEdge(threadNode, NULL);
            retval = 0;
        } else {
            /* pthread_mutex_lock blocks, so while it's blocking, our T->R edge is valid. 
             * Once it returns, it means the resource is now assigned to the thread, so we need to 
             * update the graph accordingly. */
            pthread_mutex_unlock( &ragMutex );
            {
            /* We have to unlock ragMutex, because we need to let the other processes modify the graph
                when they're done with their resources, so we can actually lock ours. */
                //printf("DEBUG: Lock | No cycles! lock_id = %d, thread_id = %d\n", lock->id, threadID);
                pthread_mutex_lock(&(lock->mutex));
            }
            pthread_mutex_lock( &ragMutex );
            List_setEdge(threadNode, NULL);
            List_setEdge(resourceNode, threadNode);
            retval = 1;
        }
    }
    pthread_mutex_unlock( &ragMutex );

    //printf("DEBUG: lock | LOCK EXITING. Retval = %d, thread = %d, and id = %d\n", retval, threadID, lock->id);
    return retval;
}

void unlock(SmartLock* lock) {
	/* pthread_mutex_unlock(&(lock->mutex)); */

    /* KENNYS CODE BELOW */
    int threadID = (int) pthread_self();
    int resourceID = lock->id;
    /* The resourceNode will necessarily exist, because we assume lock_init was called. */
    
    pthread_mutex_lock( &ragMutex );
        /* Critical section, because of the if clauses */ 
        struct nodeStruct* resourceNode = List_findNode( resources, resourceID );
        struct nodeStruct* threadNode = List_findNode( threads, threadID );

        if ( threadNode == NULL ){
            /* If the thread hasn't even called anything yet (this makes no sense, but we'll cover it anyway)
               then don't do anything and just return. */
            //printf("DEBUG: unlock | threadNode was null! lock_id = %d\n", lock->id);

            pthread_mutex_unlock( &ragMutex );
            return;
        } else if ( resourceNode->edge_to != threadNode ){
            /* If the thread doesn't even have a claim on the lock, then don't do anything */
            //printf("DEBUG: unlock | The resource edge was not to threadNode! lock_id = %d\n", lock->id);

            pthread_mutex_unlock( &ragMutex );
            return;
        } else {
            //printf("DEBUG: unlock | Unlocking successful. lock_id = %d\n", lock->id);
            List_setEdge(resourceNode, NULL);
            pthread_mutex_unlock(&(lock->mutex));

            pthread_mutex_unlock( &ragMutex );
            return;
        }
}

/*
 * Cleanup any dynamic allocated memory for SmartLock to avoid memory leak
 * You can assume that cleanup will always be the last function call
 * in main function of the test cases.
 */
void cleanup() {
    /* KENNYS CODE BELOW */

    /* Maybe some other logic goes here? */

    pthread_mutex_lock( &ragMutex );
    {
        if (resources != NULL){  List_deleteAll(&resources); }
        if (threads != NULL){    List_deleteAll(&threads);   }
    }
    pthread_mutex_unlock( &ragMutex );
}

