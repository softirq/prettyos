TOPDIR=../..
include $(TOPDIR)/Generic.mak

OBJS=sock.o

all:$(OBJS) 

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(OBJS) 

