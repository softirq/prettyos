CC:=gcc
LD:=ld
CFLAGS:= -g -fno-builtin -Wall
INCLUDE:=-I ../include/  -I ../include/linux -I ../include/asm-i386
FSOBJS:=bitmap.o inode.o super.o stat.o fcntl.o namei.o open.o fs.o read_write.o buffer.o file_dev.o link.o
pfs.o:$(FSOBJS) 
	$(LD) -r -o $@ $(FSOBJS)
	sync
%.o:%.c
	$(CC) -c $(CFLAGS) $< $(INCLUDE)
clean:
	rm -f *.o



