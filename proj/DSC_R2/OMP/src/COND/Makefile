#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g -Wall
#CFLAG		= -g -m64

INC_PATH	= -I. -I../../include -I../../libsrc/COMM

LIB_PATH	= -L../../lib

LIBS		= -lcomm -lsocket -lnsl -lpthread

SRCS		=	cond_main.c \
				cond_init.c \
				cond_proc.c \
				cond_mmchdl.c \
				cond_tools.c

OBJS		= $(SRCS:.c=.o)

AOUT		= cond

DEST		= ../../bin

#
#---------------------------------------------------------------------
#
all:: $(AOUT) install

#$(OBJS): $(@:.o=.c)
#	$(CC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(INC_PATH) -c $<


$(AOUT): $(OBJS)
	$(CC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS)


install:
	cp $(AOUT) $(DEST)

clean:
	rm -f $(OBJS) $(AOUT) core
