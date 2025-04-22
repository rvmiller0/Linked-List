#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stdatomic.h>
#include "lock-free.c"

#define NUM_THREADS 8
#define OPERATIONS_PER_THREAD 10000
#define VALUE_RANGE 1000

// Global atomic head pointer
_Atomic(Node*) head = NULL;

// Keep track of expected values using atomic operations
#define BUCKET_SIZE (VALUE_RANGE + 1)
typedef struct {
    int value;
    _Atomic int count;  // Track how many times this value should be in the list
    _Atomic bool used;  // Flag to indicate if this bucket is used
} ValueCounter;

ValueCounter* expected_values;

/**
 * Thread struct
 */
typedef struct {
    int thread_id;
    int seed;
} ThreadArg;

/**
 * Initialize the expected values array
 */
void init_expected_values() {
    expected_values = calloc(BUCKET_SIZE, sizeof(ValueCounter));
    if (expected_values == NULL) {
        perror("calloc failed for expected_values");
        exit(EXIT_FAILURE);
    }

    // Initialize atomic variables
    for (int i = 0; i < BUCKET_SIZE; i++) {
        atomic_init(&expected_values[i].count, 0);
        atomic_init(&expected_values[i].used, false);
    }
}

/**
 * Lock free add value to the expected value
 */
void add_expected(int value) {
    expected_values[value].value = value;
    atomic_fetch_add(&expected_values[value].count, 1);
    atomic_store(&expected_values[value].used, true);
}

/**
 * Lock free remove value from expected values - 
 */
void remove_expected(int value) {
    if (atomic_load(&expected_values[value].used) && 
        atomic_load(&expected_values[value].count) > 0) {
        
        int old_count = atomic_fetch_sub(&expected_values[value].count, 1);
        
        if (old_count == 1) {
            atomic_store(&expected_values[value].used, false);
        }
    }
}

/**
 * Lock free get the total count of expected values
 */
int get_expected_count() {
    int total = 0;
    for (int i = 0; i < BUCKET_SIZE; i++) {
        if (atomic_load(&expected_values[i].used)) {
            total += atomic_load(&expected_values[i].count);
        }
    }
    return total;
}

/**
 * Thread implementation
 */
void* thread_function(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    int thread_id = thread_arg->thread_id;
    unsigned int seed = thread_arg->seed;
    
    printf("Thread %d starting\n", thread_id);
    
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        int operation = rand_r(&seed) % 3;
        int value = rand_r(&seed) % VALUE_RANGE;
        
        switch (operation) {
            case 0: // Insert
                insert_begin(&head, value);
                add_expected(value);
                break;
                
            case 1: // Delete
                if (delete_node(&head, value)) {
                    remove_expected(value);
                }
                break;
                
            case 2: // Search
                search(&head, value);
                break;
        }
        
        // Delay to cause thread interleaving
        if (rand_r(&seed) % 100 == 0) {
            usleep(1);
        }
    }
    
    printf("thread %d complete\n", thread_id);
    return NULL;
}

/**
 * Verification function to check list integrity
 */
bool verify_list() {
    printf("verifying integrity...\n");
    
    int actual_count = count_nodes(&head); // Count actual nodes in list
    int expected_count = get_expected_count(); // Get expected count from tracker

    // Compare counts first
    printf("node counts: actual=%d, expected=%d\n", actual_count, expected_count);
    if (actual_count != expected_count) {
        printf("verification fail: count mismatch\n");
        return false;
    }
        
    // Count occurrences of each value in list
    int* list_counts = calloc(BUCKET_SIZE, sizeof(int));
    if (list_counts == NULL) {
        perror("calloc failed for list_counts");
        return false;
    }
    
    Node* current = atomic_load(&head);
    while (current != NULL) {
        if (!atomic_load(&current->marked)) {
            if (current->data >= 0 && current->data < BUCKET_SIZE) {
                list_counts[current->data]++;
            } else {
                printf("verification fail: value %d out of expected range\n", current->data);
                free(list_counts);
                return false;
            }
        }
        current = atomic_load(&current->next);
    }
    
    // Compare with expected
    bool result = true;
    for (int i = 0; i < BUCKET_SIZE; i++) {
        int expected = atomic_load(&expected_values[i].used) ? 
                     atomic_load(&expected_values[i].count) : 0;
        if (list_counts[i] != expected) {
            printf("verification fail: value %d - found %d times, expected %d times\n", 
                   i, list_counts[i], expected);
            result = false;
        }
    }
    
    free(list_counts);
    return result;
}

int main() {
    srand(time(NULL)); // Initialize random seed
    init_expected_values(); // Initialize expected values array
    atomic_init(&head, NULL); // Initialize atomic head
    
    // Create threads
    pthread_t threads[NUM_THREADS];
    ThreadArg thread_args[NUM_THREADS];
    
    printf("starting %d threads with %d operations each\n", NUM_THREADS, OPERATIONS_PER_THREAD);
        
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].thread_id = i;
        thread_args[i].seed = rand();
        
        if (pthread_create(&threads[i], NULL, thread_function, &thread_args[i]) != 0) {
            perror("thread creation fail");
            exit(EXIT_FAILURE);
        }
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("thread join fail");
            exit(EXIT_FAILURE);
        }
    }
    
    printf("all threads complet\n");
    
    // Print final list
    printf("Final ");
    print_list(&head);
    
    // Verify integrity
    if (verify_list()) {
        printf("verification pass\n");
    } else {
        printf("verification fail\n");
    }
    
    // Clean up
    free_list(&head);
    free(expected_values);
    
    return 0;
}