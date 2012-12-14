ASM:=nasm
CC:=gcc
LD:=ld
CFLAGS:= -g -fno-builtin -Wall
INCLUDE:=-I ../include/ -I ../include/linux
OBJS:=main.o 
init.o:$(OBJS)
	$(LD) -r -o $@ $(OBJS)
%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f *.o

