
SHELL		= /bin/sh
MAKE		= `which make`
CC			= gcc
AR			= ar
RM			= /bin/rm
MV			= mv
DEPEND		= makedepend
LINT		= lint

BIN_DIR 	= ../../BIN
LIB_DIR 	= ../../../LIB
OAM_DIR 	= ../../../OAM
LIBS    	= -L. -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER	=$(shell uname -r)
KERNEL_DIR	=
#KERNEL_DIR+=/lib/modules/$(KERNEL_VER)/source/include

INCLUDES	= -I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
DEBUG   	= -g3 -Wall
DEFINES 	= -DPRINT -DDEBUG

ifeq ($(SYSTYPE), RP)
DEFINES    += -DRP
endif
ifeq ($(SYSTYPE), PI)
DEFINES    += -DPI
endif
ifeq ($(SYSTYPE), PIOLD)
DEFINES    += -DPIOLD
endif

CFLAGS		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)

SRCS		= 
SRCS	   += ftp_init.c 
SRCS	   += ftp_main.c 
SRCS	   += ftp_svc.c

TARGET		= A_FTP

OBJS		= $(SRCS:.c=.o)

#LIBS    	= $(LIBRARY) -lLogUtil -lTOOLS -lUtil -lMMDBLISTFTP -lnifo -lmems -lSTGnifo -lSTGmems -lSTGcommon_stg -lInitShm -lSHMUTIL

LIBRARY 	= -lLOGLIB -lVERLIB -lFILELIB -lIPCLIB -lUTILLIB
LIBRARY	   += -lnifo -lgifo -lcifo -lhasho -ltimerN -lSTGcommon_stg
LIBRARY	   += -lMMDBLISTFTP -lTOOLS

LDFLAGS 	= $(LIBS) $(LIBRARY)

MAKEFILE	= Makefile

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
	$(MAKE) -C MMDBLIST_FTP

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(CC) -M $(CFLAGS) -c $?
	#$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	$(MAKE) -C MMDBLIST_FTP $@
	/bin/rm -f *.o *.a $(TARGET) $(TARGET2) core* 

# DO NOT DELETE THIS LINE -- make depend depends on it.
