#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include "linked-list.c"

#define NUM_THREADS 4
#define OPERATIONS_PER_THREAD 1000
#define VALUE_RANGE 1000

Node* head = NULL; // Global head pointer

// Better tracking of expected values - use a simple hash table
#define BUCKET_SIZE (VALUE_RANGE + 1)
typedef struct {
    int value;
    int count;  // Track how many times this value should be in the list
    bool used;  // Flag to indicate if this bucket is used
} ValueCounter;

ValueCounter* expected_values;
pthread_mutex_t expected_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Thread struct
 */
typedef struct {
    int thread_id;
    int seed;
} ThreadArg;

/**
 * Initialize the expected values array (hash table)
 */
void init_expected_values() {
    expected_values = calloc(BUCKET_SIZE, sizeof(ValueCounter));
    if (expected_values == NULL) {
        perror("calloc failed for expected_values");
        exit(EXIT_FAILURE);
    }
}

/**
 * Add value to the expected values
 */
void add_expected(int value) {
    pthread_mutex_lock(&expected_mutex);
    expected_values[value].value = value;
    expected_values[value].count++;
    expected_values[value].used = true;
    pthread_mutex_unlock(&expected_mutex);
}

/**
 * Remove value from expected values
 */
void remove_expected(int value) {
    pthread_mutex_lock(&expected_mutex);
    if (expected_values[value].used && expected_values[value].count > 0) {
        expected_values[value].count--;
        if (expected_values[value].count == 0) {
            expected_values[value].used = false;
        }
    }
    pthread_mutex_unlock(&expected_mutex);
}

/**
 * Get the total count of expected values
 */
int get_expected_count() {
    pthread_mutex_lock(&expected_mutex);
    int total = 0;
    for (int i = 0; i < BUCKET_SIZE; i++) {
        if (expected_values[i].used) {
            total += expected_values[i].count;
        }
    }
    pthread_mutex_unlock(&expected_mutex);
    return total;
}

/**
 * Thread implementation
 */
void* thread_function(void* arg) {
    ThreadArg* thread_arg = (ThreadArg*)arg;
    int thread_id = thread_arg->thread_id;
    unsigned int seed = thread_arg->seed;
    
    printf("thread %d start\n", thread_id);
    
    for (int i = 0; i < OPERATIONS_PER_THREAD; i++) {
        int operation = rand_r(&seed) % 3;
        int value = rand_r(&seed) % VALUE_RANGE;
        
        switch (operation) {
            case 0: // Insert
                insert_node(&head, value);
                add_expected(value);
                break;
            case 1: // Delete
                delete_node(&head, value);
                remove_expected(value);
                break;
            case 2: // Search
                search(head, value);
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
 * Comprehensive verification
 */
bool verify_list() {
    printf("verifying integrity...\n");
    bool result = true;
    
    // First, count actual nodes in the list
    int actual_count = count_nodes(head);
    
    // Get expected count from our tracking
    int expected_count = get_expected_count();
    
    // Compare counts first
    printf("node counts: actual=%d, expected=%d\n", actual_count, expected_count);
    if (actual_count != expected_count) {
        printf("verification fail: count mismatch\n");
        result = false;
    }
    
    // Now check each expected value is in the list the correct number of times
    pthread_mutex_lock(&expected_mutex);
    
    // 1. Count occurrences of each value in the list
    int* list_counts = calloc(BUCKET_SIZE, sizeof(int));
    if (list_counts == NULL) {
        perror("calloc failed for list_counts");
        pthread_mutex_unlock(&expected_mutex);
        return false;
    }
    
    pthread_rwlock_rdlock(&list_rwlock);
    Node* current = head;
    while (current != NULL) {
        if (current->data >= 0 && current->data < BUCKET_SIZE) {
            list_counts[current->data]++;
        } else {
            printf("verification fail: value %d out of expected range\n", current->data);
            result = false;
        }
        current = current->next;
    }
    pthread_rwlock_unlock(&list_rwlock);
    
    // 2. Compare with expected counts
    for (int i = 0; i < BUCKET_SIZE; i++) {
        int expected = expected_values[i].used ? expected_values[i].count : 0;
        if (list_counts[i] != expected) {
            printf("verification fail: value %d - found %d times, expected %d times\n", 
                   i, list_counts[i], expected);
            result = false;
        }
    }
    
    free(list_counts);
    pthread_mutex_unlock(&expected_mutex);
    
    return result;
}

/**
 * Runs the tests
 */
int main() {
    srand(time(NULL)); // Initialize random seed

    // Initialize expected values array
    init_expected_values();
    
    // Create threads
    pthread_t threads[NUM_THREADS];
    ThreadArg thread_args[NUM_THREADS];
    
    printf("starting %d threads w/ %d operations each\n", NUM_THREADS, OPERATIONS_PER_THREAD);
    
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
    
    printf("all threads complete\n");
    
    // Print the final list
    printf("Final ");
    print_list(head);
    
    // Verify integrity
    if (verify_list()) {
        printf("verification pass\n");
    } else {
        printf("verification fail\n");
    }
    
    // Clean up
    free_list(head);
    free(expected_values);
    
    return 0;
}