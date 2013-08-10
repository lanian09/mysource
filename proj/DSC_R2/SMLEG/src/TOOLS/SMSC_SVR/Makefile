#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= /usr/openwin/bin/makedepend
LINT		= lint
CFLAG		= -g
CFLAG_TRU64	= -g -DTRU64

INC_PATH	= -I. -I../../include -I../../../include -I../../libsrc/COMM  -I${MYSQL_HOME}

LIB_PATH	= -L../../lib -L../../../lib

LIBS		= -lcomm  -lInitShm -lsocket -lnsl
LIBS_TRU64	= -lcomm -lInitShm

SRCS		=	smsc_main.c \
				smsc_init.c \
				smsc_proc.c \
				socklib.c

OBJS		= $(SRCS:.c=.o)

AOUT		= SMSC 

#DEST		= ../../bin
DEST		= /DSC/NEW/BIN/

#
#---------------------------------------------------------------------
#
all:: $(AOUT)

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

dep: ${SRCS}
	${DEPEND} -o.o -f Makefile -- ${INC_PATH} -- ${SRCS}

install:
	\cp $(AOUT) $(DEST)

clean:
	\rm -f $(OBJS) $(AOUT) core
# DO NOT DELETE

