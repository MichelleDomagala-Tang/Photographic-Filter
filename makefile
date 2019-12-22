# Filename: makefile.f
# Author: Michelle Domagala-Tang, domagalm, 400182920

CFLAGS=-Wall -O2 -pg -ansi 
filter: filter.o
	$(CC) -o filter $?
clean: 
	@rm -rf fib *.o