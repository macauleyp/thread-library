// File:	my_pthread.c
// Author:	Yujie REN
// Date:	January 2019

// List all group member's name: Macauley Pinto, Ryan Townsend, Itamar Levi
// username of iLab: msp194, il166, rrt51
// iLab Server: cd.css.rutgers.edu

#include "my_pthread_t.h"

// INITAILIZE ALL YOUR VARIABLES HERE
// YOUR CODE HERE



my_pthread_t currThread = 0;


#ifndef MLFQ
	
	int sched = 10;


#else 
	int sched = 11;

#endif



int initialized = -2;
int firstThread = -1;
int mutexCounter = 0;

unsigned long int x = 0;

int exitedYield = -1;



my_pthread_t createdThread = 0;
my_pthread_t completedThreadID = 0;
my_pthread_t threadToJoin = 0;

int completed = 0;


int join = 0;

int waitingToJoin = 0;
int cleanUpthread = -1;
node *threadToExit;

int mutID = 0;

int cleanUpThread = -1;
void *returnValue = NULL;

ucontext_t scheduleThreads;
ucontext_t exitThreadContext;
ucontext_t joinThreadContext;
ucontext_t interruptHandlerContext;




struct tcbNode * prevThread;

my_pthread_t allThreadsCreated = -1;



volatile int v;
struct mutexList;
struct runningThreadQueue *A[4]; 
struct runningThreadQueue *runningMLFQ;
struct timespec start, end;
int timerCount = 0;

mutexNode * mutexList;

my_pthread_mutex_t m;
my_pthread_mutex_t u;
my_pthread_mutex_t t;

my_pthread_t genThreadID(){

	return ++allThreadsCreated;
}
void initPrevThreadContext(){

	prevThread = (struct tcbNode *)malloc(sizeof(struct tcbNode));
	prevThread->threadBlock = (struct threadControlBlock *)malloc(sizeof(struct threadControlBlock));
	genThreadID();
	prevThread->threadBlock->id = allThreadsCreated;
	prevThread->threadBlock->context = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(prevThread->threadBlock->context);

	getcontext(&scheduleThreads);
	scheduleThreads.uc_stack.ss_sp = malloc(stackSize);
	scheduleThreads.uc_stack.ss_size = stackSize;
	scheduleThreads.uc_stack.ss_flags = 0;
	makecontext(&scheduleThreads,(void*)(*schedule), 0);


	getcontext(&exitThreadContext);
	exitThreadContext.uc_stack.ss_sp = malloc(stackSize);
	exitThreadContext.uc_stack.ss_size = stackSize;
	exitThreadContext.uc_stack.ss_flags = 0;

	getcontext(&joinThreadContext);
	joinThreadContext.uc_stack.ss_sp = malloc(stackSize);
	joinThreadContext.uc_stack.ss_size = stackSize;
	joinThreadContext.uc_stack.ss_flags = 0;
	
}


/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, 
                      void *(*function)(void*), void * arg) {
	// Create Thread Control Block
	// Create and initialize the context of this thread
	// Allocate space of stack for this thread to run
	// After everything is all set, push this thread into run queue

	//fprintf(stdout,"Creating Thread.\n");

	// YOUR CODE HERE

	
	if(initialized == -2){
		
		initPrevThreadContext(); //initializes main thread
		 
		initQueues();  //initialized queues

		initialized = -1;

	}
	
	struct tcbNode *newThread = (struct tcbNode *)malloc(sizeof(struct tcbNode));
	

	if(!newThread){
		fprintf(stderr,"Error allocating stack memory\n");
		exit(1);
	}

	//allocate space for TCB and initialize context for new Thread
	newThread->threadBlock = (struct threadControlBlock *)malloc(sizeof(struct threadControlBlock));
	
	genThreadID();		//assign ThreadID
	newThread->threadBlock->id = allThreadsCreated;
	*thread = newThread->threadBlock->id;

//	fprintf(stdout,"Thread ID: %d | newThread: %d\n",*thread,newThread->threadBlock->id);	

	
	newThread->threadBlock->context = (ucontext_t *)malloc(sizeof(ucontext_t));
	getcontext(newThread->threadBlock->context);
	(*newThread->threadBlock->context).uc_stack.ss_sp = malloc(stackSize);
	(*newThread->threadBlock->context).uc_stack.ss_size = stackSize;
	(*newThread->threadBlock->context).uc_stack.ss_flags = 0;
	
	newThread->threadBlock->priority = 0;
	newThread->threadBlock->join = 0;
	newThread->threadBlock->elapsed = 0;

	
	newThread->threadBlock->func = function;
	newThread->threadBlock->args = arg;
	
	
	newThread->threadBlock->status = READY;




	(*newThread->threadBlock->context).uc_link = prevThread->threadBlock->context;
	makecontext(newThread->threadBlock->context, (void*)(*threadWrapper), 0);
	
	prevThread = newThread;

	//fprintf(stdout,"P_THREAD_CREATE: Queueing Thread\n");

	enQueueState(newThread,RUNNING, sched, 0);
	
	if(initialized == -1){
	
		initializeTimerInterrupt();
	}
	
	return 0;
	
};
void threadWrapper(){
	

	node *threadToExecute;
	if(sched == 10){
		threadToExecute = deQueueState(RUNNING);
		threadToExecute->threadBlock->returnValue = threadToExecute->threadBlock->func(threadToExecute->threadBlock->args);
		threadToExecute->threadBlock->status = COMPLETE;

		my_pthread_exit(threadToExecute); 

	}else{
		threadToExecute = runningMLFQ->front;
		
		threadToExecute->threadBlock->returnValue = threadToExecute->threadBlock->func(threadToExecute->threadBlock->args);
		returnValue = threadToExecute->threadBlock->returnValue;
		threadToExecute->threadBlock->status = COMPLETE;

		//fprintf(stdout,"Exiting thread %d....\n",threadToExecute->threadBlock->id);

		my_pthread_exit(threadToExecute); 
	}
	

}
void updateRunningThreadsPriority(){
		
	node *dummy = runningQueue->front;
	while(dummy){
		if(dummy->threadBlock->status == RUNNING){

			//fprintf(stdout,"Thread %d Priority: %d.\n",dummy->threadBlock->id,dummy->threadBlock->priority);

			++dummy->threadBlock->priority;
		}
		dummy = dummy->next;
	}

}
void initializeTimerInterrupt(){

	//fprintf(stdout,"Initializing Handler\n");
	
	if(signal(SIGALRM,timerInterruptHandler) == SIG_ERR){
		//fprintf(stderr,"Unable to catch alarm.\n");

		exit(1);

	}
	timer.it_value.tv_sec = INTERVAL/1000;
	timer.it_value.tv_usec = (INTERVAL*1000) % 1000000;	
	timer.it_interval = timer.it_value;

	if(setitimer(ITIMER_REAL,&timer,NULL) == -1){
		//fprintf(stderr,"Error setting timer.\n");

		exit(1);

	}

	initialized = 1;
	while(1){	
		if(join == 1){

			pause();

			//printf("After pause\n");

			if(exitedYield == 1){

	
				return;
			}

		}else{

			break;

		}
			
	}
   initialized = -1;
}
void timerInterruptHandler(){

	//fprintf(stdout,"Interrupt Occured.\n");
	
	if(sched == 10){
		updateRunningThreadsPriority(); 
		//runningThread->threadBlock->priority = runningThread->threadBlock->priority + 1;
		if(waitingToJoin != 0){
			//fprintf(stdout,"Cleaning up thread\n");
			
			waitingToJoin = 0;
			node *threadToDestroy = findByThreadID(threadToJoin,sched);
			node *remove = removeCompletedThread(threadToDestroy->threadBlock->id,COMPLETE);
			setcontext(&exitThreadContext);
		}
		
	}else{

		

		clock_gettime(CLOCK_REALTIME, &end);
			
		if(waitingToJoin != 0){
			//fprintf(stdout,"Cleaning up thread\n");
			
			waitingToJoin = 0;
			
			
			setcontext(&exitThreadContext);
		}


		if(completed < allThreadsCreated){

			if(timerCount>=100){	
			
				shiftThreadsInQueue(A);
				timerCount=0;
				//printMLFQ(A);
			}
		}
	
	
	
	}


	if(completed < allThreadsCreated){
			int thread = my_pthread_yield();
	
	}

	
	
	if(exitedYield == 1){

		return;
	}	
}

/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield() {
	// Change thread state from Running to Ready
	// Save context of this thread to its thread control block
	// Switch from thread context to scheduler context
	
	// YOUR CODE HERE
		
	//fprintf(stdout,"{YIELD}\n");

	if(sched == 10){

		node *setRunningThreadToReady = runningQueue->front;
		
		if(!setRunningThreadToReady){
			
			//fprintf(stdout,"No items in queue\n");

			exitedYield = 1;
			return completed;

		} 
		
		while(setRunningThreadToReady){
			
			if(setRunningThreadToReady->threadBlock->status == RUNNING){

				setRunningThreadToReady->threadBlock->status = READY;	
				runningQueue->readySize = runningQueue->readySize + 1;
				break;
				
			}
			setRunningThreadToReady = setRunningThreadToReady->next;					
		}

		if(!setRunningThreadToReady){
			setRunningThreadToReady = runningQueue->front;
			
		}
		
		//outputCompletedThreads();
		if(firstThread == -1){
			firstThread = 1;
			schedule();
		}else{
		
			//fprintf(stdout,"Thread: %d status is %d\n",setRunningThreadToReady->threadBlock->id,setRunningThreadToReady->threadBlock->status);


			if(setRunningThreadToReady->threadBlock->status == COMPLETE){
				//fprintf(stdout,"{YIELD} Completed Thread %d\n",setRunningThreadToReady->threadBlock->id);

				node *remove = removeCompletedThread(setRunningThreadToReady->threadBlock->id,COMPLETE);

				my_pthread_yield();

				
						

				
			}else if(setRunningThreadToReady != NULL && setRunningThreadToReady->threadBlock != NULL && setRunningThreadToReady->threadBlock->id != 0){
				//fprintf(stdout,"Threads completed %d\n",completed);
				swapcontext(setRunningThreadToReady->threadBlock->context,&scheduleThreads);

			}

		}		

		return completed;

	}else{

		if(firstThread == -1){
			//printf("Here\n");
			//printMLFQ(A);
			firstThread = 1;
			schedule();

		}else{
		//	printMLFQ(A);
		//	printf("Here 2\n");
			node *setMeToReady = runningMLFQ->front;
			setMeToReady->threadBlock->status = READY;

			if(setMeToReady->queueLevel == 0 && setMeToReady->threadBlock->elapsed > 50){

				//printf("THREAD %d HAS RUN FOR %d: GREATER THAN ALLOWED: 50 SHIFTING FROM LEVEL %d TO LEVEL %d.\n",setMeToReady->threadBlock->id,setMeToReady->threadBlock->elapsed,setMeToReady->queueLevel,setMeToReady->queueLevel+1);
				setMeToReady->threadBlock->elapsed = 0;
				enQueueState(setMeToReady, READY, sched, setMeToReady->queueLevel+1);

			}else if(setMeToReady->queueLevel == 1 && setMeToReady->threadBlock->elapsed > 100){

				//printf("THREAD %d HAS RUN FOR %d: GREATER THAN ALLOWED: 100 SHIFTING FROM LEVEL %d TO LEVEL %d.\n",setMeToReady->threadBlock->id,setMeToReady->threadBlock->elapsed,setMeToReady->queueLevel,setMeToReady->queueLevel+1);
				setMeToReady->threadBlock->elapsed = 0;
				enQueueState(setMeToReady, READY, sched, setMeToReady->queueLevel+1);

			}else if(setMeToReady->queueLevel == 2 && setMeToReady->threadBlock->elapsed > 150){

				//printf("THREAD %d HAS RUN FOR %d: GREATER THAN ALLOWED: 150 SHIFTING FROM LEVEL %d TO LEVEL %d.\n",setMeToReady->threadBlock->id,setMeToReady->threadBlock->elapsed,setMeToReady->queueLevel,setMeToReady->queueLevel+1);
				setMeToReady->threadBlock->elapsed = 0;
				enQueueState(setMeToReady, READY, sched, setMeToReady->queueLevel+1);

			}else{

				//printf("THREAD %d HAS RUN FOR %d: lower than allowed, or at max THAN ALLOWED: 200 NOT SHIFTING FROM LEVEL %d.\n",setMeToReady->threadBlock->id,setMeToReady->threadBlock->elapsed,setMeToReady->queueLevel);
				//enqueue same case for level 3++ and no drop
				enQueueState(setMeToReady, READY, sched, setMeToReady->queueLevel);

			}

			timerCount++;
			int timeDifference = ((end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000);
			setMeToReady->threadBlock->elapsed += timeDifference;

			if(setMeToReady != NULL && setMeToReady->threadBlock != NULL && setMeToReady->threadBlock->id != 0){
				
				swapcontext(setMeToReady->threadBlock->context,&scheduleThreads);
		
			}




		}	
		return completed;
	}
};

/* terminate a thread */
void my_pthread_exit(void *value_ptr) {
	// Deallocated any dynamic memory created when starting this thread

	// YOUR CODE HERE

	node *threadToDestroy = (node *)value_ptr;
	threadToJoin = threadToDestroy->threadBlock->id;
	
	//fprintf(stdout,"{PTHREAD - EXIT} Thread %d\n",threadToDestroy->threadBlock->id);

	threadToDestroy->threadBlock->join = 1;

	
	completed++;

	if(sched == 10){
		
		
		
		threadToDestroy->threadBlock->status = COMPLETE;

		
		getcontext(threadToDestroy->threadBlock->context);
		returnValue = threadToDestroy->threadBlock->returnValue;
		
		

		if(waitingToJoin == 0){
			waitingToJoin = 1;
			//fprintf(stdout,"Going to Join Context with return value %s\n",(char *)returnValue);
			setcontext(&joinThreadContext);

		}else{		

		
			//fprintf(stdout,"Removing...\n");
			getcontext(&exitThreadContext);

			if(waitingToJoin == 1){
				//fprintf(stdout,"Going to interrupt...\n");
				
				
				timerInterruptHandler();
	
			}else{
	
				//fprintf(stdout,"Freeing...\n");
			
				free(threadToDestroy->threadBlock->context);
				threadToDestroy->threadBlock->context = NULL;
				free(threadToDestroy->threadBlock);
				threadToDestroy->threadBlock = NULL;
				free(threadToDestroy);
				threadToDestroy = NULL;
				
				int status = setcontext(&scheduleThreads);
			}
			
		}
	}else{
		node* remove = runningMLFQ->front;
		if(!remove){
			//fprintf(stderr,"REMOVE IS NULL\n");
		}else{

			//remove->next=NULL;
			
			//remove->threadBlock->status = COMPLETE;

			

			getcontext(threadToDestroy->threadBlock->context);

			if(waitingToJoin == 0){
				waitingToJoin = 1;
				//fprintf(stdout,"Going to Join Context with return value %s\n",(char *)returnValue);
				setcontext(&joinThreadContext);

			}else{		

		
				//fprintf(stdout,"Removing...\n");
				getcontext(&exitThreadContext);

				if(waitingToJoin == 1){
				//fprintf(stdout,"Going to interrupt...\n");
				
				
					timerInterruptHandler();
	
				}else{
	
				//fprintf(stdout,"Freeing...\n");
			
					free(threadToDestroy->threadBlock->context);
					threadToDestroy->threadBlock->context = NULL;
					free(threadToDestroy->threadBlock);
					threadToDestroy->threadBlock = NULL;
					free(threadToDestroy);
					threadToDestroy = NULL;
				
					int status = setcontext(&scheduleThreads);
				}
			
			}
		
		}
			
	}
	
};

/* wait for thread termination */
//Change to completed queue later instead of ready queue
int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	// Waiting for a specific thread to terminate
	// Once this thread finishes,
	// Deallocated any dynamic memory created when starting this thread
  
	// YOUR CODE HERE
	int counter = 0;
	
	//fprintf(stdout,"Waiting for thread %d\n",thread);
	node *dummy = findByThreadID(thread,sched);
	
	
	join = 1;
	getcontext(&joinThreadContext);
	if(waitingToJoin == 0){
		if(firstThread != -1){
			//fprintf(stdout,"Going to Original Context\n");
			returnValue = NULL;
			waitingToJoin = 1;
		
			node *removeThread;
			if(sched == 10){
				
				removeThread = findByThreadID(threadToJoin,sched);

			}else{

				removeThread = runningMLFQ->front;

			} 
		//	fprintf(stdout,"{JOIN} %s\n",(char *)removeThread->threadBlock->returnValue);
			setcontext(removeThread->threadBlock->context);
		}else{
			
			while(dummy){
				//fprintf(stdout,"{Yielding....}\n");
				my_pthread_yield();	
			
			}	
		}
	}else{
		if(value_ptr != NULL && dummy != NULL){
			//fprintf(stdout,"Set Return Value\n");
			*value_ptr = returnValue;
	
		}
		
		//join = 0;	
		waitingToJoin = 0;
		return 0;
	}
	//fprintf(stdout,"Returning from join\n");
	
	return 0;
};

/*

		while(dummy){
		
			if(dummy->threadBlock == NULL){
			
				return 0;
			}
		//printf("dummy->threadBlock: %d\n", dummy->threadBlock->id);

			if(dummy->threadBlock->id == thread && dummy->threadBlock->join == 0){
					
					if(dummy->threadBlock->join == 1){
						//fprintf(stdout,"Break\n");
						dummy->threadBlock->status = COMPLETE;
						break;
		
					}else{
						//sleep(100);
						
						my_pthread_yield();
						continue;
					}
					
					
			}else{

				dummy = dummy->next;


			}

		}
*/

void appendMutex(mutexNode * head,mutexNode * mutex){
	mutexNode * ptr = head;
	if(ptr==NULL){
		head = mutex;
		head->next=NULL;
		return;
	}
	while(ptr->next!=NULL){
		ptr=ptr->next;
	}
	ptr->next = mutex;
	mutex->next = NULL;
	mutexCounter++;
	return;
}
void addToBlockedList(my_pthread_mutex_t* mutex, node* thread){
	
	if(mutex->blockedList==NULL){
		mutex->blockedList = (node *) malloc(sizeof(node));
		mutex->blockedList = thread;
		mutex->front = thread;
		mutex->blockedList->next =NULL;
	}
	
	else{
		node* ptr = mutex->blockedList;
		while(ptr!=NULL){
			ptr=ptr->next;
		}
		ptr = thread;
		//if stcf enqueuestate
		//if mlqf next = null
		
	}
}

node* removeBlockedList(my_pthread_mutex_t *mutex){
	if(mutex->front==NULL){
		return NULL;
	}
	node* ptr = mutex->front;
	mutex->front = mutex->front->next;
	return ptr;

}
/* initialize the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	// Initialize data structures for this mutex
	//printf("INITIALIZING MUTEX INIT\n");
	
	// YOUR CODE HERE
	
	
	mutexNode *newMutex = (mutexNode *)malloc(sizeof(mutexNode));
	newMutex->mutex=mutex;
	newMutex->mutex->id = mutexCounter;
	mutexCounter++;
	newMutex->mutex->flag = 0;
	newMutex->mutex->holder = NULL;
	newMutex->mutex->blockedList = NULL;
	newMutex->mutex->front = NULL;
	appendMutex(mutexList, newMutex);
	
	//printf("LEAVING MUTEX INITIALIZER\n");
	return 0;
};

/* acquire the mutex lock */


int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	// Use the built-in test-and-set atomic function to test the mutex
	// If mutex is acquired successfuly, enter critical section
	// If acquiring mutex fails, push current thread into block list 
	// and context switch to scheduler 

	// YOUR CODE HERE
	//check if mutex is already locked
		//if already locked
			//calling thread is blocked
		//else lock thread
			//set owner to running thread

#ifndef MLFQ
	if(mutex->flag==0){
		mutex->holder = runningQueue->front;//changed from threadbock to front
		//printf("MUTEX ACQUIRED BY THREAD %d\n",mutex->holder->id);
		while(__sync_lock_test_and_set(&mutex->flag,1));
		//__sync_lock_release(&mutex);	
	}else{
		node *ptr = deQueueState(RUNNING);
		//printf("MUTEX BLOCKING THREAD %d\n",ptr->threadBlock->id);
		enQueueState(ptr, BLOCKED, sched, 0);
		addToBlockedList(mutex,ptr);//be sure to preserve pointer ties
		swapcontext(ptr->threadBlock->context,&scheduleThreads);
	}
#else
	if(mutex->flag==0){
		mutex->holder = runningMLFQ->front;
		while(__sync_lock_test_and_set(&mutex->flag,1));
	}else{
		node *ptr = runningMLFQ->front;
		addToBlockedList(mutex,ptr);//be sure to cut off pointer ties
		swapcontext(ptr->threadBlock->context,&scheduleThreads);
	}

#endif
	//printf("LEAVING LOCK\n");
	//printf("leaving mutex lock\n");
	return 0;
};

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	
	// Release mutex and make it available again. 
	// Put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	//printf("INSIDE UNLOCK\n");

#ifndef MLFQ
	node *ptr=runningQueue->front;
	if(ptr->threadBlock->id==mutex->holder->threadBlock->id){
			//printf("the holder thread is attempting to unlock the mutex\n");
			//printf("MUTEX RELEASED BY THREAD %d\n",mutex->holder->id);
		//__sync_lock_release(&mutex);

		//below is version 1
		__sync_lock_test_and_set(&mutex->flag,0);

		//block below is version 2
		//__sync_synchronize();
		//mutex->flag=0;
	
		ptr = mutex->front;
		
		while(ptr != NULL){
			ptr=deQueueState(BLOCKED);
			if(ptr == NULL) break;
			enQueueState(ptr, READY, sched, 0);
			ptr = ptr->next;
		}

		
	}
	//printf("LEAVING UNLOCK\n");
	
#else


	node *ptr=runningMLFQ->front;
	if(ptr->threadBlock->id==mutex->holder->threadBlock->id){
		__sync_lock_test_and_set(&mutex->flag,0);
	}	
	
	node *ptr2 = removeBlockedList(mutex);
	while(ptr2!=NULL){
		
		//create a function to pull node from list
		//ptr2->next=NULL;
		enQueueState(ptr2,RUNNING,sched,ptr2->queueLevel);
		
		ptr2 = removeBlockedList(mutex);
	}
	//removeBlockedList(mutex);
#endif
	return 0;
};


/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	// Deallocate dynamic memory created in my_pthread_mutex_init
	//printf("INSIDE DESTROY\n");
	
	//find *mutex in mutex list by ID
		//disconnect it
		//free allocated

	mutexNode*ptr = mutexList;
	mutexNode *prev;
	while(ptr!=NULL){
		if(ptr->mutex->id == mutex->id){
			prev->next=ptr->next;
			break;
		}
		prev=ptr;
		ptr=ptr->next;
	}
	//edge cases
	free(ptr);
	ptr = NULL;
	//free(mutex->holder);
	//printf("438\n");
	//mutex->holder = NULL;
	//printf("440\n");
	//free(mutex);
	//printf("442\n");
	//mutex = NULL;
	//printf("LEAVING DESTROY\n");
	return 0;
};


/* scheduler */
static void schedule() {
	// Every time when timer interrup happens, your thread library 
	// should be contexted switched from thread context to this 
	// schedule function
		
	// Invoke different actual scheduling algorithms
	// according to policy (STCF or MLFQ)

	// YOUR CODE HERE

// schedule policy

	if(sched == 10){
	
		sched_stcf();

	}else{
//#else 
	sched_mlfq();

	}
//#endif

}

/* Preemptive SJF (STCF) scheduling algorithm */
static void sched_stcf() {
	// Your own implementation of STCF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	//fprintf(stdout,"{STCF} In scheduler.\n");

	

	
	node *threadToSchedule = deQueueByPriority();
	
	if(!threadToSchedule){

		//fprintf(stdout,"No items in ready queue\n");

	}	
	
	while(completed < allThreadsCreated){
	
		
		threadToSchedule = deQueueByPriority();	
		if(!threadToSchedule) break;
		if(threadToSchedule->threadBlock->status == COMPLETE){

			//fprintf(stdout,"{SCHEDULER} Completed Thread %d\n",threadToSchedule->threadBlock->id);
	
			continue;
			

			
		}else{

					
			threadToSchedule->threadBlock->status = RUNNING;
		
			//fprintf(stdout,"{SCHEDULER - COMPLETED - %d} Thread %d status %d\n",completed,threadToSchedule->threadBlock->id,threadToSchedule->threadBlock->status);

		

			int status = swapcontext(&scheduleThreads,threadToSchedule->threadBlock->context);
			
			

			if(threadToSchedule != NULL && threadToSchedule->threadBlock != NULL && threadToSchedule->threadBlock->id != 0){

				//fprintf(stdout,"{Scheduler} Thread %d status %d\n",threadToSchedule->threadBlock->id,threadToSchedule->threadBlock->status);
				

			}
					
			
			getcontext(&scheduleThreads);
			
				

		}
			

	}
	
	
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// Your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
	//sched = 11;

	
	while(completed < allThreadsCreated){
		//printMLFQ(A);
		node* threadToSchedule = dequeueMLFQ(A);
		//threadToSchedule->next=NULL;
		runningMLFQ->front = threadToSchedule;
		runningMLFQ->rear = threadToSchedule;
			
		//fprintf(stdout,"{SCHEDULER - MLFQ} Scheduling thread %d\n",threadToSchedule->threadBlock->id);
		if(threadToSchedule->threadBlock == NULL) break;//segfaults when removed
		if(threadToSchedule->threadBlock->status == COMPLETE){
				//yield?
				continue;
		}

		threadToSchedule->threadBlock->status = RUNNING;
			
		clock_gettime(CLOCK_REALTIME, &start);

		int status = swapcontext(&scheduleThreads,threadToSchedule->threadBlock->context);

		if(threadToSchedule != NULL && threadToSchedule->threadBlock != NULL && threadToSchedule->threadBlock->id != 0){

			//f//printf(stdout,"{Scheduler - MLFQ} Thread %d status %d\n",threadToSchedule->threadBlock->id,threadToSchedule->threadBlock->status);	
		}


		getcontext(&scheduleThreads);	
		
		

	
	}
	

	
}

// Feel free to add any other functions you need

// YOUR CODE HERE
void *testThreads(void *args){
	//printf("In user function.\n");
	int i;

	for(i = 0; i < 1000;i++){
		//printf("X: %d\n",x);
		x++;	
		//my_pthread_yield();	

	}

	//printf("X is %d\n",x);

	
	return "Successssssssssss";
	
}
/*int main(void){

	int numThreads = 3;
	my_pthread_t pid[numThreads];
	
	int i;
	my_pthread_mutex_init(&m,NULL);
	my_pthread_mutex_init(&u,NULL);
	my_pthread_mutex_init(&t,NULL);
	for(i = 0; i < numThreads; i++){

		my_pthread_create(&pid[i],NULL,&testThreads,NULL);
		
	}
		

	for(i = 0; i < numThreads; i++){

		void *result;
		my_pthread_join(pid[i],&result);
		printf("Thread %d returned: %s\n",pid[i],((char *)result));

	}
	
	unsigned long int expectedValue = numThreads *1000;
	printf("Expected X value is %d\n",expectedValue);
	printf("Final X value is %d\n",x);
	
	
	
	return 0;
}
*/
/*
##################################################################################################################################
#																																#
#																																#
#														  HELPER FUNCTIONS														#
#																																#
#																																#
##################################################################################################################################
*/


void initQueues(){

	blockedQueue = (struct blockedThreadQueue *)malloc(sizeof(struct blockedThreadQueue));
	blockedQueue->rear = NULL;
	blockedQueue->front = NULL;
	blockedQueue->sizeOfQueue = 0;


	runningQueue = (struct runningThreadQueue *)malloc(sizeof(struct runningThreadQueue));
	runningQueue->rear = NULL;	
	runningQueue->front = NULL;
	runningQueue->readySize = 0;
	runningQueue->runningSize = 0;

	
	completedThreads = NULL;

	int i;
	for(i=0;i<4;i++){
		A[i] = malloc(sizeof(struct runningThreadQueue));
		A[i]->front = NULL;
		A[i]->rear = NULL;
		//////printf("bop\n");
	}
	
	runningMLFQ = (struct runningThreadQueue*)malloc(sizeof(struct runningThreadQueue));
	runningMLFQ->rear = NULL;	
	runningMLFQ->front = NULL;
	runningMLFQ->readySize = 0;
	runningMLFQ->runningSize = 0;

	//below added by ryan
	mutexList = NULL;
	
	 
}

node *removeFromMLFQ(my_pthread_t threadID,struct runningThreadQueue **A){

	int i;
	node* ptr;
	node* prev;
	for(i=0;i<4;i++){
		if(A[i]->front->threadBlock->id == threadID){

			node *remove = A[i]->front;

			A[i]->front = A[i]->front->next;
		
		
		
			return remove;

		}else{
			node *curr = A[i]->front->next;
			node *prev = A[i]->front;
			
			while(curr!=NULL && curr->threadBlock->id != threadID){
				//printf("asdfj\n");
				prev = curr;
				curr = curr->next;	
				

			}
			if(!curr){

				//fprintf(stdout,"Could not find thread with ID %d\n",threadID);
			

			}else if(!curr->next){

				node *remove = A[i]->rear;

				A[i]->rear = prev;
			
				return remove;

			}else{
				node *remove = curr;
				//SEGFAULT HERE
				//printf("curr id: %d\n", curr->threadBlock->id);
				if(prev->threadBlock == NULL) return remove;
				//printf("prev id: %d\n", prev->threadBlock->id);
				
				prev->next = curr->next;
				
				
			
				return remove;
			

			}
		}
	}
	return NULL;


}


node* dequeueMLFQ(struct runningThreadQueue **A){
	//call from within sched_mlfq
	//find level that has a node
	//dequeue from that list by priority
	//return node
	
	//for each level
		//traverse list
		//find lowest priority
		//pull node from list
			//fix pointers
		//return node
		//dont change queuelevel here
		
		
	//printf("IN DEQUEUE FUNCTION\n");
	int i;
	node* ptr;

	for(i=0;i<4;i++){
		ptr=A[i]->front;
		if(ptr==NULL){
			continue;
		}

		A[i]->front=A[i]->front->next;
		if(A[i]->front==NULL){
	
			A[i]->rear = NULL;
		}
			
		ptr->next = NULL;
		//printMLFQ(A);
		return ptr;
	}
	

	return NULL;
}


void shiftThreadsInQueue(struct runningThreadQueue **A){
	int i;
	node *pointer;
	for(i = 3; i > 0; i--){
		pointer = A[i]->front; 
		//printf("%d\n", i);
		while(pointer != NULL){
			//printf("Pointer: %d\n", pointer->threadBlock->id);
			//printf("Pointing to: %d\n", A[i-1]->rear->threadBlock->id);
			if(A[i-1]->rear == NULL){
				A[i-1]->front = pointer;
			}
			else A[i-1]->rear->next = pointer;
			break;
		}
		A[i]->front = A[i]->rear = NULL;
		
	}
	//printf("DONE\n");
	pointer = A[0]->front;
	while(pointer!=NULL){
		pointer->queueLevel =0;
		pointer=pointer->next;
	}
}


void printMLFQ(struct runningThreadQueue **A){
	int i;
	
	for(i=0;i<4;i++){
		node *ptr = A[i]->front;

		printf("LEVEL %d\n",i);

		while(ptr!=NULL){

			printf("%d->",ptr->threadBlock->id);

			ptr=ptr->next;
		}

		printf("\n");

	}
}



//Use parameters to enqueue with state policy
void addToFront(tcb *data){

	struct tcbNode *newNode = (struct tcbNode *)malloc(sizeof(struct tcbNode));
	newNode->threadBlock =  (struct threadControlBlock *)malloc(sizeof(struct threadControlBlock));
	newNode->threadBlock = data;
	newNode->next = NULL;
	if(!completedThreads){
		
		
		completedThreads = newNode;
	
	}else{

		newNode->next = completedThreads;
		completedThreads = newNode;

	}
}

void enQueueState(node *block,int state, int schedulerType, int level){
	//fprintf(stdout,"{ENQUEUEING - %d} Thread %d \n",state,block->threadBlock->id);
	//stcf is 10
	//mlfq is 11

	if(schedulerType == 10){

		block->next = NULL;

		if(state == BLOCKED){
			if(!blockedQueue->rear){
				blockedQueue->rear = (node *)malloc(sizeof(node));
				blockedQueue->rear = block;
				blockedQueue->rear->next = NULL;
				blockedQueue->front = blockedQueue->rear;
			}else{
				blockedQueue->rear->next = block;
				block->next = NULL;
				blockedQueue->rear = block;
			}
			blockedQueue->sizeOfQueue++;
		}else{

			if (runningQueue->front == NULL){
				
				runningQueue->front = runningQueue->rear = block;
				block->next = NULL;

			}else{

				runningQueue->rear->next = block;
				block->next = NULL;
				runningQueue->rear = block;


			}
		
			if(state == READY){

				runningQueue->readySize = runningQueue->readySize + 1;
					
							

			}else{

			
				runningQueue->runningSize = runningQueue->runningSize + 1;


			}


		}
	}else{

		block->queueLevel = level;
	
		if(level > 3){
			//error
		}
		
		if(A[level]->front == NULL){
	
			
			A[level]->front = block;
			A[level]->rear = block;
			block->next=NULL;
			
		}else{
			node* ptr = A[level]->front;
			node* prev;
			while(ptr != NULL){
				prev = ptr;
				ptr = ptr->next;
			}
			prev->next = block;
			A[level]->rear = block;
			A[level]->rear->next = NULL;
			
			
			
		}
		//printMLFQ(A);

	}

}
node *deleteFromList(){

	node *remove = completedThreads;
	return remove;


}
node *checkIfInCompletedQueue(my_pthread_t threadID){

	node *dummy = completedThreads;
	while(dummy != NULL){

		if(dummy->threadBlock->id == threadID){
			
			//fprintf(stdout,"Found thread %d\n",dummy->threadBlock->id);
			return dummy;

		}else{
	
			dummy = dummy->next;

		}
	}
	
	return NULL;

}
void outputCompletedThreads(){


	node *dummy = completedThreads;

	//fprintf(stdout,"Threads Completed: ");

	while(dummy){

		//fprintf(stdout,"%d ->",dummy->threadBlock->id);

		dummy = dummy->next;

	}

	//fprintf(stdout,"\n");

}
node *findByThreadID(my_pthread_t threadID,int sched){


	if(sched == 10){

		node *dummy = runningQueue->front;
		while(dummy){

			if(dummy->threadBlock->id == threadID){
				//fprintf(stdout,"Found thread %d by ID\n",dummy->threadBlock->id);
				return dummy;

			}else{
	
				dummy = dummy->next;

			}
		
		}
		
	}else{
		int i;
		for(i=0;i<4;i++){
			node *ptr = A[i]->front;
		
			while(ptr!=NULL){
				if(ptr->threadBlock->id == threadID){

						return ptr;
			
				}else{

					ptr=ptr->next;

				}

			}
		
		}

	}
	
	

	return NULL;

}
node *removeCompletedThread(my_pthread_t threadID, int state){

	
	if(runningQueue->front->threadBlock->id == threadID){

		node *remove = runningQueue->front;

		runningQueue->front = runningQueue->front->next;
		
		
		
		return remove;

	}else{

		node *curr = runningQueue->front->next;
		node *prev = runningQueue->front;

		while(curr!=NULL && curr->threadBlock->id != threadID){
			//printf("asdfj\n");
			prev = curr;
			curr = curr->next;	
			

		}

		if(!curr){

			//fprintf(stdout,"Could not find thread with ID %d\n",threadID);
		

		}else if(!curr->next){

			node *remove = runningQueue->rear;

			runningQueue->rear = prev;
		
			return remove;

		}else{
			node *remove = curr;
			//SEGFAULT HERE
			//printf("curr id: %d\n", curr->threadBlock->id);
			if(prev->threadBlock == NULL) return remove;
			//printf("prev id: %d\n", prev->threadBlock->id);
			
			prev->next = curr->next;
			
			
		
			return remove;
		

		}


	}
	
	return NULL;



}
void setToComplete(my_pthread_t threadID){


	node *dummy = runningQueue->front;
	while(dummy){

		if(dummy->threadBlock->id == threadID){
			
			//fprintf(stdout,"Thread %d complete", dummy->threadBlock->id);
			dummy->threadBlock->status = COMPLETE;
			break;

		}else{
		
			dummy = dummy->next;
		}
	}


}
node *deQueueState(int state){
	
	if(state == BLOCKED){
		
		node *dummy = NULL;
		dummy = blockedQueue->front;			
		node *returnNode = NULL;

		if(!dummy){

			//fprintf(stderr,"Cannot dequeue empty blocked stack.\n");
			return NULL;

		}else{

			returnNode = dummy;
			if(dummy->next){
				
				dummy = dummy->next;
				blockedQueue->front = dummy;
		
			}else{
					
				blockedQueue->front = NULL;
				blockedQueue->rear = NULL;

			}

		}

		blockedQueue->sizeOfQueue--;
		return returnNode;

	}else{

		
		if(state == READY){

			node *ready = runningQueue->front;
			
			while(ready){


				if(ready->threadBlock->status == READY){
					runningQueue->readySize = runningQueue->readySize - 1;
					return ready;
											

				}else{

					ready = ready->next;

				}
			}
	
			if(ready == NULL){

				//fprintf(stdout,"No threads ready in queue\n");
				return NULL;
				
			}



		}else{

			node *running = runningQueue->front;
		
			while(running){


				if(running->threadBlock->status == RUNNING){
					runningQueue->runningSize = runningQueue->runningSize - 1;

					return running;
											

				}else{

					running = running->next;

				}
			}
			
			if(running == NULL){

				//fprintf(stdout,"No threads running in queue\n");
				return NULL;
				
			}

		}
	}

	return NULL;	
}

void outputPriority(){

	//fprintf(stdout,"------------------------------ OUTPUT PRIORITY ------------------------------\n");
	node *dummy;
	dummy = runningQueue->front;
	while(dummy){

		//fprintf(stdout,"{OUTPUT - PRIORITY - %d} Thread %d PRIORITY %d\n",dummy->threadBlock->status,dummy->threadBlock->id,dummy->threadBlock->priority);
		dummy = dummy->next;


	}

	//fprintf(stdout,"------------------------------ END OF OUTPUT --------------------------------\n");

}
node *deQueueByPriority(){

	node *dummy = runningQueue->front;
	if(!dummy){

		return NULL;
	
	}
	int min = runningQueue->front->threadBlock->priority;

	node *minThread = dummy;
	
	while(dummy){


		
		if((min > dummy->threadBlock->priority) && dummy->threadBlock->status != COMPLETE){
			minThread = dummy;
			min = dummy->threadBlock->priority;
		
	
		}

		dummy = dummy->next;
		
	}

	//node *removedMinThread = removeFromQueueState(minThread->threadBlock->id,READY);
	//fprintf(stdout,"{DEQUEUE - PRIORITY} Thread %d deQueued\n",minThread->threadBlock->id);
	outputPriority();
	return minThread;

}
node *removeFromQueueState(my_pthread_t threadID, int state){

	
	
	if(state == BLOCKED && blockedQueue->front->threadBlock->id == threadID){

		node *remove = blockedQueue->front;
		blockedQueue->front = blockedQueue->front->next;
		remove->next = NULL;
		return remove;

	}else if(state == RUNNING && runningQueue->front->threadBlock->id == threadID){
	
		
		return runningQueue->front;


	}else{
		node *curr;
		node *prev;
		node *currRear;
		if(state == BLOCKED){
			
			curr = blockedQueue->front->next;		
			currRear = blockedQueue->rear;
			
		}else{
			
			if(curr->threadBlock->id == threadID){


				return curr;

			}

			curr = runningQueue->front->next;
			currRear = runningQueue->rear;
			

		}
		
		
		


		while(curr != NULL && curr->threadBlock->id != threadID){
	
			
			prev = curr;
			curr = curr->next;
		}
		
		if(curr == NULL){

			//fprintf(stdout,"No node with the corresponding thread ID.\n");
			return NULL;

		}else if(curr->next == NULL){
			
			return runningQueue->rear;

		}else{
	
			return curr;

		}
		
	
	}
	
	return NULL;

}

int isEmptyState(int state){

	if(state == BLOCKED){

		if(blockedQueue->sizeOfQueue > 0){

			return 0;

		}else{
			return -1;

		}

	}else{

		if(state == RUNNING && runningQueue->runningSize > 0){

			return 0;

		}else if(state == RUNNING && runningQueue->runningSize <= 0){

			return -1;


		}else if(state == READY && runningQueue->readySize > 0){

			return 0;

		}else{
	
			return -1;

		}

	}	


}
node *removeCreatedNode(){


	node *remove = runningQueue->front->next;
	runningQueue->front->next = NULL;
	return remove;

}
void outputQueue(){
	
	printf("Threads in Running Queue: %d | ",runningQueue->runningSize);
	node *dummy;
	dummy = runningQueue->front;
	while(dummy){

		if(dummy->threadBlock->status == RUNNING){

			printf("%d -> ", dummy->threadBlock->id);
		}
			dummy = dummy->next;
		
	}
	printf("\n");
}




