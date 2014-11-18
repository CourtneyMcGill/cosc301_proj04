#include <stdio.h>
#include <stdlib.h>
#include "list.h"
#include <ucontext.h>

void list_append(struct node *thread, struct node **head){
   thread->next = NULL;
   struct node *temp = *head;
   if (*head == NULL){
     *head = thread;
  //   printf("first thing \n");
   }
   else{
      while(temp->next != NULL){
	  temp = temp->next;
	}
      temp->next = thread;
 //     printf("appended 2 \n");
   }
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
