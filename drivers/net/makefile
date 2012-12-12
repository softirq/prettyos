CC:=gcc
LD:=ld

TOPDIR=../..

CFLAGS:= -g -fno-builtin -Wall
INCLUDE:=-I./ -I../ -I$(TOPDIR)/include -I $(TOPDIR)/include/linux -I$(TOPDIR)/net

SOURCES:=$(wildcard *.c)
OBJS:=$(subst .c,.o,$(SOURCES))

all:$(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)
clean:
	rm -f *.o



