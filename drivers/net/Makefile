TOPDIR=../..
include $(TOPDIR)/Generic.mak

INCLUDE+=-I$(TOPDIR)/net/

OBJS:=ne.o space.o net_init.o loopback.o

all:$(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(OBJS)

