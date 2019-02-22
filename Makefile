CC = gcc
CFLAGS = -Wall -g
RM = rm -f

default: all

all: p2

p2: p2.c
	$(CC) $(CFLAGS) -o p2 p2.c

clean veryclean:
	$(RM) p2
