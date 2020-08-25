// File:	my_pthread_t.h
// Author:	Yujie REN
// Date:	January 2019

// List all group member's name: Macauley Pinto, Ryan Townsend, Itamar Levi
// username of iLab: msp194, il166, rrt51
// iLab Server: cd.css.rutgers.edu

#ifndef MY_PTHREAD_T_H
#define MY_PTHREAD_T_H

#define _GNU_SOURCE
#define stackSize 1024*8
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define COMPLETE 3
#define INTERVAL 100
/* To use real pthread Library in Benchmark, you have to comment the USE_MY_PTHREAD macro */
#define USE_MY_PTHREAD 1
#define STCF 10

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sched.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <setjmp.h>
void outputQueue();
void outputPriority();
typedef uint my_pthread_t;

typedef struct threadControlBlock {
	/* add important states in a thread control block */
	// thread Id
	// thread status
	// thread context
	// thread stack
	// thread priority
	// And more ...

	// YOUR CODE HERE
	my_pthread_t id;
	int status;

	ucontext_t *context;

	int elapsed;
	int join;
	int priority;
	
	void *(*func)(void *);
	void *args;
	void *returnValue;
	
	
	
	
	
} tcb; 
struct itimerval timer;
/* mutex struct definition */
typedef struct my_pthread_mutex_t {
	/* add something here */
	// YOUR CODE HERE
	int id;
	volatile int flag;
	struct tcbNode *holder;
	struct tcbNode* blockedList;
	struct tcbNode* front;
	
} my_pthread_mutex_t;


/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

// YOUR CODE HERE


typedef struct mutexNode{
	my_pthread_mutex_t *mutex;
	struct mutexNode *next;


}mutexNode;

typedef struct tcbNode{ 
    tcb *threadBlock; 
    int queueLevel;
    struct tcbNode *next;
}node;

struct blockedThreadQueue{

	node *front;
	node *rear;
	int sizeOfQueue;

};

struct runningThreadQueue{

	node* front;
	node* rear;
	int readySize;
	int runningSize;


};



struct tcbNode *completedThreads;
struct blockedThreadQueue *blockedQueue;
struct runningThreadQueue *runningQueue;

/* Function Declarations: */
void initQueues();

int isEmptyState(int state);

void enQueueState(node *block,int state, int sched, int level);

node *deQueueState(int state);

node *removeFromQueueState(my_pthread_t threadID, int state);

node *deleteFromList();

void addToFront(tcb *data);

void appendMutex(mutexNode * head,mutexNode * mutex);

void addToBlockedList(my_pthread_mutex_t* mutex, node* thread);

node* removeBlockedList(my_pthread_mutex_t *mutex);

node *removeFromMLFQ(my_pthread_t threadID,struct runningThreadQueue **A);

void enqueueMLFQ(struct runningThreadQueue* A, node* block,int level);

node* dequeueMLFQ(struct runningThreadQueue **A);

void shiftThreadsInQueue(struct runningThreadQueue **A);

void printMLFQ(struct runningThreadQueue **A);

node *findByThreadID(my_pthread_t threadID,int sched);

node *checkIfInCompletedQueue(my_pthread_t threadID);

void outputCompletedThreads();

void setToComplete(my_pthread_t threadID);

node *removeCompletedThread(my_pthread_t threadID, int state);

node *deQueueByPriority();

node  *removeCreatedNode();

void threadWrapper();

void *beginScheduling();

void initializeTimerInterrupt();

void timerInterruptHandler();

static void schedule(); 

static void sched_stcf();

static void sched_mlfq();

int cleanup(tcb thread);

/* create a new thread */
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);
	
/* give CPU pocession to other user level threads voluntarily */
int my_pthread_yield();

/* terminate a thread */
void my_pthread_exit(void *value_ptr);

/* wait for thread termination */
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/* initial the mutex lock */
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

/* aquire the mutex lock */
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/* release the mutex lock */
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/* destroy the mutex */
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#ifdef USE_MY_PTHREAD
#define pthread_t my_pthread_t
#define pthread_mutex_t my_pthread_mutex_t
#define pthread_create my_pthread_create
#define pthread_exit my_pthread_exit
#define pthread_join my_pthread_join
#define pthread_mutex_init my_pthread_mutex_init
#define pthread_mutex_lock my_pthread_mutex_lock
#define pthread_mutex_unlock my_pthread_mutex_unlock
#define pthread_mutex_destroy my_pthread_mutex_destroy
#endif

#endif
