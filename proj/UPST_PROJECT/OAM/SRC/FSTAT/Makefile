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
INC_DIRS   +=-I/usr/lib/gcc/i386-redhat-linux/3.4.6/include/
DEFINES		=-DPRINT -DAPPLOG
DEFINES	   +=-DCLI_VERSION

OPTIMIZE	=-g3 -Wall -fPIC

#ORACLE_LIB_DIRS	=-L${ORACLE_HOME}/lib
#ORACLE_LIBS		=-lclntsh

LIB_DIRS    =-L../../LIB -L${LIB_HOME}/LIB
LIB_DIRS   +=-L/usr/lib64/mysql/
LIBS        =
LIBS       +=-lpthread
LIBS       +=-lLOGLIB
LIBS       +=-lUTILLIB
LIBS       +=-lVERLIB
LIBS       +=-lIPCLIB
LIBS	   +=-lFIDBLIB
LIBS	   +=-lFILELIB
LIBS	   +=-lnifo -lgifo -lcifo
LIBS	   +=-lDBLIB
LIBS	   +=-lmysqlclient
LIBS	   +=-ltimerN
#LIBS	   +=-lSTGtimerN
#LIBS	   +=-lhashg
#LIBS	   +=-lSTGhashg
LIBS	   +=-ldl -lm -lz

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS} ${ORACLE_LIB_DIRS} ${ORACLE_LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=FSTAT

SRCS		=fstat_db.c fstat_fault.c fstat_init.c fstat_load.c
SRCS		+=fstat_main.c m_msgq.c fstat_traffic.c
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
