TOPDIR=.
include $(TOPDIR)/Generic.mak

DEBUG=true 

ifdef DEBUG
BOOT:=boot/boot.com
else
BOOT:=boot/boot.bin
endif
LOADER:=boot/loader.bin
KERNEL:=kernel/kernel.bin

INITOBJS:=init/main.o

KERNELOBJS:=kernel/core.o kernel/start.o kernel/i8259.o kernel/global.o kernel/traps.o kernel/clock.o \
	kernel/syscall.o kernel/console.o kernel/keyboard.o kernel/sched.o kernel/sys.o kernel/tty.o \
	kernel/printf.o kernel/vsprintf.o kernel/proc.o kernel/panic.o kernel/signal.o kernel/hd.o kernel/fork.o \
	kernel/exit.o kernel/wait.o kernel/sched_fair.o kernel/pid.o

FSOBJS:=fs/bitmap.o fs/super.o fs/inode.o fs/stat.o fs/fcntl.o fs/fs.o fs/namei.o fs/open.o fs/file.o \
	fs/read_write.o  fs/buffer_head.o fs/link.o fs/binfmt_elf.o fs/block.o

MMOBJS:=mm/memory.o mm/swap.o mm/pgtable.o mm/mmap.o mm/kmalloc.o mm/buddy.o mm/slab.o

NETOBJS:=net/net.o

#NETLIB:=net/net.a

LIBOBJS:=lib/string.o lib/kliba.o lib/stringa.o lib/klibc.o lib/errno.o lib/list.o lib/math.o lib/rbtree.o lib/radix-tree.o lib/bitmap.o

#OBJS:=$(KERNELOBJS) $(INITOBJS) $(FSOBJS) $(MMOBJS) $(LIBOBJS) $(NETLIB) 
OBJS:=$(KERNELOBJS) $(INITOBJS) $(FSOBJS) $(MMOBJS) $(LIBOBJS) 

.PHONY:clean subdir
all:everything 

everything:$(BOOT) $(LOADER) $(KERNEL) 

$(BOOT):boot/boot.asm 
	$(ASM) $(ASMINCLUDE) $< -o $@ 
	#@sz -e $(BOOT) 
$(LOADER):boot/loader.asm
	$(ASM) $(ASMINCLUDE) $< -o $@
	#@sz -e $(LOADER)
#$(KERNEL):$(OBJS) subdir
$(KERNEL):$(OBJS) 
	#$(LD) -s -Ttext 0x10400 -o $@ $(OBJS) $(INCLUDE)
	$(LD) -s -Ttext 0x100000 -o $@ $(OBJS) $(INCLUDE)
	#@sz -e $(KERNEL)

init/%.o:init/%.c
	$(CC) -c $(CFLAGS) $<  -o $@ $(INCLUDE)

kernel/%.o:kernel/%.asm
	$(ASM) -f elf $< -o $@ $(INCLUDE)
kernel/%.o:kernel/%.c
	$(CC) -c $(CFLAGS) $<  -o $@ $(INCLUDE)
fs/%.o:fs/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE)
mm/%.o:mm/%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE)
lib/%.o:lib/%.asm
	$(ASM) -f elf $< -o $@ $(INCLUDE)
lib/%.o:lib/%.c
	$(CC) -c $(CFLAGS) $< -o $@  $(INCLUDE)

SUBDIR:=drivers fs init kernel lib mm net
subdir:
	@for dir in $(SUBDIR); do \
		(cd $$dir && $(MAKE)); \
		done

clean:
	rm -f $(BOOT) $(LOADER) $(KERNEL)
	@for dir in $(SUBDIR); do \
		 (cd $$dir && $(MAKE) clean); \
		  done

upload:
	@sz -e $(BOOT) 
	@sz -e $(LOADER)
	@sz -e $(KERNEL)
