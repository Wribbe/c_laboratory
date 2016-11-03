#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h> // for sleep() (Linux).


// Struct for storing data to index, rotations, and sleep_duration.
typedef struct data_struct {
    int index;
    int rotations;
    unsigned int sleep_duration;
} data_struct;


// Do some work.
void * do_work(void * arg) {

    // Cast void pointer argument to data_struct pointer.
    data_struct * data = (data_struct * )arg;

    // Set up messages.
    const char * fmt_init_message = "This is thread #%d, and it will sleep for %d seconds.\n";
    const char * fmt_slept_message = "This is thread #%d, and it is done sleeping.\n";

    // Restore values from pointer struct.
    int index = data->index;
    unsigned int sleep_duration = data->sleep_duration;
    int max_rotations = data->rotations;

    // Keep track of rotations in thread context.
    int current_rotation = 0;

    // Work -> Sleep -> Work until done.
    while(current_rotation < max_rotations) {
        printf(fmt_init_message, index, sleep_duration);
        sleep(sleep_duration);
        printf(fmt_slept_message, index);
        current_rotation++;
    }
    return NULL;
}

int main(void) {

    // If this is run with the time command it should take (10-1)*10s = 90s.
    // Which it does: " .. cpu 1:30.01 total"

    // Constants.
    int NUM_THREADS = 10;
    int WORKITERATIONS = 10;

    // Thread related data.
    pthread_t thread_pool[NUM_THREADS];
    data_struct thread_input[NUM_THREADS];

    // Thread creation.
    for (int i=0; i<NUM_THREADS; i++) {
        // Creating new thread input struct on the stack.
        thread_input[i] = (data_struct){.index=i,
                                        .sleep_duration=i,
                                        .rotations=WORKITERATIONS};
        pthread_create(&thread_pool[i], NULL, do_work, &thread_input[i]);
    }

    // Wait until the threads are done.
    for (int i=0; i<NUM_THREADS; i++) {
        pthread_join(thread_pool[i], NULL);
    }

    return 0;
}
