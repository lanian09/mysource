
CC		= gcc

BIN_DIR = ../../BIN
LIB_DIR = ../../../LIB
OAM_DIR = ../../../OAM

LIBS    = -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER=$(shell uname -r)
KERNEL_DIR =
#KERNEL_DIR+=/lib/modules/$(KERNEL_VER)/source/include

OPTIMIZE=
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

CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)

#LIBRARY= -lInitShm -lLogUtil -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg -lSTGhasho -lSTGnifo \
#		  -lSTGmems -lSTGmemg -lSTGtimerN -lSTGcommon_stg -lSTGhashg -lUtil -lPAGE -lTOOLS -lSHMUTIL
LIBRARY = -lLOGLIB -lnifo -lgifo -lcifo -lhasho -ltimerN -lPAGE -lSTGcommon_stg -lVERLIB -lFILELIB -lIPCLIB
LDFLAGS = $(LIBS) $(LIBRARY)

SRCS	=
SRCS   += call_init.c call_msgq.c call_func.c call_utils.c call_main.c
TARGET	= A_CALL

OBJS	= $(SRCS:.c=.o)

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

#$(SRCS):
#	$(GET) -s $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)
	cp $(TARGET) $(BIN_DIR)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	/bin/rm -f *.o $(TARGET) core* 

# DO NOT DELETE THIS LINE -- make depend depends on it.
