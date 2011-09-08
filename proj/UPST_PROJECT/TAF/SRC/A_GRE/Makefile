
SHELL	= /bin/sh
MAKE	= `which make`
CC		= gcc
AR		= ar
RM		= /bin/rm
MV		= mv
DEPEND	= makedepend
LINT	= lint

BIN_DIR = ../../BIN
LIB_DIR = ../../../LIB
OAM_DIR = ../../../OAM

LIBS    = -L. -L../../LIB -L$(LIB_DIR)/LIB

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

M_FLAG	= 
CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)
SRCS	= 
SRCS   += agre_init.c 
SRCS   += agre_main.c 
SRCS   += agre_serv.c 
SRCS   += agre_func.c

TARGET	= A_GRE

OBJS	= $(SRCS:.c=.o)

#LIBS    = $(LIBRARY) -lLogUtil -lnifo -lmems -lhasho -lmemg -ltimerN -lhashg -lSTGhasho -lSTGnifo -lSTGmems -lSTGmemg \
			-lSTGtimerN -lSTGcommon_stg -lSTGhashg -lUtil -lPKT -lPSESS -lANZ_LIB -lInitShm -lTOOLS -lSHMUTIL \
			-lGREENTRYSESS

LIBRARY = -lLOGLIB -lVERLIB -lFILELIB  -lIPCLIB -lUTILLIB
LIBRARY+= -lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg
LIBRARY+= -lPSESS -lGREENTRYSESS -lANZ_LIB

LDFLAGS  = $(LIBS) $(LIBRARY)

OBJS    = $(SRCS:.c=.o)

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

lib:
	$(MAKE) -C MMDB_PSESS
	$(MAKE) -C MMDB_GREENTRY

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	#$(MAKE) -C MMDB_PSESS clean
	#$(MAKE) -C MMDB_GREENTRY clean
	/bin/rm -f *.o $(TARGET) $(TARGET2) core* 

doc:
	doxygen

# DO NOT DELETE THIS LINE -- make depend depends on it.
