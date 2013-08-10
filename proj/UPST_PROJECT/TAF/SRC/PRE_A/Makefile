
SHELL	= /bin/sh
CC		= gcc
AR		= ar
RM		= /bin/rm
MV		= mv

BIN_DIR = ../../BIN
LIB_DIR = ../../../LIB
OAM_DIR = ../../../OAM

LIBS    = -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER=$(shell uname -r)
KERNEL_DIR =
#KERNEL_DIR+=/lib/modules/$(KERNEL_VER)/source/include

OPTIMIZE=
INCLUDES=-I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
DEBUG	= -g3 -Wall
DEFINES = -DCHK_RADIUS -DBUFFERING

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
SRCS   += prea_init.c 
SRCS   += prea_flt.c 
SRCS   += prea_msgq.c 
SRCS   += prea_frag.c
SRCS   += prea_main.c 

TARGET	= PRE_A 

OBJS	= $(SRCS:.c=.o)

#LIBS    = $(LIBRARY) -lLogUtil -lANZ_LIB -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg \
		  -lSTGhasho -lSTGnifo -lSTGmems -lSTGmemg -lSTGtimerN -lSTGcommon_stg -lSTGhashg \
		  -lUtil -lTOOLS -lInitShm -lIPPOOLBIT -lSHMUTIL
LIBRARY	= -lLOGLIB -lVERLIB -lIPCLIB -lFILELIB -lUTILLIB
LIBRARY+= -lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg
LIBRARY+= -lIPPOOLBIT -lANZ_LIB
LDFLAGS = ${LIBS} ${LIBRARY}

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

install :
	cp $(TARGET) $(BIN_DIR)

depend: $(SRCS)
	${CC} -M ${CFLAGS} < $?

clean:
	/bin/rm -f *.o $(TARGET) core* 

# DO NOT DELETE THIS LINE -- make depend depends on it.
