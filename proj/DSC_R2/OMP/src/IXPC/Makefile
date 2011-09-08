#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g -Wall
CFLAG_TRU64	= -g -DTRU64

INC_PATH	= -I. -I../../include -I../../libsrc/COMM

LIB_PATH	= -L../../lib

LIBS		= -lcomm -lsocket -lnsl
LIBS_TRU64	= -lcomm 

SRCS		=	ixpc_main.c \
				ixpc_init.c \
				ixpc_route.c \
				ixpc_proc.c \
				socklib.c

OBJS		= $(SRCS:.c=.o)

AOUT		= ixpc

DEST		= ../../bin

#
#---------------------------------------------------------------------
#
all:: $(AOUT) install

#$(OBJS): $(@:.o=.c)
#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $(@:.o=.c) ; \
#	else \
#		$(CC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) ; \
#	fi
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(INC_PATH) -c $< 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $< ; \
#	else \
#		$(CC) $(CFLAG) $(INC_PATH) -c $< ; \
#	fi


$(AOUT): $(OBJS)
	$(CC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS) 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS_TRU64) ; \
#	else \
#		$(CC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS) ; \
#	fi


install:
	cp $(AOUT) $(DEST)

clean:
	rm -f $(OBJS) $(AOUT) core
