#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

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
    Node *new_node = create_node(data);
    new_node->next = *head_ref;
    *head_ref      = new_node;   // update head while still locked - important!
}

/**
 * Deletes a given node
 */
void delete_node(Node **head_ref, int data) {
    Node *head = *head_ref;
    if (head == NULL) { // empty list
        return;
    }

    if (head->data == data) { // delete head

        Node *temp = head;
        *head_ref  = head->next;   // update head while locked
        free(temp);
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
}

/**
 * Returns a given node, uses read lock instead of write lock
 * since it doesn't modify the list
 */
Node* search(Node* head, int data) {
    Node* cur = head;
    while (cur != NULL) {
        if (cur->data == data) {
            return cur;
        }
        cur = cur->next;
    }
    
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
    if (head == NULL) {
        printf("empty list\n");
        return;
    }

    Node* cur = head;
    printf("List: ");
    while (cur != NULL) {
        printf("%d -> ", cur->data);
        cur = cur->next;
    }
    printf("END\n");
}

/**
 * Count nodes in list
 */
int count_nodes(Node* head) {
    int count = 0;
    Node* cur = head;
    while (cur != NULL) {
        count++;
        cur = cur->next;
    }
    return count;
}

/**
 * Free memory used by the list
 */
void free_list(Node* head) {
    Node* current = head;
    Node* next;
    
    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
}