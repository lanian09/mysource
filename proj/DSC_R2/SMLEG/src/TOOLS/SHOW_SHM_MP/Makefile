#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g  -DDEBUG

INC_PATH	= -I. -I../../../include -I../../libsrc/COMM -I${MYSQL_HOME}/include

LIB_PATH	= -L../../../lib -L${MYSQL_HOME}/lib

LIBS		= -lcomm -lsocket -lnsl -lmysqlclient -lz -lm -lpthread
LIBS2		= -lcomm -lsocket -lnsl

SRCS		= show_shm.c

OBJS		= $(SRCS:.c=.o)

AOUT		= show_shm

DEST		= ../../../bin

#
#---------------------------------------------------------------------
#
all:: $(AOUT) install


.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(INC_PATH) -c $<


$(AOUT): $(OBJS)
	$(CC) -o $(AOUT) $(CFLAG) $(OBJS) $(LIB_PATH) $(LIBS)


install:
	cp $(AOUT) $(DEST)

clean:
	rm -f $(OBJS) $(AOUT)  core
