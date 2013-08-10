# -------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`

CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

LIB_HOME    = ../../../LIB
OMP_HOME    = ../..
PRJ_HOME    = ../../..
INC_DIRS    =-I. -I${OMP_HOME}/INC -I${PRJ_HOME}/INC -I${LIB_HOME}/INC
DEFINES		=-DPRINT -DDEBUG -DAPPLOG
DEFINES     +=-D_ENABLE_HEARTBEAT_

OPTIMIZE	=-g3 -Wall -fPIC

#ORACLE_LIB_DIRS	=-L${ORACLE_HOME}/lib
#ORACLE_LIBS		=-lclntsh

LIB_DIRS    =-L../../LIB -L${LIB_HOME}/LIB
LIB_DIRS   +=-L/usr/lib64/mysql
LIBS        =-lLOGLIB -lDBLIB -lUTILLIB -lIPCLIB -lVERLIB -lFILELIB
LIBS	   +=-lnifo -lgifo -lcifo -lmysqlclient -lz -lm

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS} ${ORACLE_LIB_DIRS} ${ORACLE_LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=COND

SRCS        =
SRCS	   +=cond_db.c
SRCS	   +=cond_mem.c
SRCS	   +=cond_func.c cond_msg.c cond_sock.c
SRCS	   +=cond_init.c
SRCS	   +=cond_main.c
SRCS	   +=cond_ipc.c

OBJS		=${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LDFLAGS}

dep: ${SRCS}
	${CC} -M ${CFLAGS} -c $?

clean:
	rm -f ${TARGET} ${OBJS} core*

install:
	@for i in ${TARGET}; do \
		echo Installing $$i in ${DEST_DIR}; \
		${CP} $$i ${DEST_DIR}; \
	done

# DO NOT DELETE
