#!/bin/sh
CC		= gcc
RM		= rm -f
DEPEND	= makedepend
LINT	= lint
CFLAG	= -g #-DDEBUG-DTRC_TIMER
DEFINES	= -DTPS #-DDEBUG #-DTRU64 #-DRESTRICTED_CLIENT #-DSOREUSEPORT #-DTCPNODELAY

INC_DIR	= -I. -I../../include
LIB_DIR	= -L../../lib
LIBS	=
CFLAGS	= $(CFLAG) $(DEFINES) $(INC_DIR)

SRCS	=	conflib.c \
			loglib.c \
			socklib.c \
			keepalivelib.c \
			trclib.c \
			comm_util.c \
			proc_version.c \
			check_run_myproc.c \
			netOrderChange.c \
			netOrderSysInfoChange.c \
			netOrderSceInfoChange.c \
			netOrderL3pdChange.c \
			strutil.c \
			ping.c 
#			timer.c

OBJS	= $(SRCS:.c=.o)

AOUT	= libcomm.a

DEST	= ../../lib

#
#---------------------------------------------------------------------
#
all:: $(AOUT) install

$(AOUT): $(OBJS)
	ar rcus $@ $(OBJS)
#	ar ts $(DEST)/$(AOUT)

install:
	\cp $(AOUT) $(DEST)

clean:
	\rm -f $(OBJS) $(AOUT) core
