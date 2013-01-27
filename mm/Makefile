TOPDIR=..
include $(TOPDIR)/Generic.mak

MMOBJS:=pgtable.o memory.o kmalloc.o swap.o mmap.o buddy.o slab.o filemap.o

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

