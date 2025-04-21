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
Node* insert(Node* head, int data) {
    Node* new_node = create_node(data);
    new_node->next = head;
    return new_node;
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

    head = insert(head, 20);
    head = insert(head, 15);
    head = insert(head, 10);
    head = insert(head, 5);
    printLL(head);
}