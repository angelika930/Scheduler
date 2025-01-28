#include <stdio.h>
#include <stdlib.h> 
#include "lwp.h"

static thread head = NULL;
static thread tail = NULL;
int qlen = 0;

//admits new thread to scheduler
void rr_admit(thread new) {

   if (new == NULL) {
      return;
   }
   //if first thread admitted
   if (head == NULL && tail == NULL) {
      head = tail = new;
      new->sched_one = new;
   }
   else { //add to end of queue
      tail->sched_one = new;
      tail = new;
      new->sched_one = head;

   }
   qlen++;
}
//finds thread in queue and removes it
void rr_remove(thread victim) {
   if (head == NULL) {
      return;
   }
   //check if only one thread in queue
   else if (head == tail) {
      if  (head == victim) {
         head = tail = NULL;
         qlen--;
      }
   }//check if thread is at the end
   else if (tail == victim) {
      thread temp = head;
      //sched_one ptr is used as next ptr
      while (temp->sched_one != tail) {     
         temp = temp->sched_one;
      }
      tail = temp;
      tail->sched_one = head;
      qlen--;
   }//if thread is the head 
   else if (head == victim) {
      head = head->sched_one;
      tail->sched_one = head;
      qlen--;
   }
   else { //if thread is somewhere in the middle
      thread temp = head;
      while (temp->sched_one != head) {
         if (temp->sched_one == victim) {
            temp->sched_one = victim->sched_one;
            qlen--;
         }
         temp = temp->sched_one;
      }
   }
}

thread rr_next() {
   if (head != NULL) {
      thread nextThread = head;
      return nextThread;
   }
   else {
      return NULL;
   }
}
int rr_qlen() {
   return qlen;
}

//define our roundRobin scheduler
struct scheduler rr = {NULL, NULL, rr_admit, rr_remove, rr_next, rr_qlen};
scheduler roundRobin = &rr;
