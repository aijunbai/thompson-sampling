PROBLEMS=	ctp3 sailing race random-mdps puzzle rect tree wet taxi
.PHONY: FORCE_MAKE

all:
	for p in $(PROBLEMS); do \
	    (cd $$p; make) \
	done

clean: FORCE_MAKE
	for p in $(PROBLEMS); do \
	    (cd $$p; make clean) \
	done
	(cd engine; make clean)
	rm -f *~ core
