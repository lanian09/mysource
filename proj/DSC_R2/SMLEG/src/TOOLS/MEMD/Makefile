include ../../../MkConfig.mk 

SHELL	= /bin/sh
CC		= gcc
AR		= ar
RM		= /bin/rm
MV		= mv
DEPEND	= makedepend
LINT	= lint

TOP_BIN	= ../../../bin
TOP_LIB	= ../../../lib
LIBRARY = -L../../../lib
INCLUDES= -I. -I../../../include -I../../../libsrc/COMM
DFINES =

ifeq ($(BSD_DEBUG),)
DEBUG   = -g3 -Wall
else
DEBUG   = $(BSD_DEBUG)
endif

#DEBUG 	+= -DFUNC_TIME_CHECK

TARGET 	= MEMS

M_FLAG	= 
CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)
#SRCS	= memd_view.c
SRCS	= mems_view.c

OBJS	= $(SRCS:.c=.o)

#LIBS    = -lIPC -lSHMUTIL -lInitShm -lcomm -lLogUtil -lMIF
LIBS    = -lLogUtil -lIPC -lSHMUTIL -lInitShm -lcomm -lnifo

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


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LIBRARY) $(LDFLAGS) 
	$(CP) $(TARGET) $(TOP_BIN)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(TOP_LIB)

doc:
	doxygen

clean:
	/bin/rm -rf *.o $(TARGET) core* TEST_RESULT*.TXT MEMG

# DO NOT DELETE THIS LINE -- make depend depends on it.

