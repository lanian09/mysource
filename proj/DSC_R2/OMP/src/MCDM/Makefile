#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g -Wall

INC_PATH	= -I. -I../../include -I../../libsrc/COMM

LIB_PATH	= -L../../lib -L/usr/local/mysql/lib

LIBS		= -lcomm -lsocket -lnsl -lpthread

SRCS		=	mcdm_main.c \
				mcdm_init.c \
				mcdm_ownmmc.c \
				mcdm_rxdistrreq.c \
				mcdm_rxdistrres.c \
				mcdm_proc.c \
				mcdm_tools.c

OBJS		= $(SRCS:.c=.o)

AOUT		= mcdm

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
