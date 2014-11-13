#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include <ucontext.h>

void list_append(ucontext_t *context, struct node **head){
    struct node *new_node = malloc(sizeof(struct node)); //make space on heap
    new_node->ctx = *context;
    new_node->next = NULL;
	if (*head == NULL){
	    *head = new_node;
	    return;
	}
    struct node *temp = *head;
    while(temp->next != NULL){
	temp = temp->next;
    }
    temp->next = new_node;
}

void list_clear(struct node *list) {
    while (list != NULL) {
        struct node *tmp = list;
        list = list->next;
        free(tmp);
    }
}

void list_print(struct node *list) {
    int i = 0;
    printf("In list_print\n");
    while (list != NULL) {
        printf("List item %d", i++);
        list = list->next;
    }
}
