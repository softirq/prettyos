TOPDIR=..
include $(TOPDIR)/Generic.mak

OBJS:=main.o 
INIT=init.o

$(INIT):$(OBJS)
	$(LD) -r -o $@ $(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(INIT) 
	rm -f $(OBJS) 
