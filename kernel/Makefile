ASM:=nasm
CC:=gcc
LD:=ld
CFLAGS:= -g -fno-builtin -Wall
INCLUDE:=-I ../include/ -I ../include/linux -I ../include/asm-i386
OBJS:=core.o start.o i8259.o global.o traps.o hd.o \
	clock.o syscall.o console.o keyboard.o sched.o sys.o tty.o printf.o\
	vsprintf.o proc.o panic.o signal.o  fork.o exit.o timer.o bitops.o \
	wait.o
kernel.o:$(OBJS)
	$(LD) -r -o $@ $(OBJS)
%.o:%.asm
	$(ASM) -f elf $< -o $@ $(INCLUDE)
%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f *.o

