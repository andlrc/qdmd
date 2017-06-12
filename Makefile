CFLAGS	= -Wall -Werror -Wno-unused-function -O2
DFLAGS	= -Wall -Werror -Wno-unused-function -g -DQDMD_DEBUG
YFLAGS	= -d
DYFLAGS	= -d -v

OFILES	= qdmd.o qdmd.g.o qdmd.lx.o
qdmd:	qdmd.h y.tab.h	$(OFILES)
	$(CC) $(CFLAGS) -o qdmd $(OFILES) -lfl -ly

qdmd.g.o:	qdmd.h qdmd.g.y
qdmd.lx.o:	qdmd.h y.tab.h qdmd.lx.l

y.tab.h:	qdmd.g.o
qdmd.o:	qdmd.c

debug:	clean
	$(MAKE) CFLAGS="$(DFLAGS)" YFLAGS="$(DYFLAGS)"

clean:
	-rm qdmd $(OFILES) y.tab.h y.output
