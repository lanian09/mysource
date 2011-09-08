#include ../../MkConfig.mk

CC			= gcc
TOP_LIB		= ../../../lib
TOP_BIN		= ../../../bin
LIBRARY 	= -L../../../libsrc/SM_API/solaris -L$(TOP_LIB)

INCLUDES	= -L ./ -L../../lib -I../../../include -I../../../libsrc/SM_API/include -I../../../libsrc/SM_API/include/system
#DEFINES		= -DPRINT -DSMAPI -D_LOGOUT_
DEFINES		= -DPRINT -DSMAPI -PROPERTY
#DEFINES		= -DPRINT -DSMAPI

CFLAGS		= -g $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
#SRCS		= leg_sce_comm.c 
SRCS		= leg_sce_thread.c 
TARGET		= SCESIM

OBJS		= $(SRCS:.c=.o)

LIBS		= $(LIBRARY) -lIPC -lLogUtil -lcomm -lsmapi -lpthread -lsocket -lInitShm

MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#

#

all: $(TARGET) 

$(SRCS):
#	$(GET) -s $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LIBRARY) $(LDFLAGS)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(TOP_BIN)


clean:
	rm -f *.o $(TARGET) core*
#	sccs clean

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
