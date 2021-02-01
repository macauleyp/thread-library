# User-Level Thread Library in C

This is a pure User-Level Thread library that has an interface similar to the standard POSIX Thread library. The code was compiled and executed successfully on CentOS 7.

# Running the Library

The User-Level Thread library allows for scheduling threads between two different policies—Preemptive Shortest Job First and Multi-Level Feedback Queue. To change scheduling policies for the User-Level Thread library execute the following commands in the libraries directory:

 - Preemptive Shortest Job First: this is already set by default. Simply execute ```make``` from the command-line.
 - Multi-Level Feedback Queue: Execute ```make SCHED=MLFQ``` from the command-line.
 **Note:** Be sure to run ```make clean``` from the command-line prior to switching scheduling policies.

The benchmark folder allows for comparison between the User-Level and POSIX Thread library between three different programs. 
 - **Note:** To use the User-Level Thread library instead of the POSIX Thread library, comment out the MACRO ```#define USE_MY_PTHREAD 1 ``` in the file my_pthread_t.h and recompile the thread library and benchmark from their respective Makefile 
	 - More specifically, execute ```make clean``` and then ```make``` in both the Thread library and benchmark directory.
- The programs parrallelCal and vectorMultiply are CPU-bound while externalCal is IO-bound.

# Thread Library API
The library features both thread and mutex lock related functions as defined below.  

 ## Thread Specific Functions
 
Creates a thread that executes the provided function.
```C
int my_pthread_create(my_pthread_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg);
```
Ends the thread that called it—if the ```value_ptr``` is not ```NULL```, any return value from the thread will be saved.
```C
void my_pthread_exit(void *value_ptr);
```
Ensures the calling thread will not continue execution until the one it references exits. If the ```value_ptr``` is not ```NULL```, the return value of the exiting thread will be passed back.
```C
int my_pthread_join(my_pthread_t thread, void **value_ptr);
```
## Mutex Specific Functions
Initializes a my_pthread_mutex_t (a mutex lock) created by the calling thread. 

 - **Note:** 'mutexattr' is ignored

```C
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);
```
Locks a given mutex such that other threads attempting to access the mutex will not run until it is unlocked.
```C
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);
```
Unlocks a given mutex. 
```C
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
```
Destroys a given mutex.

 - **Note:** The mutex should be unlocked before performing this operation. 

```C
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);
```


