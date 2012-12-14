TOPDIR=../..
include $(TOPDIR)/Generic.mak

all:$(OBJS)

CFLAGS+=-I./
%.o:%.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< 
clean:
	rm -f $(OBJS) 
