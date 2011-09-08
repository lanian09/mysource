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
INC_DIRS    =-I. -I${OMP_HOME}/INC -I${PRJ_HOME}/INC -I${LIB_HOME}/INC -I../../../TAM/INC
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
#LIBS	   +=-lMySQLDB
LIBS	   +=-lmysqlclient
LIBS	   +=-ldl -lm -lz

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS} ${ORACLE_LIB_DIRS} ${ORACLE_LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=MMCD
SO_LIB		=ERRPRNLIB

SRCS		=mmcd_db.c builtin.c cmd_get.c cmd_load.c cmd_user.c help_proc.c
SRCS		+=lib_load.c mmcd_main.c mmcd_init.c mmcd_mem.c mmcd_sock.c
SRCS		+=mmcd_util.c para_load.c reload.c
OBJS		=${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: so ${TARGET} install

so:
	${MAKE} -C ${SO_LIB}

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LDFLAGS}

dep: ${SRCS}
	${CC} -M ${CFLAGS} -c $?

clean:
	${MAKE} -C ${SO_LIB} clean
	rm -f ${TARGET} ${OBJS} core*

install:
	@for i in ${TARGET}; do \
		echo Installing $$i in ${DEST_DIR}; \
		${CP} $$i ${DEST_DIR}; \
	done

# DO NOT DELETE
