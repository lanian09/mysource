# -------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`
MAKEDEPEND	=`which makedepend`

CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

LIB_HOME    = ../../../LIB
OMP_HOME    = ../..
PRJ_HOME    = ../../..
INC_DIRS    =-I. -I${OMP_HOME}/INC -I${PRJ_HOME}/INC -I${LIB_HOME}/INC -I../../../TAM/INC
DEFINES		=

OPTIMIZE	=-g3 -Wall

LIB_DIRS    =-L../../LIB -L${LIB_HOME}/LIB
LIBS		=-lIPCLIB -lFILELIB -lUTILLIB -lVERLIB 
LIBS		+=-lnifo -lgifo -lcifo

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET_STARTMC	=StartMC
TARGET_KILLMC	=KillMC
TARGET_DISMC	=DisMC

TARGET			=${TARGET_STARTMC} ${TARGET_KILLMC} ${TARGET_DISMC}

SRCS_COMMON		=tifb_util.c process_info.c

SRCS_STARTMC	=${SRCS_COMMON} StartMC.c
SRCS_KILLMC		=${SRCS_COMMON} KillMC.c
SRCS_DISMC		=${SRCS_COMMON} DisMC.c

OBJS_STARTMC	=${SRCS_STARTMC:.c=.o}
OBJS_KILLMC		=${SRCS_KILLMC:.c=.o}
OBJS_DISMC		=${SRCS_DISMC:.c=.o}

# following definitions only For makedepend utility
SRCS			=${SRCS_COMMON} StartMC.c KillMC.c DisMC.c
OBJS			=${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET_STARTMC}: ${OBJS_STARTMC}
	${CC} ${CFLAGS} -o $@ ${OBJS_STARTMC} ${LDFLAGS}

${TARGET_KILLMC}: ${OBJS_KILLMC}
	${CC} ${CFLAGS} -o $@ ${OBJS_KILLMC} ${LDFLAGS}

${TARGET_DISMC}: ${OBJS_DISMC}
	${CC} ${CFLAGS} -o $@ ${OBJS_DISMC} ${LDFLAGS}

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
