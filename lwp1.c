#include "scheduler.h"
#include "lwp.h"
//#include "magic64.S"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>
#define BITMASK 0xFF
static tid_t next_tid = 1;
extern thread head;
extern thread tail;
extern int qlen;
extern scheduler roundRobin;
thread currThread;
thread fullList = NULL;

struct threadQ {
   thread myThread;
   struct threadQ *next;
};

struct threadQ *waitingThread;
struct threadQ *terminatedThread;


//stack size helper
static size_t calculate_stack_size(){
	long page_size = sysconf(_SC_PAGE_SIZE);
	if (page_size <= 0){
		//this was the reccomended default I saw somwhere for page size
		page_size = 4096; 
	}
	struct rlimit stack_limit; 
	if (getrlimit(RLIMIT_STACK, &stack_limit) == 0){
		size_t stack_size;
		//default to 8 MB if there's no limit
		if (stack_limit.rlim_cur == RLIM_INFINITY) {
    			stack_size = 8 * 1024 * 1024; 
		} 
		//otherwise use the provided stack size limit
		else {
			stack_size = stack_limit.rlim_cur;
		}
		
		size_t aligned_size = ((stack_size + page_size - 1) / page_size) * page_size; 
		return aligned_size;

	}
   else if (getrlimit(RLIMIT_STACK, &stack_limit) == -1) {
      perror("getrlimit failed to return resource limits");
      exit(1);
   }
	//return a default of 8 MB if all else fails
	return 8 * 1024 * 1024;
}

//wrapper function to preserve return values 
void wrap (lwpfun f, void *arg){
	int ret; 
	ret = f(arg); 
	lwp_exit(ret);
}

//make a thread and create its stack and context
tid_t lwp_create(lwpfun function, void *arg){
	thread new_thread = (thread)malloc(sizeof(thread));
	//if creation of new thread fails
	if (!new_thread){
		return NO_THREAD;	
	}
	
	size_t stack_size = calculate_stack_size(); 
	
	//allocate stack using nmap
	void *stack = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE |
		 MAP_ANONYMOUS | MAP_STACK, -1, 0);
	//check if mmap failed 
	if (stack == MAP_FAILED){
		perror("MMAP");
		free(new_thread); 
		exit(EXIT_FAILURE);
	}
	
	//setting up the new thread's stack
	new_thread->tid = next_tid++;
	new_thread->stack = stack; 
	new_thread->stacksize = stack_size;

	//this should move the base pointer to the start of our (new) stack
	new_thread->state.rbp = (unsigned long) ((char*)stack + stack_size);
	
	//Preserve Floating Point Unit
	new_thread->state.fxsave=FPU_INIT; 
	//load function into rdi 
	new_thread->state.rdi = (unsigned long)function; 
	//load arg for function into rsi 
	new_thread->state.rsi = (unsigned long)arg; 

	//Zero out all the remaining registers
	new_thread->state.rax = 0;
	new_thread->state.rbx = 0;
	new_thread->state.rcx = 0;
	new_thread->state.rdx = 0;
	new_thread->state.r8 = 0;
	new_thread->state.r9 = 0;
	new_thread->state.r10 = 0;
	new_thread->state.r11 = 0;
	new_thread->state.r12 = 0;
	new_thread->state.r13 = 0;
	new_thread->state.r14 = 0;
	new_thread->state.r15 = 0;
   	

	//move stack pointer to our base pointer
	new_thread->state.rsp = new_thread->state.rbp; 
	
	//make space for phony return address			
	new_thread->state.rsp -= sizeof(unsigned long);
	
	/*  	
   	//original placement of this alignment, not sure 
 	//we need it here, I think it's only neccessary once we've put everything
 	//we are going to use on the stack 
	//ensure that stack frame is 16 byte aligned
	if (new_thread->state.rsp % 16 != 0){
		new_thread->state.rsp -= sizeof(unsigned long);
	}
	*/

	//placing phony return address at top of stack
	*((unsigned long *)(new_thread->state.rsp)) = (unsigned long)lwp_exit;
	
	//decrement stack pointer and place the wrapper function pointer 
	new_thread->state.rsp -= sizeof(unsigned long);
	*((unsigned long *)(new_thread->state.rsp)) = (unsigned long)wrap; 
	
	//ensure that stack frame is 16 byte aligned 
	if (new_thread->state.rsp % 16 != 0){
		new_thead->state.rsp -= sizeof(unsigned long); 
	}
	
	//add thread to global list of threads 
	if(!fullList){
		fullList = new_thread;
	}
	else {
		thread temp = fullList; 
		while(temp->lib_one){
			temp = temp->lib_one; 
		}
		temp->lib_one = new_thread;
	}


	scheduler current_sched = lwp_get_scheduler(); 
	if (current_sched){ 
	//admit to scheduler
	current_sched->admit(new_thread);
	//keep track of what the current thread is
	currThread = new_thread; 
	}
	return new_thread->tid; 
}

void lwp_start(void) {
	thread new_thread = (thread) malloc(sizeof(thread));
	if (!new_thread) {
		perror("Malloc failed in lwp_start");
		return;
	}
	new_thread->tid = next_tid++;
   
	//Zero out all the registers
	new_thread->state.rax = 0;
	new_thread->state.rbx = 0;
	new_thread->state.rcx = 0;
	new_thread->state.rdx = 0;
	new_thread->state.r8 = 0;
	new_thread->state.r9 = 0;
	new_thread->state.r10 = 0;
	new_thread->state.r11 = 0;
	new_thread->state.r12 = 0;
	new_thread->state.r13 = 0;
	new_thread->state.r14 = 0;
	new_thread->state.r15 = 0;
	//preserve Floating Point Unit
	new_thread->state.fxsave=FPU_INIT;

	//settting stack in struct to null, to avoid later delalocation
	new_thread->stack = NULL;	   
	
	//admit to the scheduler
	scheduler currentSched = lwp_get_scheduler(); 
	if (currentSched){
		currentSched->admit(new_thread);
		currThread = new_thread; //set global var to current thread
	}
	lwp_yield();

}


void lwp_yield(void){
	scheduler currSched = lwp_get_scheduler();
     //DO CHECK IF NULL
	thread next_thread = currSched->next;
   thread old_thread = currThread; 
	if (next_thread == NULL) {
		exit(3); //CALL WITH TERMINATION STATUS OF CALLING THREAD
	}
	else if (currThread == NULL) {
		perror("No thread to swap with");
		exit(1);
	} //if there is a current and next thread
   
	else {//swap current thread's registers with next thread's
		swap_rfiles(&old_thread->state, &next_thread->state);
		currThread = new_thread;
      //go to back of scheduler, enables round robin
      currSched->admit(old_thread);
	}
}

void lwp_exit(int exitval) {
   //retrieve lower 8 bits
   int lower_bits = (unsigned int) exitval & BITMASK;
   currThread->status = lower_bits;
   //Place into terminated thread queue if no
   //threads are waiting
   if (waitingThread == NULL) {
      
      roundRobin->remove(currThread); 
      struct threadQ *threadStruct = malloc(sizeof(struct threadQ));
      threadStruct->myThread = currThread;
      threadStruct->next = NULL;
      //place into queue of thread waiting         
      if (terminatedThread == NULL) {
         terminatedThread = threadStruct;
      }
      else {
         struct threadQ *temp = terminatedThread;
         while (temp->next != NULL) {
            temp = temp->next;
         }  
         temp->next = threadStruct;
      }
   } 
   //ELSE GET FROM WAITING QUEUE

   lwp_yield();

}

tid_t lwp_wait(int *status) {
   if (terminatedThread == NULL) {
      
      roundRobin->remove(currThread); 
      struct threadQ *threadStruct = malloc(sizeof(struct threadQ));
      threadStruct->myThread = currThread;
      threadStruct->next = NULL;
      //place into queue of thread waiting         
      if (waitingThread == NULL) {
         waitingThread = threadStruct;
      }
      else {
         struct threadQ *temp = waitingThread;
         while (temp->next != NULL) {
            temp = temp->next;
         }  
         temp->next = threadStruct;
      }
   } 

}

//function to get the thread id from our global currentThread structure, this
//function trusts that yield properly updates the current thread each time that 
//the context is changed
tid_t lwp_gettid(void){
	//if there is no current thread
	if (currThread == NULL){
		return NO_THREAD;
	}
	return currThread->tid;
}

//function to get a thread from the global list of threads if it matches the 
//provided thread id
thread tid2thread(tid_t tid){
	thread temp = fullList;
	//while there are threads in the list
	while (temp){
		//if the temp thread has the correct tid, return it 
		if (temp->tid == tid){
			return temp; 
		}
		//keep traversing through the list of all threads
		temp = temp->lib_one; 
	}
	//if the tid is invalid, return NULL 
	return NULL;
}

void lwp_set_scheduler(scheduler sched) {
   //1). Initialize a new scheduler
   sched = malloc(sizeof(struct scheduler));
   if (sched == NULL) {
      perror("Malloc failed when setting scheduler");
      exit(1);
   }
   sched = {NULL, NULL, rr_admit, rr_remove, rr_next, rr_qlen};      
   //2). transfer threads from one scheduler to the next
   thread temp;
   while (temp = roundRobin->next != NULL) {
      sched->admit(temp);//admit to new scheduler
   }
   //Reassign global scheduler
   roundRobin = sched;
}
//return current value of global roundRobin scheduler
scheduler lwp_get_scheduler(void){
   return roundRobin;

}

