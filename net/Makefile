TOPDIR:=..
include $(TOPDIR)/Generic.mak

LIBNAME=net.a

SUBDIR:=inet unix $(TOPDIR)/drivers/net
INETOBJS:=inet/*.o
UNIXOBJS:=unix/*.o
DRIVEROBJS:=$(TOPDIR)/drivers/net/*.o

CPPFLAGS+=-I./inet
.PHONY:subdir

$(LIBNAME):$(OBJS) subdir 
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJS) $(INETOBJS) $(UNIXOBJS) $(DRIVEROBJS)
%.o:%.c
	$(CC) -c $(CFLAGS) $< 

subdir:
	@for dir in $(SUBDIR); do \
		(cd $$dir && $(MAKE)); \
		done

clean:
	rm -f *.o
	@for dir in $(SUBDIR); do \
		 (cd $$dir && $(MAKE) clean); \
		  done

