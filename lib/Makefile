TOPDIR=..
include $(TOPDIR)/Generic.mak

LIBOBJS:=klibc.o string.o kliba.o misc.o errno.o

all:$(LIBOBJS)
#	$(LD) -r -o $@ $(OBJS)
#	sync

%.o:%.asm
	$(ASM) -f elf $< -o $@ $(INCLUDE)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(LIBOBJS) 
