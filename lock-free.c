#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>

// Define the Node structure with atomic next pointer
typedef struct Node {
    int data;
    _Atomic(struct Node*) next;
    _Atomic bool marked;  // Flag for logical deletion
} Node;

/**
 * Allocates and creates a new node object
 */
Node* create_node(int data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        printf("malloc fail\n");
        exit(1);
    }
    new_node->data = data;
    atomic_store(&new_node->next, NULL); // Atomicity for protection
    atomic_store(&new_node->marked, false);
    return new_node;
}

/**
 * Lock-free insert a node at the first position
 */
Node* insert_begin(_Atomic(Node*) *head_ptr, int data) {
    Node* new_node = create_node(data);
    Node* expected;
    
    do {
        expected = atomic_load(head_ptr);
        atomic_store(&new_node->next, expected);
    } while (!atomic_compare_exchange_strong(head_ptr, &expected, new_node)); // Compare and swap like slides
    
    return new_node;
}
/**
 * Find a node, helper for deletion
 */
bool find(_Atomic(Node*) *head_ptr, int data, Node** pred_ptr, Node** cur_ptr) {
retry: { // Block to stop compiler warning (and in case lab machines are running old C)
    Node* pred = atomic_load(head_ptr);
    if (pred == NULL) {
        *pred_ptr = NULL;
        *cur_ptr = NULL;
        return false;
    }
    
    Node* cur = atomic_load(&pred->next);
    
    while (cur != NULL) {
        Node* succ = atomic_load(&cur->next);
        
        // Check if current node is marked
        if (atomic_load(&cur->marked)) {
            // Try to physically remove the logically deleted node
            if (!atomic_compare_exchange_strong(&pred->next, &cur, succ)) {
                // CAS failed, retry from the beginning
                goto retry;
            }
            cur = succ;
        } else {
            if (cur->data >= data) {
                *pred_ptr = pred;
                *cur_ptr = cur;
                return cur->data == data;
            }
            pred = cur;
            cur = succ;
        }
    }
    
    *pred_ptr = pred;
    *cur_ptr = NULL;
    return false;
    }
}

/**
 * Deletes a given node
 * Deletes logically first then for real
 */
bool delete_node(_Atomic(Node*) *head_ptr, int data) {
    Node* head = atomic_load(head_ptr);
    if (head == NULL) {
        printf("empty list\n");
        return false;
    }
    
    // Special case for head
    if (head->data == data) {
        bool expected = false; // Try to mark the head for deletion
        if (atomic_compare_exchange_strong(&head->marked, &expected, true)) {
            Node* next = atomic_load(&head->next); // Try to atomically replace the head
            if (atomic_compare_exchange_strong(head_ptr, &head, next)) {
                free(head);
            }
            return true;
        }
        return false;
    }
    
    Node* pred;
    Node* cur;
    bool found;
    
    while (true) {
        found = find(head_ptr, data, &pred, &cur);
        if (!found || cur == NULL) {
            printf("delete: value %d not found\n", data);
            return false;
        }
        
        bool expected = false; // Try to mark the node for deletion
        if (!atomic_compare_exchange_strong(&cur->marked, &expected, true)) {
            continue; // Already marked or cas failed, retry
        } // Node is now logically deleted

        Node* succ = atomic_load(&cur->next); // Update next pointer to physically remove the node
        if (atomic_compare_exchange_strong(&pred->next, &cur, succ)) {
            free(cur);
        }
        // If cas fail, the loop will call find() again to ensure deletion
        return true;
    }
}

/**
 * Wait-free return a node with a given value
 */
Node* search(_Atomic(Node*) *head_ptr, int data) {
    Node* cur = atomic_load(head_ptr);
    while (cur != NULL) {
        if (!atomic_load(&cur->marked) && cur->data == data) {
            return cur;
        }
        cur = atomic_load(&cur->next);
    }
    return NULL;
}

/**
 * Wait-free check if a value exists in the list
 */
bool contains(_Atomic(Node*) *head_ptr, int data) {
    return search(head_ptr, data) != NULL;
}

/**
 * Wait-free print list contents
 */
void print_list(_Atomic(Node*) *head_ptr) {
    Node* head = atomic_load(head_ptr);
    if (head == NULL) {
        printf("empty list\n");
        return;
    }

    Node* cur = head;
    printf("List: ");
    while (cur != NULL) {
        if (!atomic_load(&cur->marked)) {
            printf("%d -> ", cur->data);
        }
        cur = atomic_load(&cur->next);
    }
    printf("END\n");
}

/**
 * Wait-free count nodes in list
 */
int count_nodes(_Atomic(Node*) *head_ptr) {
    int count = 0;
    Node* cur = atomic_load(head_ptr);
    while (cur != NULL) {
        if (!atomic_load(&cur->marked)) { // Don't count logically deleted nodes
            count++;
        }
        cur = atomic_load(&cur->next);
    }
    return count;
}

/**
 * Free memory used by the list
 */
void free_list(_Atomic(Node*) *head_ptr) {
    Node* cur = atomic_load(head_ptr);
    Node* next;
    
    while (cur != NULL) {
        next = atomic_load(&cur->next);
        free(cur);
        cur = next;
    }
    atomic_store(head_ptr, NULL);
}