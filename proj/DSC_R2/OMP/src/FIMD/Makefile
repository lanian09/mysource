#!/bin/sh
CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
#CFLAG		= -g  -DDEBUG -Wall
# 프로세스별 알람 코드/타입을 사용하지 않을때.
#CFLAG		= -g  -DDEBUG -Wall -D_NOT_USED_PROCESS_ALARM_TYPE_
#CFLAG		= -g  -DDEBUG -Wall 
CFLAG		= -g  -Wall -D_TPS_ -DDEBUG -D_REENTRANT 

INC_PATH	= -I. -I../../include -I../../libsrc/COMM -I${MYSQL_HOME}/include

LIB_PATH	= -L../../lib -L${MYSQL_HOME}/lib

#20041202.mnpark
#LIBS		= -lcomm -lsocket -lnsl -lmysqlclient -lm -lpthread
LIBS		= -lcomm -lsocket -lnsl -lmysqlclient -lz -lm -lpthread
LIBS2		= -lcomm -lsocket -lnsl

SRCS		=	fimd_main.c \
				fimd_init.c \
				fimd_mysql.c \
				fimd_almlevel.c \
				fimd_almmsg.c \
				fimd_hdlalm.c \
				fimd_hwalm.c \
				fimd_mmchdl.c \
				fimd_proc.c \
				fimd_stat.c \
				fimd_stsrpt.c \
				fimd_dupl.c \
				fimd_tools.c \
				fimd_keepalive.c \
				fimd_pdalmlmt.c \
				fimd_rxtxmsg.c \
				fimd_scealmlmt.c \
				fimd_legalmlmt.c \
				fimd_l2swalmlmt.c

SRCS2	= showsfdb.c
#SRCS3	= testsfdb.c

OBJS		= $(SRCS:.c=.o)
OBJS2		= $(SRCS2:.c=.o)
#OBJS3		= $(SRCS3:.c=.o)

AOUT		= fimd
AOUT2		= showsfdb
AOUT3		= testsfdb

DEST		= ../../bin

#
#---------------------------------------------------------------------
#
#all:: $(AOUT) $(AOUT2) $(AOUT3) install
all:: $(AOUT) $(AOUT2) install

#$(OBJS): $(@:.o=.c)
#	$(CC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)

#$(OBJS2): $(@:.o=.c)
#	$(CC) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAG) $(INC_PATH) -c $<


$(AOUT): $(OBJS)
	$(CC) -o $(AOUT) $(CFLAG) $(OBJS) $(LIB_PATH) $(LIBS)

$(AOUT2): $(OBJS2)
	$(CC) -o $(AOUT2) $(CFLAG) $(OBJS2) $(LIB_PATH) $(LIBS2)

#test: $(AOUT3)
#$(AOUT3): $(OBJS3)
#	$(CC) -o $(AOUT3) $(CFLAG) $(OBJS3) $(LIB_PATH) $(LIBS2)


install:
	cp $(AOUT) $(AOUT2) $(DEST)

#clean:
#	rm -f $(OBJS) $(OBJS2) $(OBJS3) $(AOUT) $(AOUT2) $(AOUT3) core
clean:
	rm -f $(OBJS) $(OBJS2) $(OBJS3) $(AOUT) $(AOUT2) $(AOUT3) core

#clean3:
#	rm -f $(OBJS3) $(AOUT3) core

