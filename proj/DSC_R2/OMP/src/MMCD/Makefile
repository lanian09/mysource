#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
CFLAG		= -g -Wall -DDEBUG_MODE
CFLAG       +=-DDEBUG

#INC_PATH	= -I. -I../../include -I../../libsrc/COMM -I/opt/mysql/mysql/include
INC_PATH    = -I. -I../../include -I../../libsrc/COMM -I${MYSQL_HOME}/include

#LIB_PATH	= -L../../lib -L/opt/mysql/mysql/lib
LIB_PATH    = -L../../lib -L${MYSQL_HOME}/lib


#20041202.mnpark
#LIBS		= -lcomm -lsocket -lnsl -lmysqlclient -lm
LIBS		= -lcomm -lsocket -lnsl -lmysqlclient -lz -lm 

SRCS		=	mmcd_main.c \
				mmcd_init.c \
				mmcd_req.c \
				mmcd_res.c \
				mmcd_builtin.c \
				mmcd_proc.c \
				mmcd_loadcmdtbl.c \
				mmcd_tools.c

SRCS2		=	build_mmldb.c \
				mmcd_loadcmdtbl.c \
				mmcd_tools.c

OBJS		= $(SRCS:.c=.o)
OBJS2		= $(SRCS2:.c=.o)

AOUT		= mmcd
AOUT2		= build-mmldb

DEST		= ../../bin

#
#---------------------------------------------------------------------
#
all:: $(AOUT) $(AOUT2) install

#$(OBJS): $(@:.o=.c)
#	$(CC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)

#$(OBJS2): $(@:.o=.c)
#	$(CC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(INC_PATH) -c $<


$(AOUT): $(OBJS)
	$(CC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS)

$(AOUT2): $(OBJS2)
	$(CC) -o $(AOUT2) $(OBJS2) $(LIB_PATH) $(LIBS)


install:
	cp $(AOUT) $(AOUT2) $(DEST)

clean:
	rm -f $(OBJS) $(OBJS2) $(AOUT) $(AOUT2) core
