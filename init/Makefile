TOPDIR=..
include $(TOPDIR)/Generic.mak

OBJS:=main.o 

init.o:$(OBJS)
	$(LD) -r -o $@ $(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(OBJS) 
