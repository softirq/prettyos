TOPDIR=..
include $(TOPDIR)/Generic.mak

FSOBJS:=bitmap.o inode.o super.o stat.o fcntl.o namei.o open.o fs.o read_write.o buffer.o file_dev.o link.o

PFS=pfs.o

all:$(FSOBJS)

#$(PFS):$(FSOBJS) 
	#$(LD) -r -o $@ $(FSOBJS)
	#sync

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(PFS)
	rm -f $(FSOBJS) 
