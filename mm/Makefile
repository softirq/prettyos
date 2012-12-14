CC:=gcc
LD:=ld
CFLAGS:= -g -fno-builtin -Wall
INCLUDE:=-I ../include/ -I ../include/linux/ -I ../include/asm-i386
FSOBJS:=memory.o pgtable.o swap.o mmap.o
mm.o:$(FSOBJS) 
	$(LD) -r -o $@ $(FSOBJS)
	sync
%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)
clean:
	rm -f *.o



