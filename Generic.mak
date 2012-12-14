ASM:=nasm
CC:=gcc -m32
LD:=ld -m elf_i386
#AR:=ar -X32 
AR:=ar 
MAKE:=make

CFLAGS=-g -fno-builtin -Wall -ggdb
INCLUDE=-I$(TOPDIR)/include/ -I$(TOPDIR)/include/linux/ -I$(TOPDIR)/include/asm-i386/
ASMINCLUDE=-I $(TOPDIR)/boot/include/
ARFLAGS=-r

#SOURCES=$(wildcard *.c)
#OBJS=$(subst .c,.o,$(SOURCES))
