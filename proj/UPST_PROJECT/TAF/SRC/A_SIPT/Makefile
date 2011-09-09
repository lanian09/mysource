
MAKE	=`which make`
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

LIBS    = -L./ -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER=$(shell uname -r)
KERNEL_DIR =
#KERNEL_DIR=/lib/modules/$(KERNEL_VER)/source/include

INCLUDES = -I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
INCLUDES+= -I./SIGCOMP_INC

DEBUG    = -g3 -Wall
DEFINES  =

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
SRCS   += sipt_init.c 
SRCS   += sipt_main.c 
SRCS   += sipt_msgq.c 
SRCS   += sipt_util.c 
SRCS   += sipt_func.c 
SRCS   += sipt_sess.c

TARGET	= A_SIPT

OBJS	= $(SRCS:.c=.o)

#LIBS    = $(LIBRARY) -lInitShm -lLogUtil -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg -lSTGhasho \
				-lSTGnifo -lSTGmems -lSTGmemg -lSTGtimerN -lSTGhashg -lSTGcommon_stg -lUtil \
				-lSIP -lsigcomp -lglib-2.0 -lm -lTOOLS -lSHMUTIL
LIBRARY = -lLOGLIB -lVERLIB -lFILELIB -lIPCLIB -lUTILLIB
LIBRARY+= -lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg
LIBRARY+= -lSIP -lm
LIBRARY+= -lsigcomp -lglib-2.0

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


all: lib $(TARGET) install

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

lib :
	$(MAKE) -C SIGCOMP

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(CC) -M $(CFLAGS) -c $?
	#$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	$(MAKE) -C SIGCOMP $@
	/bin/rm -f *.o *.a $(TARGET) $(TARGET2) core* 

# DO NOT DELETE THIS LINE -- make depend depends on it.
