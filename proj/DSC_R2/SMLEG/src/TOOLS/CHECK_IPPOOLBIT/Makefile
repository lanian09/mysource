
SHELL   = /bin/sh
CC      = gcc
AR      = ar
RM      = /bin/rm
MV      = mv
DEPEND  = makedepend
LINT    = lint

TOP_BIN = ../../../bin

INCLUDES= -L../../../lib -I../../../include -I./
DEFINES =
#OPTIMIZE= -O
OPTIMIZE= -O -g3 -Wall
ifeq ($(DTAS_DEBUG),)
DEBUG   = -g3 -DDEBUG -Wall
else
DEBUG   = $(DTAS_DEBUG)
endif

M_FLAG  = 
CFLAGS  = $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)
SRCS    = check_main.c


TARGET  = CHECK_IPPOOLBIT

OBJS    = $(SRCS:.c=.o)
LIBS	= -lANZ_LIB -lLogUtil -lIPC -lInitShm -lSHMUTIL -lcomm -lIPPOOLBIT -lsocket  -lnsl

MAKEFILE= Makefile

#
#-----------------------------------------------------------
#

#

.SUFFIXES: .o
.c.o:
	$(CC) $(CFLAGS) -c $<


all: $(TARGET)


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBRARY) $(LIBS)
#	cp $(TARGET) $(TOP_BIN)

clean:
	/bin/rm -f *.o $(TARGET) core*



# DO NOT DELETE THIS LINE -- make depend depends on it.

