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

OAM_DIR		=../../../OAM
LIB_DIR		=../../../LIB

INC_DIRS	=-I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
INC_DIRS	+=-I/usr/lib/gcc/i486-linux-gnu/3.4.6/include
DEFINES		=-DPRINT -D__LITTLE_ENDIAN_BITFIELD -D_GNU_SOURCE
DEFINES		+=-D_ENABLE_CONFIGURATION_LOADED_FILENAME_

OPTIMIZE	=-g3 -Wall

LIB_DIRS	=-L../../LIB -L/usr/lib64/mysql -L$(LIB_DIR)/LIB
#LIBS		=-lLogUtil -lShmUtil -lShmQueue -lMySQLDB -lmysqlclient -lm -lz
LIBS		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB -lmysqlclient -lDBLIB -lFILELIB
LIBS	   += -lnifo -lgifo -lcifo -ltimerN -lhasho -lm -lz

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../BIN
#------------------------ Source & Objects & Libraries --------------------------
TARGET		=SI_NMS

SRCS		=si_nms_main.c si_nms_init.c si_nms_func.c si_nms_sock.c
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
