ASM:=nasm
CC:=gcc -m32
LD:=ld -m elf_i386
#AR:=ar -X32 
AR:=ar 
MAKE:=make

CFLAGS= -g -fno-builtin -Wall -ggdb
CPPFLAGS=-I$(TOPDIR)/include/ -I$(TOPDIR)/include/linux
ASMINCLUDE=-I $(TOPDIR)/boot/include/
ARFLAGS=-r

SOURCES=$(wildcard *.c)
OBJS=$(subst .c,.o,$(SOURCES))
