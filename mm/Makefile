TOPDIR=..
include $(TOPDIR)/Generic.mak

MMOBJS:=memory.o pgtable.o swap.o mmap.o

all:$(MMOBJS)

#mm.o:$(FSOBJS) 
	#$(LD) -r -o $@ $(FSOBJS)
	#sync

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(MMOBJS) 

