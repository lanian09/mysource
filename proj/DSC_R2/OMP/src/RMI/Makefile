#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g -Wall

INC_PATH	= -I. -I../../include -I../../libsrc/COMM

LIB_PATH	= -L../../lib

LIBS		= -lcomm -lsocket -lnsl

SRCS		=	rmi_main.c \
				rmi_init.c \
				rmi_builtin.c

OBJS		= $(SRCS:.c=.o)

AOUT		= rmi

DEST		= ../../bin

#
#---------------------------------------------------------------------
#
all:: $(AOUT) install

#$(OBJS): $(@:.o=.c)
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(INC_PATH) -c $<

$(AOUT): $(OBJS)
	$(CC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS)


install:
	cp $(AOUT) $(DEST)

clean:
	rm -f $(OBJS) $(AOUT) core
