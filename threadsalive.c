#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <ucontext.h>
#include "list.c"
#include "list.h"

#include "threadsalive.h"

/* ***************************** 
     stage 1 library functions
   ***************************** */
ucontext_t main_ctx; //global variabe will not be placed in queue used for swapping purposes
struct node* list;

void ta_libinit(void) {
	list = NULL;
	getcontext(&main_ctx);
//    list = malloc(sizeof(struct node));
    return;
}

void ta_create(void (*func)(void *), void *arg) {
    unsigned char *stack = (unsigned char *)malloc(128000);
    ucontext_t ctx;
    getcontext(&ctx); //initiliaze context
    ctx.uc_stack.ss_sp = stack;
    ctx.uc_stack.ss_size = 128000;
    ctx.uc_link = &main_ctx;
    makecontext(&ctx, (void (*) (void)) func, 1, arg); //thread goes to func

    struct node *new_thread = malloc(sizeof(struct node));
    new_thread->next = NULL;
    new_thread->ctx = ctx;


    if (list == NULL){
	list = new_thread;
	return;
    }
    struct node *temp = list;

    while(temp->next != NULL){
	temp = temp -> next;
    }
    temp->next = new_thread;

    return;
}

void ta_yield(void) {
    if(list == NULL){
	return;
    }
    struct node *temp = list;
    struct node *current = list;
    while(temp->next != NULL){
	temp = temp->next;
    }
    temp->next = current;
    list = list->next;
    current->next = NULL;
    swapcontext(&current->ctx, &list->ctx);
    return;
}

int ta_waitall(void) {

    if (list == NULL)
       {return 0;}
    struct node *current_node = list;
    while(current_node != NULL){
	swapcontext(&main_ctx, &current_node->ctx);
	current_node = current_node->next;
    }
	printf("Jack's fault\n");
    while(list!=NULL){
	struct node *temp = list;
 	list = list->next;
	free(temp->ctx.uc_stack.ss_sp);
	free(temp);
    }
    return 0;
}


/* ***************************** 
     stage 2 library functions
   ***************************** */

void ta_sem_init(tasem_t *sem, int value) {
    sem->count = value;
    sem->sem_list = malloc(sizeof(struct node));
}

void ta_sem_destroy(tasem_t *sem) {
    while(sem->sem_list->next != NULL){
	list_append(&sem->sem_list->ctx, &list);
	struct node *temp = sem->sem_list;
	sem-> sem_list = sem->sem_list->next;
	free(temp);
    }
    free(sem);
}

void ta_sem_post(tasem_t *sem) {
	sem->count += 1;
	if((sem->count <= 0) && (sem->sem_list != NULL)){
	   swapcontext(&main_ctx, &sem->sem_list->ctx);
	}
}

void ta_sem_wait(tasem_t *sem) {
	if (sem->count > 0){
	   sem->count -=1;

	}
	if (sem == 0){
	   swapcontext(&main_ctx, &list->ctx);
	}

}

void ta_lock_init(talock_t *mutex) {
	mutex->sem = *(tasem_t*)malloc(sizeof(tasem_t));
	ta_sem_init(&mutex->sem,1);
}

void ta_lock_destroy(talock_t *mutex) {
	ta_sem_destroy(&mutex->sem);
	free(&mutex->sem);
}

void ta_lock(talock_t *mutex) {
	ta_sem_wait(&mutex->sem);
}

void ta_unlock(talock_t *mutex) {
	ta_sem_post(&mutex->sem);
}


/* ***************************** 
     stage 3 library functions
   ***************************** */
/*

void ta_cond_init(tacond_t *cond) {
	cond->cond_list = *(node*)malloc(sizeof(node));
}

void ta_cond_destroy(tacond_t *cond) {
    while(cond->cond_list->next != NULL){
	list_append(&cond->cond_list->ctx, &list);
	struct node *temp = cond->cond_list;
	cond-> cond_list = cond->cond_list->next;
	free(temp);
    }
    free(cond);
}
void ta_wait(talock_t *mutex, tacond_t *cond) {
    ta_unlock(mutex);
    struct node *temp = malloc(sizeof(struct node));
    *temp = *cond->cond_list;
    cond->cond_list = cond_list->next;
    list_append(&temp->ctx, &cond->cond_list); //putting it at the end of the list
    swapcontext(&temp->ctx, &cond->cond_list->ctx);
    return;
	ta_lock(mutex);
}

void ta_signal(tacond_t *cond) {
	ta_sem_post(&cond->sem);
}
*/

