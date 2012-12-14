ASM:=nasm
CC:=gcc -m32
LD:=ld -m elf_i386
CFLAGS:= -g -fno-builtin
INCLUDE:=-I ../include/ -I ../include/linux
OBJS:=klibc.o string.o kliba.o misc.o errno.o
all:$(OBJS)
#	$(LD) -r -o $@ $(OBJS)
#	sync
%.o:%.asm
	$(ASM) -f elf $< -o $@ $(INCLUDE)
%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f *.o
