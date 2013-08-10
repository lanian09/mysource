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

#------------------------ Makefile which have no SubDir -------------------------
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
DEFINES		=-DPRINT
DEFINES     +=-DDNMS_CONN_DISABLE

OPTIMIZE	=-g3 -Wall

LIB_DIRS	=-L../../../LIB/LIB
LIBS		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB -lNSOCKLIB
LIBS		+= -lnifo -lgifo -lcifo -ltimerN -lhasho -lm

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../BIN
#------------------------ Source & Objects & Libraries --------------------------
TARGET		=SI_SVCMON

SRCS		=si_svcmon_func.c si_svcmon_init.c si_svcmon_main.c si_svcmon_sock.c si_svcmon_msgq.c clist_memg.c si_svcmon_if.c
OBJS		=${SRCS:.c=.o}
#--------------------------------------------------------------------------------
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
