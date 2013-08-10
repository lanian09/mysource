
SHELL	= /bin/sh
CC		= gcc
AR		= ar
RM		= /bin/rm
MV		= mv
DEPEND	= makedepend
LINT	= lint

TOP_BIN	= ../../BIN
TAF_BIN	= /AQUA/BIN
TOP_LIB	= ../../LIB
LIBRARY =   -L$(TOP_LIB)

KERNEL_VER=$(shell uname -r)
KERNEL_DIR=/lib/modules/$(KERNEL_VER)/source/include

INCLUDES=-I$(KERNEL_DIR) -I../../INC -I.
DEFINES = -DCHK_RADIUS
ifeq ($(AQUA_DEBUG),)
DEBUG= -g3 -Wall
else
DEBUG   = $(AQUA_DEBUG)
endif

M_FLAG	= 
CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)
SRCS	= prea_chk.c

TARGET	= PREA_CHK 

OBJS	= $(SRCS:.c=.o)

LIBS    = $(LIBRARY) -lLogUtil -lANZ_LIB -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg \
		  -lSTGhasho -lSTGnifo -lSTGmems -lSTGmemg -lSTGtimerN -lSTGcommon_stg -lSTGhashg \
		  -lUtil -lTOOLS -lIPPOOLBIT -lSHMUTIL -lInitShm

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


all: $(TARGET) $(TARGET2)

#$(SRCS):
#	$(GET) -s $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

$(TARGET2): $(OBJS2)
	$(CC) $(CFLAGS) -o $@ $(OBJS2) $(LIBS) $(LIBRARY) $(LDFLAGS)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(TAF_BIN)

clean:
	/bin/rm -f *.o $(TARGET) $(TARGET2) core* 

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
