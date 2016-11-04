#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>


// Struct for function pointers.
typedef struct function_pointers {
    void (*add_func)();
} function_pointers;


// Struct for worker thread data.
typedef struct worker_data {
    pthread_mutex_t * mutex;
    pthread_cond_t * work_sig;
    pthread_cond_t * mon_sig;
    int * shared_data;
    int * work_available;
    int * stop_work;
    function_pointers * pointers;
    int index;
} worker_data;


// Struct for monitor thread data.
typedef struct monitor_data {
    pthread_cond_t * work_sig;
    pthread_cond_t * mon_sig;
    pthread_mutex_t * mutex;
    int * shared_minimum;
    int * shared_data;
    int * work_available;
} monitor_data;


// Shared addition method.
void shared_addition(worker_data * data) {
    (*(data->shared_data))++;
    printf("[DATA]: The current shared variable is: %d\n", *(data->shared_data));
}


void * do_additions(void * arg) {

    // Cast arg -> worker_data.
    worker_data * data = (worker_data *)arg;

    // Get the mutex.
    pthread_mutex_lock(data->mutex);
    printf("[WORKER #%d]: has mutex.\n", data->index);

    for(;;) { // Infinite work-loop.


        while(!*(data->work_available)) {
            // There is no work, sleep.
            printf("[WORKER #%d]: No work, sleeping.\n", data->index);

            pthread_cond_wait(data->work_sig, data->mutex);
        }

        // Return gracefully if work is done.
        if (*(data->stop_work)) {
            break;
        }

        printf("[WORKER #%d]: Starting work!\n", data->index);

        // Got the lock, add to the shared resource.
        (*data->pointers->add_func)(data);
        // Set work flag.
        *(data->work_available) = 0;
        // Signal monitor thread.
        printf("[WORKER #%d]: Done, signaling monitor.\n", data->index);
        pthread_cond_signal(data->mon_sig);
    }

    // Release lock for next worker.
    pthread_mutex_unlock(data->mutex);
    return NULL;
}

int check_done(monitor_data * data) {
    int shared = *(data->shared_data);
    int minimum = *(data->shared_minimum);
    if (shared < minimum) {
        printf("[MONITOR]: Work to be done %d < %d.\n", shared, minimum);
        return 0;
    } else {
        printf("[MONITOR]: DONE!\n");
        return 1;
    }
}

void * do_monitor(void * arg) {

    // Cast arg -> monitor data.
    monitor_data * data = (monitor_data * )arg;

    // Make sure everything is initialized.
    sleep(4);

    printf("[MONITOR]: Waiting for mutex.\n");
    pthread_mutex_lock(data->mutex);
    printf("[MONITOR]: got mutex.\n");

    while(!*(data->work_available)) { // No one is working.
        if (check_done(data)) {
            break;
        }
        printf("[MONITOR]: No one is working, kick a working thread into gear.\n");
        *(data->work_available) = 1;
        // Kick one of the worker threads into gear.
        pthread_cond_signal(data->work_sig);
        // Wait for the worker thread to finish and signal.
        pthread_cond_wait(data->mon_sig, data->mutex);
    }

    pthread_mutex_unlock(data->mutex);

    return NULL;
}


// Main function.
int main(void) {

    // Constants.
    int shared_minimum = 500;
    int shared_data = 0;
    int num_threads = 10;
    int work_available = 0;
    int stop_work = 0;

    // Create mutex.
    pthread_mutex_t add_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Create signals.
    pthread_cond_t new_data_sig = PTHREAD_COND_INITIALIZER;
    pthread_cond_t do_work_sig = PTHREAD_COND_INITIALIZER;

    // Worker thread-pool related data.
    pthread_t thread_pool[num_threads];
    worker_data input_data[num_threads];

    // Monitor thread related data.
    pthread_t monitor_thread;

    // Set up the pointer struct.
    function_pointers pointers = {.add_func = shared_addition};

    // Spin up some worker threads.
    for (int i=0; i<num_threads; i++) {
        printf("[MAIN]: Creating worker thread #%d.\n", i+1);
        input_data[i] = (worker_data){
            .mutex = &add_mutex,
            .work_sig = &do_work_sig,
            .mon_sig = &new_data_sig,
            .shared_data = &shared_data,
            .pointers = &pointers,
            .work_available = &work_available,
            .index = i+1,
            .stop_work = &stop_work,
        };
        pthread_create(&thread_pool[i], NULL, do_additions, &input_data[i]);
    }

    // Create and start the monitor thread.
    monitor_data mon_data = {
       .work_sig = &do_work_sig,
       .mon_sig = &new_data_sig,
       .mutex = &add_mutex,
       .shared_minimum = &shared_minimum,
       .shared_data = &shared_data,
       .work_available = &work_available,
    };

    pthread_create(&monitor_thread, NULL, do_monitor, &mon_data);
    printf("[MAIN]: Created monitor thread.\n");

    // Wait for the monitor tread to return.
    printf("[MAIN]: Waiting to join monitor thread.\n");
    pthread_join(monitor_thread, NULL);
    printf("[MAIN]: Joined with monitor thread.\n");

    printf("[MAIN]: Setting stop_work to 1.\n");
    stop_work = 1;
    work_available = 1;

    printf("[MAIN]: Broadcasting to all sleeping workers.\n");
    // Wake up sleeping worker threads.
    pthread_mutex_lock(&add_mutex);
    pthread_cond_broadcast(&do_work_sig);
    pthread_mutex_unlock(&add_mutex);

    // Wait for all worker threads to join.
    for(int i=0; i<num_threads; i++) {
        printf("[MAIN]: Joining working thread %d.\n", i);
        pthread_join(thread_pool[i], NULL);
        printf("[MAIN]: Done joining working thread %d.\n", i);
    }


    // Destroy mutex.
    pthread_mutex_destroy(&add_mutex);

    // Destroy signals;
    pthread_cond_destroy(&new_data_sig);
    pthread_cond_destroy(&do_work_sig);
    return 0;
}
