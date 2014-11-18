
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

// COURTNEY MCGILL and PARKER REYNOLDS


ucontext_t main_ctx; //global variabe will not be placed in queue used for swapping purposes
struct node* list;
int destroyed_threads = 0;
int sem_blocked;

int cond_blocked;



void ta_libinit(void) {
	list = NULL;
	cond_blocked = 0;
	sem_blocked = 0;

    return;
}


void ta_create(void (*func)(void *), void *arg) {
    struct node *new_thread = malloc(sizeof(struct node));
    new_thread->next = NULL;
    unsigned char *stack = (unsigned char *)malloc(128000);
    getcontext(&new_thread->ctx); //initiliaze context
    new_thread->ctx.uc_stack.ss_sp = stack;
    new_thread->ctx.uc_stack.ss_size = 128000;
    new_thread->ctx.uc_link = &main_ctx;
    makecontext(&new_thread->ctx, (void (*) (void)) func, 1, arg); //thread goes to func
    list_append(new_thread, &list);
    return;
   }

void ta_yield(void) {
    	if(list == NULL){
		return;
    	}
   	struct node *temp = list;
    	list = list->next;
	list_append(temp, &list);
	if (list->next != NULL){
	
	}
    	swapcontext(&temp->ctx, &list->ctx);
    	return;
}
int ta_waitall(void) {

	while(list!=NULL){
		swapcontext(&main_ctx, &list->ctx);
		if(list!=NULL){
			struct node *temp = list;
			list = list->next;
			free(temp->ctx.uc_stack.ss_sp);
			free(temp);
		}
	}
	if(cond_blocked!=0 || sem_blocked != 0){
		printf("condblocked = %d \n", cond_blocked);
		printf("semblocked = %d \n", sem_blocked);
		return -1;
	}
	else{
		return 0;
	}

}
/* ***************************** 
     stage 2 library functions
   ***************************** */


void ta_sem_init(tasem_t *sem, int value) {
    sem->count = value;
    sem->sem_list = NULL;
}

void ta_sem_destroy(tasem_t *sem) {
    	while(sem->sem_list != NULL){
		struct node *temp = sem->sem_list;
		sem->sem_list = sem->sem_list->next;
		free(temp->ctx.uc_stack.ss_sp);
		free(&temp->ctx);
//		free(temp);
		destroyed_threads++;
	}
}


void ta_sem_post(tasem_t *sem) {
	//if there is a thread in blocked queue move it to front of ready queue, move current thread to end of ready queue, swap from current to the front of ready queue
	//if theres nothing on the semaphore queue then change counter and keep running
	sem->count++;
	if((sem->count > 0) && sem->sem_list != NULL){
	        sem_blocked --;
		printf("decremented sem blocked-- is now = %d \n", sem_blocked);
		//move current thread to end of ready list
		struct node *head = list;
		list = list->next;
		list_append(head, &list);
		// move front of blocked queue to front of ready queue
		struct node *temp = sem->sem_list;
		sem->sem_list = sem->sem_list->next;
		temp->next = list;
		list = temp; 
		//swap from current (now on back of ready queue) to front of queue
		swapcontext(&head->ctx, &list->ctx);
	}
	//the sem count is incremented regardless

}


void ta_sem_wait(tasem_t *sem) {
	//if value > 0 then decrement and keep running
	//if value of semaphore is 0 then you need to add yourself to the semqueue and take off ready queue context switch to new head of ready queue
	if(sem->count ==0){
	        sem_blocked ++;
		printf("increamented sem blocked-- is now = %d \n", sem_blocked);
		//take first node off of ready queue
		struct node *thread = list;
		list = list->next;
		//add current node to end of bloked queue
		list_append(thread, &sem->sem_list);
		//swap context to new head of redy queue
		swapcontext(&thread->ctx, &list->ctx);
	}
	else{
	    sem->count --;
	}

}
void ta_lock_init(talock_t *mutex) {
	//mutex->sem = *(tasem_t *)malloc(sizeof(tasem_t));
	ta_sem_init(&mutex->sem, 1);
}
void ta_lock_destroy(talock_t *mutex) {
    	ta_sem_destroy(&mutex->sem);
//	free(&mutex->sem);
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
void ta_cond_init(tacond_t *cond) {

        cond->cond_list = NULL;
}
void ta_cond_destroy(tacond_t *cond) {
    
    	while(cond->cond_list != NULL){
		struct node *temp = cond->cond_list;
		cond->cond_list = cond->cond_list->next;
		free(temp->ctx.uc_stack.ss_sp);
		free(&temp->ctx);
		free(&temp);
		destroyed_threads ++; 
	}
}
void ta_wait(talock_t *mutex, tacond_t *cond) {
     if(mutex==NULL || cond==NULL){
	 return;
     }
     ta_unlock(mutex);
     struct node *temp = list;
     list = list->next;
     list_append(temp, &cond->cond_list);
     cond_blocked ++;
     if(list == NULL){
	swapcontext(&temp->ctx, &main_ctx);
     }
	else{
		swapcontext(&temp->ctx, &list->ctx);
	}
	ta_lock(mutex);

}
void ta_signal(tacond_t *cond) {


    if (cond->cond_list != NULL){
        cond_blocked --;
	struct node *temp = cond->cond_list;
	cond->cond_list = cond->cond_list->next;
	list_append(temp, &list);

    }
}
