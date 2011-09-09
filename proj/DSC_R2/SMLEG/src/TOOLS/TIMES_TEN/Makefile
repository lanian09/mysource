

#LDFLAGS     = -m64

INSTDIR         = /SM/pcube/lib/tt/TimesTen/pcubesm22

CC              = gcc

LDFLAGS         = -m64

INCS            = -I$(INSTDIR)/include 

CFLAGS          = $(INCS) -m64 -DTPSONLY
#CFLAGS          = $(INCS) 

TTLINK          = -L$(INSTDIR)/lib -R$(INSTDIR)/lib

DCLIBS          = $(TTLINK) -ltten 

PROGS		= pcube_subs 

TOP_BIN		= ../../../bin

#
# Top level targets
#

#all:        $(PROGS)
all: $(PROGS) install

install:
	cp $(PROGS) $(TOP_BIN)

clean:
	rm -f $(PROGS) *.o

pcube_subs:    tt_basic    pcube_subs.o
	$(CC) $(DCLIBS) $(LDFLAGS) -lrt -o pcube_subs pcube_subs.o tt_basic.o

pcube_subs.o:  tt_basic.o pcube_subs.c
	$(CC) $(CFLAGS) -g -o pcube_subs.o -c pcube_subs.c

tt_basic:
	$(CC) $(CFLAGS) -g -o tt_basic.o -c tt_basic.c
