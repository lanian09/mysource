
MAKE	=`which make`
CC		= gcc

BIN_DIR	= ../../BIN
LIB_DIR = ../../../LIB
OAM_DIR = ../../../OAM

LIBS    = -L./ -L../../LIB -L$(LIB_DIR)/LIB


#KERNEL_VER=$(shell uname -r)
KERNEL_DIR =
#KERNEL_DIR+=/lib/modules/$(KERNEL_VER)/source/include

INCLUDES=-I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
DEFINES =
DEBUG   = -g3 -Wall

ifeq ($(SYSTYPE), RP)
DEFINES += -DRP
endif
ifeq ($(SYSTYPE), PI)
DEFINES += -DPI
endif
ifeq ($(SYSTYPE), PIOLD)
DEFINES += -DPIOLD
endif

CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)

SRCS	=
SRCS   += sctp_func.c
SRCS   += sctp_serv.c
SRCS   += sctp_init.c
SRCS   += sctp_main.c

TARGET	= A_SCTP

OBJS	= $(SRCS:.c=.o)

#LIBRARY =-lnifo -lmems -lSTGnifo -lSTGmems -lSHMUTIL -lLogUtil -lASSOSTACK -lInitShm -lTOOLS
LIBRARY = -lLOGLIB -lVERLIB -lUTILLIB
LIBRARY+= -lnifo -lgifo -lcifo
LIBRARY+= -lASSOSTACK
LDFLAGS = $(LIBS) $(LIBRARY)

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


all: lib $(TARGET) install

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

lib :
	$(MAKE) -C SCTPSTACK_LIB

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	$(MAKE) -C SCTPSTACK_LIB $@
	/bin/rm -f *.o *.a $(TARGET) core* 

# DO NOT DELETE THIS LINE -- make depend depends on it.
