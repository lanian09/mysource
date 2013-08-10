
SHELL		=/bin/sh
CC			=`which gcc`
AR			=`which ar`
RM			=`which rm`
MV			=`which mv`
DEPEND		=makedepend
LINT		=lint

TOP_BIN		=../../BIN
TOP_LIB		=../../LIB
LIBRARY 	=-L$(TOP_LIB) -L../../../LIB/LIB

KERNEL_VER	=$(shell uname -r)
KERNEL_DIR	=/lib/modules/$(KERNEL_VER)/source/include

INCLUDES	=-I$(KERNEL_DIR) -I../../INC -I. -I../../../INC  -I../../../LIB/INC -I../../../OAM/INC
DEFINES 	=
ifeq ($(AQUA_DEBUG),)
DEBUG   	=-g3 -Wall
else
DEBUG   	=$(AQUA_DEBUG)
endif

ifeq ($(SYSTYPE), RP)
DEFINES    +=-DRP
endif
ifeq ($(SYSTYPE), PI)
DEFINES    +=-DPI
endif
ifeq ($(SYSTYPE), PIOLD)
DEFINES    +=-DPIOLD
endif

M_FLAG		= 
CFLAGS		=$(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)
SRCS		=im_init.c im_main.c im_msgq.c
TARGET		=A_IM

OBJS		=$(SRCS:.c=.o)

#LIBS    	=$(LIBRARY) -lInitShm -lLogUtil -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg -lSTGhasho -lSTGnifo -lSTGmems -lSTGmemg -lSTGtimerN -lSTGcommon_stg -lSTGhashg -lUtil -lPAGE -lTOOLS -lSHMUTIL
LIBS		=$(LIBRARY)
LIBS	   +=-lLOGLIB -lVERLIB -lFILELIB  -lIPCLIB
LIBS	   +=-lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg ### -lPKT 
#LIBS	   +=-lTOOLS	#dGetPlatformType

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
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)
	cp $(TARGET) $(TOP_BIN)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(TAF_BIN)

clean:
	/bin/rm -f *.o $(TARGET) $(TARGET2) core* 

# DO NOT DELETE THIS LINE -- make depend depends on it.
