CC = gcc 
CFLAGS = -g -pedantic -Wall -Wextra -O2

all: hinfosvc

hinfosvc: hinfosvc.o
		$(CC)$(CFLAGS) hinfosvc.o -o hinfosvc
hinfosvc.o: hinfosvc.c 
		$(CC)$(CFLAGS) -c hinfosvc.c -o hinfosvc.o
 
