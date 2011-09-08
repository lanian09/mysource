# -------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`
MAKEFILE	= Makefile

CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

LIB_HOME    = ../../../LIB
OMP_HOME    = ../..
PRJ_HOME    = ../../..
INC_DIRS    =-I. -I${OMP_HOME}/INC -I${PRJ_HOME}/INC -I${LIB_HOME}/INC
DEFINES		=-DDEBUG -DLOGDEBUG
DEFINES	   +=-D_NO_HPLOG
#DEFINES	   +=-DENABLE_ENCLOSURE

OPTIMIZE	=-g3 -Wall
#OPTIMIZE   +=-O2 -fno-strict-aliasing	# -g 와 -O option은 동시에 주면 안됨... refer to man gcc
OPTIMIZE   += -fno-strict-aliasing

LIB_DIRS    =-L../../LIB -L${LIB_HOME}/LIB
LIBS		=
LIBS	   +=-lLOGLIB
LIBS	   +=-lUTILLIB
LIBS	   +=-lVERLIB
LIBS	   +=-lIPCLIB
LIBS       +=-lnifo -lgifo -lcifo

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=CHSMD

#SRCS		=chsmd_disk.c chsmd_hw.c chsmd_func.c chsmd_init.c chsmd_load.c
#SRCS		+=chsmd_main.c chsmd_mmc.c chsmd_mmc_func.c chsmd_ntpd.c chsmd_sw.c

SRCS		=
#SRCS	   +=t.c	# <---------------- TEST  용 main
SRCS	   +=chsmd_main.c
SRCS	   +=chsmd_ntpd.c
SRCS	   +=chsmd_load.c chsmd_msg.c chsmd_init.c
SRCS	   +=chsmd_sw.c
SRCS	   +=chsmd_mmc.c
SRCS	   +=chsmd_mmc_func.c
SRCS	   +=chsmd_hw.c
SRCS	   +=chsmd_disk.c
SRCS	   +=chsmd_func.c

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

lint:
	lint ${INC_DIRS} ${SRCS}

clean:
	rm -f ${TARGET} ${OBJS} core*

install:
	@for i in ${TARGET}; do \
		echo Installing $$i in ${DEST_DIR}; \
		${CP} $$i ${DEST_DIR}; \
	done

# DO NOT DELETE
