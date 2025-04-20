#include <stdio.h>

struct Node {
    char id;
    struct Node* next;
};

void printLL(struct Node head){
    struct Node cur = head;
    do {
        printf("%c, ",cur.id);
        cur = *cur.next;
    } while (cur.next != NULL);
}

// remove LL struct, make it have no tail. the node with next = NULL is a natural tail. maybe keep empty head, maybe don't.

int insert(struct Node *head, char id){
    struct Node new;
    new.id = id;
    new.next = head->next;
    head->next = &new;

    return 0;
}

int main(){
    struct Node head;
    head.id = 'h';
    head.next = NULL;

    insert(&head, 't');

    insert(&head, 'm');

    printLL(head);
}