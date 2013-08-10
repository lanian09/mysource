#**********************************************************
#                 DQMS Project
#
#	Author	: 
#	Section	: 
#	SCCS ID	: 
#	Date	 	: 
#	Revision History :
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
CP			=`which cp`
MV			=`which mv`
RM			=`which rm`
MAKEFILE		= Makefile


OPTIMIZE	= -g3 -Wall
#DEFINES	= -DDEBUG -DPRINT
DEFINES		= -DDEBUG -DPRINT

INC_DIRS	=-I. -I../../INC -I../../../INC -I../../../OAM/INC -I../../../LIB/INC 
INC_DIRS	+= -I/usr/lib/gcc/i486-linux-gnu/3.4.6/include

LIB_DIRS	=-L../../../LIB/LIB -L../../LIB
LIBS		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB -lNSOCKLIB
#LIBS		+= -lnifo -lgifo -lcifo -ltimerN -lhasho -lmemg -lSTGhasho -lSTGmemg -lm
LIBS		+= -lnifo -lgifo -lcifo -ltimerN -lhasho -lm

CFLAGS		= ${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		= ${LIB_DIRS} ${LIBS}

DEST_DIR	= ../../BIN

#-------------------- Source & Objects & Libraries------------------------

TARGET		= CHGSVCM

SRCS		= chgsvc_init.c chgsvc_main.c chgsvc_proc.c chgsvc_mem.c chgsvc_if.c
OBJS		= ${SRCS:.c=.o}

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
