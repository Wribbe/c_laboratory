#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

void * say_hello(void * argument) {
    int thread_id = *((int*)argument);
    printf("Hello world from thread: %d\n", thread_id);
}

int main(int argc, char * argv) {

    int NUM_THREADS = 10;
    int thread_args[NUM_THREADS];

    pthread_t thread_pool[NUM_THREADS];

    // Create threads that say hello.
    for (int i=0; i<NUM_THREADS; i++) {
        thread_args[i] = i;
        pthread_create(&thread_pool[i], NULL, say_hello, &thread_args[i]);
    }

    // Join the threads.
    for (int i=0; i<NUM_THREADS; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    return 0;
}
