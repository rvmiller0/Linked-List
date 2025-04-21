#include <stdio.h>
#include <stdlib.h>

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
Node* insert_begin(Node* head, int data) {
    Node* new_node = create_node(data);
    new_node->next = head;
    return new_node;
}

/**
 * Insert a node at the last position
 */
Node* insert_end(Node* head, int data) {
    Node* new_node = create_node(data);
    if (head == NULL) {
        return new_node;
    }
    Node* cur = head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new_node;
    return head;
}

/**
 * Inserts a new node after a given node
 */
void insert_after(Node* prev_node, int data) {
    if (prev_node == NULL) {
        printf("node is null\n");
        return;
    }
    Node* new_node = create_node(data);
    new_node->next = prev_node->next;
    prev_node->next = new_node;
}

/**
 * Deletes a given node
 */
Node* delete_node(Node* head, int data) {
    if (head == NULL) {
        printf("empty list\n");
        return NULL;
    }
    if (head->data == data) {
        Node* temp = head;
        head = head->next;
        free(temp);
        return head;
    }
    Node* cur = head;
    Node* prev = NULL;

    while (cur != NULL && cur->data != data) {
        prev = cur;
        cur = cur->next;
    }

    if (cur == NULL) {
        printf("dnf\n");
        return head;
    }

    prev->next = cur->next;
    free(cur);
    return(head);
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
 * Prints list contents
 */
void printLL(struct Node* head){
    if (head == NULL) {
        printf("empty list");
        return;
    }

    Node* cur = head;
    printf("List: ");
    while (cur != NULL) {
        printf("%d -> ", cur->data);
        cur = cur->next;
    }
    printf("endL\n");
}

/**
 * Tests list
 */
int main(){
    Node* head = NULL;

    head = insert_begin(head, 10);
    head = insert_begin(head, 5);
    head = insert_end(head, 20);
    Node* second = head->next;
    insert_after(second, 15);
    printLL(head);
}