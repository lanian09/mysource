
CC		= gcc

BIN_DIR	= ../../BIN
LIB_DIR	= ../../../LIB
OAM_DIR = ../../../OAM

LIBS	= -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER=$(shell uname -r)
KERNEL_DIR =
#KERNEL_DIR=/lib/modules/$(KERNEL_VER)/source/include

INCLUDES=-I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
DEFINES =

ifeq ($(SYSTYPE), RP)
DEFINES += -DRP
endif
ifeq ($(SYSTYPE), PI)
DEFINES += -DPI
endif
ifeq ($(SYSTYPE), PIOLD)
DEFINES += -DPIOLD
endif

DEBUG   = -g3 -Wall

M_FLAG	= 
CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)

#LIBRARY = -lInitShm -lLogUtil -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg -lSTGhasho -lSTGnifo -lSTGmems -lSTGmemg -lSTGtimerN -lSTGcommon_stg -lSTGhashg -lUtil -lPKT -lTOOLS -lSHMUTIL
LIBRARY = -lLOGLIB -lVERLIB -lFILELIB  -lIPCLIB -lUTILLIB
LIBRARY+= -lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg ### -lPKT 
LIBRARY+= -lTOOLS	#dGetPlatformType
LDFLAGS  = $(LIBS) $(LIBRARY)

SRCS	=
SRCS   += tcp_util.c
SRCS   += tcp_msgq.c
SRCS   += tcp_func.c
SRCS   += tcp_sess.c
SRCS   += tcp_init.c
SRCS   += tcp_main.c

TARGET	= A_TCP

OBJS	= $(SRCS:.c=.o)

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


all: $(TARGET) install

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	/bin/rm -f *.o $(TARGET) $(TARGET2) core* 

doc:
	doxygen

# DO NOT DELETE THIS LINE -- make depend depends on it.
