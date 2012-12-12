TOPDIR=../..

include $(TOPDIR)/Generic.mak

all:$(OBJS) 

%.o:%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< 

clean:
	rm -f $(OBJS) 

