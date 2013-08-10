
SHELL	= /bin/sh
CC		= gcc
CP  	= /usr/bin/cp -f
AR		= ar
RM		= /bin/rm
MV		= mv
DEPEND	= makedepend
LINT	= lint

TOP_BIN	= ../../BIN
TAF_BIN	= /AQUA/BIN
TOP_LIB	= ../../../lib

LIBRARY = -L../lib
INCLUDES= -I../inc
DEFINES =

ifeq ($(AQUA_DEBUG),)
DEBUG   = -g3 -Wall
else
DEBUG   = $(AQUA_DEBUG)
endif

TARGET	= memg
TARGETLIB	= libmemg.a

M_FLAG	= 
CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)
SRCS	= memg.c

OBJS	= $(SRCS:.c=.o)

LIBS    = $(LIBRARY) -lSTGmemg

MAKEFILE= Makefile

#
#-----------------------------------------------------------
#

#
.SUFFIXES: .c .s .o .i .u
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(SFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<


all: $(TARGETLIB)

#$(SRCS):
#	$(GET) -s $@

$(TARGETLIB) : $(OBJS)
	$(RM) -f $@
	$(AR) clq $@ $(OBJS)
	$(AR) ts $@
	${CP} $@ $(TOP_LIB)
	$(RM) *.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
#	cp $(TARGET) $(TAF_BIN)

doc:
	doxygen

clean:
	$(RM) -f *.s *.u *.o *.i core $(TARGETLIB)

# DO NOT DELETE THIS LINE -- make depend depends on it.
