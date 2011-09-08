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

OAM_DIR		=../../../OAM
LIB_DIR		=../../../LIB

INC_DIRS	=-I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
INC_DIRS	+=-I/usr/lib/gcc/i486-linux-gnu/3.4.6/include
DEFINES		=-DDEBUG -DCHANGE_USER_LEVEL -DAPPLOG

OPTIMIZE	=-g3 -Wall

LIB_DIRS	=-L../../LIB -L$(LIB_DIR)/LIB
LIBS		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB -lDBLIB -lFILELIB
LIBS	   += -lnifo -lgifo -lcifo -ltimerN -lhasho -lTOOLS -lm
#LIBS        =-lShmQueue -lLogUtil -lShmUtil -lhasho -lmemg -lnifo -lmems -ltimerN -lhashg -lSTGtimerN -lSTGhashg -lSTGhasho -lSTGmemg -lSTGnifo -lSTGmems -lSTGcommon_stg

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=M_TRACE

SRCS		=trace_init.c trace_main.c trace_func.c trace_link.c
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
