#include <lwp.h>
#include <stdio.h>

typedef struct scheduler {
   thread head;
   thread tail;
   void (*init)(void);
   void (*shutdown)(void);
   void (*admit)(thread new);
   void (*remove)(thread victim);
   thread (*next)(void);
   int (*qlen)(void);
} *scheduler;

struct scheduler rr = {NULL, NULL, rr_admit(), rr_remove(), rr_next(), rr_qlen};
//initialize empty queue
void *init() {
   rr->head =  NULL;
   rr->tail = NULL;
   rr->qlen = 0;
}

//admits new thread to scheduler
void *admit(thread new) {
   //if first thread admitted
   if (rr->head == NULL && rr->tail == NULL) {
      head = tail = new;
      new->sched_one = new;
   }
   else { //add to end of queue
      rr->tail->sched_one = new;
      rr->tail = new;
      new->sched_one = head;

   }

}
//finds thread in queue and removes it
void *remove(thread victim) {
   if (rr->head == NULL) {
      return;
   }
   //check if only one thread in queue
   else if (rr->head == rr->tail) {
      if  (rr->head == victim) {
         rr->head = rr->tail = NULL;
         *(qlen) = *(qlen) - 1;
      }
   }//check if thread is at the end
   else if (rr->tail == victim) {
      thread temp = rr->head;
      //sched_one ptr is used as next ptr
      while (temp->sched_one != rr->tail) {     
         temp = temp->sched_one;
      }
      rr->tail = temp;
      rr->tail->sched_one = rr->head;
      *(qlen) = *(qlen) - 1;
   }//if thread is the head 
   else if (rr->head == victim) {
      rr->head = rr->head->sched_one;
      rr->tail->sched_one = rr->head;
      *(qlen) = *(qlen) - 1;
   }
   else { //if thread is somewhere in the middle
      thread temp = rr->head;
      while (temp->sched_one != head) {
         if (temp->sched_one == victim) {
            temp->sched_one = victim->sched_one;
         }
      }
   }
}
