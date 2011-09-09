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

INC_DIRS	=-I. -I../../INC -I../../../INC -I../../../OAM/INC -I../../../LIB/INC 
INC_DIRS	+=-I/usr/lib/gcc/i486-linux-gnu/3.4.6/include
DEFINES		=-DDEBUG -DAPPLOG
## DIAMETER를 파일로 남기기 위한 모듈
DEFINES		+=-DENABLE_ANALYZE_DIAMETER
## DSTA 연동 모듈
DEFINES		+= -DENABLE_WEB_LOG
DEFINES		+= -DIMSI2MIN

OPTIMIZE	=-g3 -Wall

LIB_DIRS	=-L../../LIB -L../../../LIB/LIB 
LIBS		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB -lNSOCKLIB -lSTGcommon_stg
LIBS		+= -lnifo -lgifo -lcifo -ltimerN -lhasho -lm

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=M_LOG

SRCS		=mlog_init.c mlog_main.c mlog_func.c mlog_msgq.c mlog_if.c
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
