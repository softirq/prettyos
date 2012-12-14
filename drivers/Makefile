TOPDIR:=..
include $(TOPDIR)/Generic.mak

SUBDIR:=block net 

subdir:
	@for dir in $(SUBDIR); do \
		(cd $$dir && $(MAKE)); \
		done

clean:
	rm -f *.o
	@for dir in $(SUBDIR); do \
		 (cd $$dir && $(MAKE) clean); \
		  done

