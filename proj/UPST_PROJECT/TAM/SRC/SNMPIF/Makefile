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
#DEFINES		=-DPRINT -DAPPLOG -DSNMPIF `/usr/bin/net-snmp-config --cflags`
DEFINES		=-DPRINT -DAPPLOG -DSNMPIF

OPTIMIZE	=

#ORACLE_LIB_DIRS	=-L${ORACLE_HOME}/lib
#ORACLE_LIBS		=-lclntsh

LIB_DIRS	=-L../../LIB -L/usr/lib64/mysql -L$(LIB_DIR)/LIB
LIBS		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB -lmysqlclient -lDBLIB -lFILELIB
LIBS	   += `/usr/bin/net-snmp-config --agent-libs` -lnifo -lgifo -lcifo -ltimerN -lhasho -lm
#LIBS		=-lShmQueue -lLogUtil -lShmUtil `/usr/bin/net-snmp-config --agent-libs`

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS} ${ORACLE_LIB_DIRS} ${ORACLE_LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TABLE		=
TARGET		=SNMPIF

SRCS		=main.c chnTable.c dskTable.c edTable.c ethTable.c
SRCS		+=fanTable.c memTable.c ntpTable.c prTable.c nifoTable.c
SRCS		+=pwrTable.c queTable.c cpuTable.c upresto.c
OBJS		=${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LDFLAGS}

source:
	env MIBS="+UPRESTO-MIB" mib2c $(TABLE)

#subagent:
#	net-snmp-config --compile-subagent $(TARGETS) $(TABLE).c

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
