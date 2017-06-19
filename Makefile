CFLAGS	= -Wall -Werror -Wno-unused-function -O2
DFLAGS	= -Wall -Werror -Wno-unused-function -g -DQDMD_DEBUG
YFLAGS	= -d
DYFLAGS	= -d -v
LFLAGS	= --yylineno

OFILES	= qdmd.o qdmd.g.o qdmd.lx.o safe.o
qdmd:	qdmd.h y.tab.h	$(OFILES)
	$(CC) $(CFLAGS) -o qdmd $(OFILES) -lfl -ly

qdmd.g.o:	safe.h qdmd.h qdmd.g.y
qdmd.lx.o:	safe.h qdmd.h y.tab.h qdmd.lx.l

y.tab.h:	qdmd.g.o
safe.o:	safe.h safe.c
qdmd.o:	qdmd.h qdmd.c

install:	qdmd
	mv qdmd $(DESTDIR)/usr/bin/qdmd

uninstall:
	rm $(DESTDIR)/usr/bin/qdmd

debug:	clean
	$(MAKE) CFLAGS="$(DFLAGS)" YFLAGS="$(DYFLAGS)"

clean:
	-rm qdmd $(OFILES) y.tab.h y.output
