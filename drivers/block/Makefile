TOPDIR=../..
include $(TOPDIR)/Generic.mak

OBJS=ll_rw_blk.o

all:$(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(OBJS) 

