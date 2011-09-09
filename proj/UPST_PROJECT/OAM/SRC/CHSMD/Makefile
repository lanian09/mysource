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
DEFINES		=
DEFINES	   +=-DCLI_VERSION

OPTIMIZE	=-g3 -Wall -fPIC
#OPTIMIZE   +=-O2 -fno-strict-aliasing # -g 와 -O option은 동시에 주면 안됨... refer to man gcc
OPTIMIZE   += -fno-strict-aliasing

#ORACLE_LIB_DIRS	=-L${ORACLE_HOME}/lib
#ORACLE_LIBS		=-lclntsh

LIB_DIRS    =-L../../LIB -L${LIB_HOME}/LIB
LIBS        =
LIBS       +=-lpthread
LIBS       +=-lLOGLIB
LIBS       +=-lUTILLIB
LIBS       +=-lVERLIB
LIBS       +=-lIPCLIB
LIBS	   +=-lFIDBLIB
LIBS	   +=-lFILELIB
LIBS	   +=-lnifo -lgifo -lcifo

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS} ${ORACLE_LIB_DIRS} ${ORACLE_LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=CHSMD

SRCS        = 
SRCS       += chsmd_cpu.c
SRCS       += chsmd_disk.c
SRCS       += chsmd_hw.c chsmd_msg.c chsmd_sw.c chsmd_mmc_func.c chsmd_mmc.c chsmd_init.c
SRCS       += chsmd_link.c
SRCS       += chsmd_mem.c
SRCS       += chsmd_ntp.c
SRCS	   += chsmd_main.c
SRCS	   += chsmd_mask.c
SRCS	   += chsmd_func.c

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
