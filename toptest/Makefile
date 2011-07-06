
SHELL		= /bin/sh
CC			= gcc
AR			= ar
RM			= /bin/rm
MV			= mv
DEPEND		= makedepend
LINT		= lint

INSTALL		= /usr/ucb/install

TOP_BIN		= ../../BIN
USR_BIN		= /tmfapp/BIN
TOP_LIB		= ../../LIB
LIBRARY =   -L$(TOP_LIB)


INCLUDES = -L/home/uamyd/WNTAF/LIB -I/home/uamyd/WNTAF/INC
DEFINES  = -DDEBUG -DBL20P -D_GN_WNTAF
#OPTIMIZE = -O
OPTIMIZE = 

ifeq ($(DTAS_DEBUG),)
DEBUG    = -g3 -DDEBUG -DLOGDEBUG
else
DEBUG    = $(DTAS_DEBUG)
endif

M_FLAG		= 
CFLAGS		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) 
SRCS		= 	main.c \
			load.c \
			ntpd.c \
				
TARGET		=	CHSMD

OBJS		= $(SRCS:.c=.o)

LIBS       = -lSHMUTIL -lLogUtil -lInitShm

LOC_LIB		=  
MAKEFILE	= Makefile

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


$(TARGET): $(SRCS) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) -g

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)


install:

clean:
	/bin/rm -f *.o $(TARGET) core*

# DO NOT DELETE THIS LINE -- make depend depends on it.

