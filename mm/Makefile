TOPDIR=..
include $(TOPDIR)/Generic.mak

MMOBJS:=memory.o buddy.o slab.o swap.o pgtable.o mmap.o kmalloc.o page_alloc.o filemap.o

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

