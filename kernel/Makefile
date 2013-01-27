TOPDIR=..
include $(TOPDIR)/Generic.mak

OBJS:=core.o start.o i8259.o global.o traps.o hd.o \
	clock.o syscall.o console.o keyboard.o sched.o sys.o tty.o \
	proc.o panic.o signal.o  fork.o exit.o timer.o wait.o sched_fair.o \
	pid.o


KERNEL=kernel.o

$(KERNEL):$(OBJS)
	$(LD) -r -o $@ $(OBJS)

%.o:%.asm
	$(ASM) -f elf $< -o $@ $(INCLUDE)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(KERNEL) 
	rm -f $(OBJS) 

