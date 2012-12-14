DEBUG=true 
ASM:=nasm
CC:=gcc -m32
LD:=ld -m elf_i386
CFLAGS:= -g -fno-builtin -Wall -ggdb
INCLUDE:=-I include/ -I include/linux/ -I include/asm-i386/
ASMINCLUDE:= -I boot/include/

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
	kernel/exit.o kernel/bitops.o kernel/wait.o

FSOBJS:=fs/bitmap.o fs/super.o fs/inode.o fs/stat.o fs/fcntl.o fs/fs.o fs/namei.o fs/open.o fs/file_dev.o \
	fs/read_write.o  fs/buffer.o fs/link.o

MMOBJS:=mm/memory.o mm/swap.o mm/pgtable.o mm/mmap.o mm/kmalloc.o

NETOBJS:=net/net.o

#NETLIB:=net/net.a

LIBOBJS:=lib/misc.o lib/kliba.o lib/string.o lib/klibc.o lib/errno.o

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
	$(LD) -s -Ttext 0x10400 -o $@ $(OBJS) $(INCLUDE)
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

subdir:
#	cd net;$(MAKE)
#net/%.o:net/%.c
#	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE)

upload:
	@sz -e $(BOOT) 
	@sz -e $(LOADER)
	@sz -e $(KERNEL)

clean:
	cd init;rm -f *.o
	cd kernel;rm -f *.o
	cd fs;rm -f *.o
	cd mm; rm -f *.o
	cd net;rm -f *.o
	cd lib;rm -f *.o
	cd drivers/net; rm -f *.o;
	cd drivers/block; rm -f *.o;
	rm -f $(BOOT) $(LOADER) $(KERNEL)
