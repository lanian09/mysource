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

INC_DIRS	=-I./ -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
INC_DIRS	+=-I/usr/lib/gcc/i486-linux-gnu/3.4.6/include
DEFINES		=-DDEBUG -DAPPLOG

OPTIMIZE	=-g3 -Wall

LIB_DIRS	=-L../../LIB -L/usr/lib64/mysql -L$(LIB_DIR)/LIB
LIBS		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB -lmysqlclient -lDBLIB -lFILELIB -lNSOCKLIB
LIBS	   += -lnifo -lgifo -lcifo -ltimerN -lhasho -lm

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=SI_DB

#SRCS		=si_db_init.c si_db_main.c si_db_sock.c nsock.c si_db_func.c si_db_msgq.c clist_memg.c
SRCS		=si_db_init.c si_db_main.c si_db_sock.c si_db_func.c si_db_msgq.c clist_memg.c
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
