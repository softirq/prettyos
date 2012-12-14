ASM:=nasm
CC:=gcc -m32
LD:=ld -m elf_i386
CFLAGS:= -g -fno-builtin -Wall

INCLUDE:=-I ../include/ -I ../include/linux

OBJS:=main.o 
init.o:$(OBJS)
	$(LD) -r -o $@ $(OBJS)
%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f *.o

