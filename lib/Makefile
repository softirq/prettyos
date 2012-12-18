TOPDIR=..
include $(TOPDIR)/Generic.mak

LIBOBJS:=klibc.o stringa.o kliba.o string.o errno.o list.o

all:$(LIBOBJS)
#	$(LD) -r -o $@ $(OBJS)
#	sync

%.o:%.asm
	$(ASM) -f elf $< -o $@ $(INCLUDE)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(LIBOBJS) 
