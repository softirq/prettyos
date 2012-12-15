TOPDIR=..
include $(TOPDIR)/Generic.mak

MMOBJS:=kmalloc.o memory.o pgtable.o swap.o mmap.o

MM=mm.o
all:$(MMOBJS)

#$(MM):$(FSOBJS) 
	#$(LD) -r -o $@ $(FSOBJS)
	#sync

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(MM)
	rm -f $(MMOBJS) 

