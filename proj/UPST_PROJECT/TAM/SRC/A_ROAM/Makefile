#**********************************************************
#                 DQMS Project
#
#	Author   : Han-jin Park
#	Section  : DQMS Project
#	SCCS ID  : Makefile
#	Date	 : 05/20/09
#	Revision History :
#			'09. 5. 20 Initial
#
#	Description:
#
#	Copyright (c)uPRESTO 2005
#***********************************************************

# -------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`
MAKEDEPEND	=`which makedepend`
MAKEFILE	=Makefile

CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

INC_DIRS	=-I. -I../../INC -I../../../INC -I../../../LIB/INC -I../../../OAM/INC
INC_DIRS	+=-I/usr/lib/gcc/i486-linux-gnu/3.4.6/include
DEFINES		=-D_DEBUG -DAPPLOG
OPTIMIZE	=-g3 -Wall

#ORACLE_LIB_DIRS	=-L${ORACLE_HOME}/lib
#ORACLE_LIBS		=-lclntsh

LIB_DIRS	=-L../../../LIB/LIB
#LIBS		=-lhasho -lmemg -lnifo -lmems -ltimerN -lhashg -lSTGtimerN -lSTGhashg -lSTGhasho -lSTGmemg -lSTGnifo -lSTGmems -lSTGcommon_stg -lm
LIBS		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB
LIBS	   += -lnifo -lgifo -lcifo -ltimerN -lhasho -lm

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS} ${ORACLE_LIB_DIRS} ${ORACLE_LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=A_ROAM

SRCS		=roam_main.c roam_init.c roam_func.c roam_msgq.c roam_util.c
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
