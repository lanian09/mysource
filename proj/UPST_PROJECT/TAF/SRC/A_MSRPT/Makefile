
SHELL	= /bin/sh
CC		= gcc
AR		= ar
RM		= /bin/rm
MV		= mv
DEPEND	= makedepend
LINT	= lint

BIN_DIR = ../../BIN
LIB_DIR = ../../../LIB
OAM_DIR = ../../../OAM

LIBS    = -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER=$(shell uname -r)
KERNEL_DIR =
#KERNEL_DIR=/lib/modules/$(KERNEL_VER)/source/include

INCLUDES=-I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
DEBUG   = -g3 -Wall
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

M_FLAG	= 
CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)
SRCS	= 
SRCS   += msrpt_init.c 
SRCS   += msrpt_main.c 
SRCS   += msrpt_util.c 
SRCS   += msrpt_func.c 
SRCS   += msrpt_msgq.c

TARGET	= A_MSRPT

OBJS	= $(SRCS:.c=.o)

#LIBS    = $(LIBRARY) -lInitShm -lLogUtil -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg -lSTGhasho \
			-lSTGnifo -lSTGmems -lSTGmemg -lSTGtimerN -lSTGhashg -lSTGcommon_stg -lMSRP -lTOOLS -lSHMUTIL
LIBRARY = -lLOGLIB -lVERLIB -lFILELIB -lIPCLIB -lUTILLIB
LIBRARY+= -lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg -lMSRP
LIBRARY+= -lTOOLS

LDFLAGS  = $(LIBS) $(LIBRARY)

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


all: $(TARGET) install

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(CC) -M $(CFLAGS) -c $?
	#$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	/bin/rm -f *.o $(TARGET) $(TARGET2) core* 

# DO NOT DELETE THIS LINE -- make depend depends on it.
