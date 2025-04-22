#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node* next;
    bool marked;
} Node;

// this may be unnecessary
bool CAS(int original, int expected, int update){
    if (original == expected){
        original = update;
        return true;
    }
    return false;
}

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
    new_node->marked = false;
    return new_node;
}

/**
 * Checks if the two given nodes are unmarked and connected
*/
bool nodes_safe(Node* prev, Node* cur){
    return (!prev->marked && !cur->marked) && prev->next == cur;
}

/**
 * Insert a node at the first position
 */
void insert_node(Node** head_ref, int data) {
    Node *new_node = create_node(data);
    while(true) {
        if(nodes_safe(*head_ref, (*head_ref)->next)){
            new_node->next = *head_ref;
            *head_ref = new_node;
            break;
        }
    }
}

/**
 * Deletes a given node
 */


// should this be a bool to return success or failure?
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
        if(nodes_safe(prev, cur)){
            if (cur->data == data){
                cur->marked = 1;
                prev->next = cur->next;
                free(cur);
                return; // success
            }
            else{
                return; // failure
            }
        }
    }

    return;           // failure
}

/**
 * Returns a given node
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
    Node* target = search(head, data);
    if (target == NULL)
        return false;
    return !target->marked; // if target is marked, we consider it missing
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