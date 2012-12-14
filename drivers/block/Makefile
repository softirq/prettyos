CC:=gcc -m32
LD:=ld -m elf_i386

TOPDIR=../..

CFLAGS:= -g -fno-builtin -Wall
INCLUDE:=-I./ -I../ -I$(TOPDIR)/include -I $(TOPDIR)/include/linux -I$(TOPDIR)/block

SOURCES:=$(wildcard *.c)
OBJS:=$(subst .c,.o,$(SOURCES))

all:$(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)
clean:
	rm -f *.o



