CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib

SCHED = PSJF

all: my_pthread.a

my_pthread.a: my_pthread.o
	$(AR) libmy_pthread.a my_pthread.o
	$(RANLIB) libmy_pthread.a

my_pthread.o: my_pthread_t.h

ifeq ($(SCHED), PSJF)
	$(CC) -pthread $(CFLAGS) my_pthread.c
else ifeq ($(SCHED), MLFQ)
	$(CC) -pthread $(CFLAGS) -DMLFQ my_pthread.c
else
	echo "no such scheduling algorithm"
endif

clean:
	rm -rf testfile *.o *.a
