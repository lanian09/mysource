#!/bin/sh
CC			= cc
GCC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g -Wall
CFLAG_TRU64	= -g -Wall -DTRU64

INC_PATH	= -I. -I../../include -I../../libsrc/COMM

LIB_PATH	= -L../../lib

LIBS		= -lcomm -lsocket -lnsl
LIBS_TRU64	= -lcomm

SRCS		= ifb_comm.c
SRCS1		= disprc.c
SRCS2		= startprc.c
SRCS3		= killprc.c
SRCS4		= sysrmqm.c
SRCS5		= clrq.c

OBJS		= $(SRCS:.c=.o)
OBJS1		= $(SRCS1:.c=.o)
OBJS2		= $(SRCS2:.c=.o)
OBJS3		= $(SRCS3:.c=.o)
OBJS4		= $(SRCS4:.c=.o)
OBJS5		= $(SRCS5:.c=.o)

#SRCS1		= ifb_comm.c  disprc.c
#SRCS2		= ifb_comm.c  startprc.c
#SRCS3		= ifb_comm.c  killprc.c
#SRCS4		= ifb_comm.c  sysrmqm.c
#SRCS5		= ifb_comm.c  clrq.c

#OBJS1		= $(SRCS1:.c=.o)
#OBJS2		= $(SRCS2:.c=.o)
#OBJS3		= $(SRCS3:.c=.o)
#OBJS4		= $(SRCS4:.c=.o)
#OBJS5		= $(SRCS5:.c=.o)


AOUT1		= disprc
AOUT2		= startprc
AOUT3		= killprc
AOUT4		= rmsysqm
AOUT5		= clrq

DEST		= ../../bin

#
#---------------------------------------------------------------------
#
all:: $(AOUT1) $(AOUT2) $(AOUT3) $(AOUT4) $(AOUT5) install

$(OBJS): $(@:.o=.c)
	$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)

$(OBJS1): $(@:.o=.c)
	$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $(@:.o=.c) ; \
#	else \
#		$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) ; \
#	fi

$(AOUT1): $(OBJS) $(OBJS1) 
	$(GCC) -o $(AOUT1) $(OBJS1) $(OBJS) $(LIB_PATH) $(LIBS) 

#	if [ `uname` = "OSF1" ]; then \
#		$(CC) -o $(AOUT1) $(OBJS1) $(LIB_PATH) $(LIBS_TRU64) ; \
#	else \
#		$(GCC) -o $(AOUT1) $(OBJS1) $(LIB_PATH) $(LIBS) ; \
#	fi

$(OBJS2): $(@:.o=.c)
	$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $(@:.o=.c) ; \
#	else \
#		$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) ; \
#	fi

$(AOUT2): $(OBJS) $(OBJS2)
	$(GCC) -o $(AOUT2) $(OBJS) $(OBJS2) $(LIB_PATH) $(LIBS)

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) -o $(AOUT2) $(OBJS2) $(LIB_PATH) $(LIBS_TRU64) ; \
#	else \
#		$(GCC) -o $(AOUT2) $(OBJS2) $(LIB_PATH) $(LIBS) ; \
#	fi

$(OBJS3): $(@:.o=.c)
	$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $(@:.o=.c) ; \
#	else \
#		$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) ; \
#	fi

$(AOUT3): $(OBJS) $(OBJS3)
	$(GCC) -o $(AOUT3) $(OBJS) $(OBJS3) $(LIB_PATH) $(LIBS) 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) -o $(AOUT3) $(OBJS3) $(LIB_PATH) $(LIBS_TRU64) ; \
#	else \
#		$(GCC) -o $(AOUT3) $(OBJS3) $(LIB_PATH) $(LIBS) ; \
#	fi

$(OBJS4): $(@:.o=.c)
	$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $(@:.o=.c) ; \
#	else \
#		$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) ; \
#	fi

$(AOUT4): $(OBJS) $(OBJS4)
	$(GCC) -o $(AOUT4) $(OBJS) $(OBJS4) $(LIB_PATH) $(LIBS) 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) -o $(AOUT4) $(OBJS4) $(LIB_PATH) $(LIBS_TRU64) ; \
#	else \
#		$(GCC) -o $(AOUT4) $(OBJS4) $(LIB_PATH) $(LIBS) ; \
#	fi

$(OBJS5): $(@:.o=.c)
	$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $(@:.o=.c) ; \
#	else \
#		$(GCC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c) ; \
#	fi

$(AOUT5): $(OBJS) $(OBJS5)
	$(GCC) -o $(AOUT5) $(OBJS) $(OBJS5) $(LIB_PATH) $(LIBS) 

#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) -o $(AOUT5) $(OBJS5) $(LIB_PATH) $(LIBS_TRU64) ; \
#	else \
#		$(GCC) -o $(AOUT5) $(OBJS5) $(LIB_PATH) $(LIBS) ; \
#	fi


install:
	cp $(AOUT1) $(AOUT2) $(AOUT3) $(AOUT4) $(AOUT5) $(DEST)

clean:
	rm -f $(OBJS) $(OBJS1) $(OBJS2) $(OBJS3) $(OBJS4) $(OBJS5) $(AOUT1) $(AOUT2) $(AOUT3) $(AOUT4) $(AOUT5) core
