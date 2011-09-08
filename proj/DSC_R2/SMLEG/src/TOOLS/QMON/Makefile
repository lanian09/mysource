#include ../../MkConfig.mk

CC			= gcc
TOP_LIB		= ../../../lib
TOP_BIN		= ../../../bin
LIBRARY 	= -L../../../libsrc/SM_API/sm-c-api-3.0.6_B35/solaris -L$(TOP_LIB)

INCLUDES	= -L../../../lib -I../../../include -I./ -I../../../libsrc/SM_API/include -I../../../libsrc/SM_API/include/system -I../../../libsrc/COMM
#DEFINES		= -DPRINT -DSMAPI -DPERF_TEST
DEFINES		= -DPRINT -DSMAPI -D_REENTRANT -DDEBUG

CFLAGS		= -g $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
SRCS		= qmon.c
TARGET		= QMON

OBJS		= $(SRCS:.c=.o)

LIBS		= $(LIBRARY) -lIPC -lLogUtil -lcomm -lsmapi -lpthread -lsocket -lInitShm
#LIBS		= $(LIBRARY) -lIPC -lpthread -lsocket

MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#

#

all: $(TARGET) 

$(TARGET): $(OBJS)
	$(CC) -mt  $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LIBRARY) $(LDFLAGS)
	cp $(TARGET) $(TOP_BIN)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
#	cp $(TARGET) $(TOP_BIN)


clean:
	rm -f *.o $(TARGET) core*
#	sccs clean

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
