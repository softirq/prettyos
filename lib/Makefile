TOPDIR=..
include $(TOPDIR)/Generic.mak

LIBOBJS:=klibc.o stringa.o kliba.o string.o errno.o list.o math.o rbtree.o bitmap.o radix-tree.o

all:$(LIBOBJS)
#	$(LD) -r -o $@ $(OBJS)
#	sync

%.o:%.asm
	$(ASM) -f elf $< -o $@ $(INCLUDE)

%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

clean:
	rm -f $(LIBOBJS) 
