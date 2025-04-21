#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

pthread_rwlock_t list_rwlock = PTHREAD_RWLOCK_INITIALIZER; // Global reader-writer lock for thread synchronization
// Used rw lock to try to solve sync issues, turns out the real reason was bad pointer control.  Leaving it like this because it still works

typedef struct Node {
    int data;
    struct Node* next;
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
    new_node->next = NULL;
    return new_node;
}

/**
 * Insert a node at the first position
 */
void insert_node(Node** head_ref, int data) {
    pthread_rwlock_wrlock(&list_rwlock);
    
    Node *new_node = create_node(data);
    new_node->next = *head_ref;
    *head_ref      = new_node;   // update head while still locked - important!   

    pthread_rwlock_unlock(&list_rwlock);
}

/**
 * Deletes a given node
 */
void delete_node(Node **head_ref, int data) {
    pthread_rwlock_wrlock(&list_rwlock); // writers have exclusive access


    Node *head = *head_ref;
    if (head == NULL) { // empty list
        pthread_rwlock_unlock(&list_rwlock);
        return;
    }

    if (head->data == data) { // delete head

        Node *temp = head;
        *head_ref  = head->next;   // update head while locked
        free(temp);
        pthread_rwlock_unlock(&list_rwlock);
        return;
    }
    Node *prev = head;
    Node *cur  = head->next;

    while (cur != NULL && cur->data != data) {
        prev = cur;
        cur  = cur->next;
    }

    if (cur != NULL) {
        prev->next = cur->next;
        free(cur);
    }
    pthread_rwlock_unlock(&list_rwlock);
}

/**
 * Returns a given node, uses read lock instead of write lock
 * since it doesn't modify the list
 */
Node* search(Node* head, int data) {
    pthread_rwlock_rdlock(&list_rwlock);
    
    Node* cur = head;
    while (cur != NULL) {
        if (cur->data == data) {
            pthread_rwlock_unlock(&list_rwlock);
            return cur;
        }
        cur = cur->next;
    }
    
    pthread_rwlock_unlock(&list_rwlock);
    return NULL;    
}

/**
 * Checks if a value exists in the list
 */
bool contains(Node* head, int data) {
    return search(head, data) != NULL;
}

/**
 * Prints list contents
 */
void print_list(Node* head) {
    pthread_rwlock_rdlock(&list_rwlock);
    
    if (head == NULL) {
        printf("empty list\n");
        pthread_rwlock_unlock(&list_rwlock);
        return;
    }

    Node* cur = head;
    printf("List: ");
    while (cur != NULL) {
        printf("%d -> ", cur->data);
        cur = cur->next;
    }
    printf("END\n");
    
    pthread_rwlock_unlock(&list_rwlock);
}

/**
 * Count nodes in list
 */
int count_nodes(Node* head) {
    pthread_rwlock_rdlock(&list_rwlock);
    
    int count = 0;
    Node* cur = head;
    while (cur != NULL) {
        count++;
        cur = cur->next;
    }
    
    pthread_rwlock_unlock(&list_rwlock);
    return count;
}

/**
 * Free memory used by the list
 */
void free_list(Node* head) {
    pthread_rwlock_wrlock(&list_rwlock);
    
    Node* current = head;
    Node* next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    
    pthread_rwlock_unlock(&list_rwlock);
}