TOPDIR=../..
include $(TOPDIR)/Generic.mak

#OBJS=af_inet.o devinet.o protocol.o raw.o route.o skbuff.o arp.o dev.o ip.o sock.o tcp.o udp.o
OBJS=af_inet.o devinet.o protocol.o raw.o route.o skbuff.o ip.o sock.o tcp.o udp.o

all:$(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(OBJS) 
