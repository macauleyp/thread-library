CC = gcc
CFLAGS = -g -w

all:: parallelCal vectorMultiply externalCal

parallelCal: 
	$(CC) $(CFLAGS) -pthread -o parallelCal parallelCal.c -L../ -lmy_pthread

vectorMultiply: 
	$(CC) $(CFLAGS) -pthread -o vectorMultiply vectorMultiply.c -L../ -lmy_pthread

externalCal: 
	$(CC) $(CFLAGS) -pthread -o externalCal externalCal.c -L../ -lmy_pthread

clean:
	rm -rf testcase parallelCal vectorMultiply externalCal *.o ./record/
