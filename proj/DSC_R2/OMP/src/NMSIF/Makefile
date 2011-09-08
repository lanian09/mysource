#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g  -DDEBUG -D_TPS_

INC_PATH	= -I. -I../../include -I../../libsrc/COMM -I${MYSQL_HOME}/include

LIB_PATH	= -L../../lib -L${MYSQL_HOME}/lib

LIBS		= -lcomm -lsocket -lnsl -lmysqlclient -lz -lm -lpthread

SRCS		=	nmsif_main.c \
				nmsif_init.c \
				socket.c \
				msgif.c \
				tool.c \
				netOrderNmsChange.c

OBJS		= $(SRCS:.c=.o)

AOUT		= nmsif

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
	$(CC) -o $(AOUT) $(CFLAG) $(OBJS) $(LIB_PATH) $(LIBS)

install:
	cp $(AOUT) $(DEST)

clean:
	rm -f $(OBJS) $(AOUT) core
