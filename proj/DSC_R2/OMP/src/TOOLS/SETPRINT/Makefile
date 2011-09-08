#!/bin/sh
CC			= cc
GCC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g
CFLAG_TRU64	= -g

INC_PATH	= -I. -I../../../include -I../../../libsrc/COMM

LIB_PATH	= -L../../../lib

LIBS		= -lcomm -lsocket -lnsl
LIBS_TRU64	= -lcomm

SRCS		= setprint.c

OBJS		= $(SRCS:.c=.o)

AOUT		= setprint

DEST		= ../../../bin

#
#---------------------------------------------------------------------
#
all:: $(AOUT) install

$(OBJS): $(@:.o=.c)
	if [ `uname` = "OSF1" ] ; then \
		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $(@:.o=.c) ; \
	else \
		$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) ; \
	fi

$(AOUT): $(OBJS)
	if [ `uname` = "OSF1" ] ; then \
		$(CC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS_TRU64) ; \
	else \
		$(GCC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS) ; \
	fi


install:
	\cp $(AOUT) $(DEST)

clean:
	\rm -f $(OBJS) $(AOUT) core
