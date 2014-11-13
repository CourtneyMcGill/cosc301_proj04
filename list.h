

#ifndef __LIST_H__
#define __LIST_H__
#include <ucontext.h>


struct node{
    ucontext_t ctx;
    struct node *next; 
};

void list_append(ucontext_t *ctx, struct node **head);


void list_clear(struct node *list);

void list_print(struct node *list);


/* your list data structure declarations */


/* your function declarations associated with the list */


#endif // __LIST_H__
