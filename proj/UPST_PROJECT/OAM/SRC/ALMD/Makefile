# -------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`
MAKEDEPEND	=`which makedepend`
MAKEFILE	=Makefile

CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

LIB_HOME	= ../../../LIB
OMP_HOME	= ../..
PRJ_HOME	= ../../..
INC_DIRS	=-I. -I${OMP_HOME}/INC -I${PRJ_HOME}/INC -I${LIB_HOME}/INC -I../../../TAM/INC
INC_DIRS	+=-I/usr/lib/gcc/i486-linux-gnu/3.4.6/include
DEFINES		=-DPRINT -DAPPLOG

OPTIMIZE	=-g3 -Wall

LIB_DIRS	=-L../../LIB -L${LIB_HOME}/LIB
LIBS		=-lLOGLIB -lIPCLIB -lFILELIB -lUTILLIB -lVERLIB 
LIBS		+=-lnifo -lgifo -lcifo 

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=ALMD

SRCS		=fstat_mmc.c almd_init.c almd_main.c almd_mem.c almd_sock.c almd_func.c
OBJS		=${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LDFLAGS}

dep: ${SRCS}
	${MAKEDEPEND} -o.o -f ${MAKEFILE} -- ${DEFINES} ${INC_DIRS} -- $?

clean:
	rm -f ${TARGET} ${OBJS} core*

install:
	@for i in ${TARGET}; do \
		echo Installing $$i in ${DEST_DIR}; \
		${CP} $$i ${DEST_DIR}; \
	done

# DO NOT DELETE
