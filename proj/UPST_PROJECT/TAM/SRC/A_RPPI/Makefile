# -------------------- Makefile which have no SubDir ---------------------
SHELL		= /bin/sh

CC			= `which cc`
AR			= `which ar`
CP			= `which cp`
MV			= `which mv`
RM			= `which rm`

BIN_DIR 	= ../../BIN
LIB_DIR 	= ../../../LIB
OAM_DIR 	= ../../../OAM

LIBS    	= -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER	= $(shell uname -r)
#KERNEL_DIR = /lib/modules/$(KERNEL_VER)/source/include

OPTIMIZE	=
INCLUDES	= -I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
OPTIMIZE	= -g3 -Wall

DEFINES 	=
DEFINES	   += -D_ACCESS_FILTER_ 			# TAF 재배치로 인한 중복된 ACCESS MSG filtering
DEFINES	   += -D_RPPI_MULTI_				# Multi RPPI
DEFINES	   += -DDISABLE_ANALYZE_DIAMETER  # CSCR-HSS 구간 분석결과인 DIAMETER LOG를 별도 생성하기로 결정하면서, A_RPPI 에서 rppi 에 별도로 남기던 로직은 삭제됨. 일단, DISABLE. by uamyd 20101014


#ORACLE_LIB_DIRS	=-L${ORACLE_HOME}/lib
#ORACLE_LIBS		=-lclntsh


CFLAGS		= ${OPTIMIZE} ${DEFINES} ${INCLUDES}

#-------------------- Source & Objects & Libraries------------------------
#LIBRARY		=-lShmQueue -lLogUtil -lShmUtil -lhasho -lmemg -lnifo -lmems -ltimerN -lhashg -lSTGtimerN -lSTGhashg -lSTGhasho -lSTGmemg -lSTGnifo -lSTGmems -lSTGcommon_stg
LIBRARY		= -lLOGLIB -lVERLIB -lIPCLIB -lUTILLIB
LIBRARY	   += -lnifo -lgifo -lcifo -ltimerN -lhasho

LDFLAGS		= ${LIBS} ${LIBRARY} ${ORACLE_LIB_DIRS} ${ORACLE_LIBS}

TARGET		= A_RPPI

SRCS		=
SRCS	   += rppi_main.c 
SRCS	   += rppi_init.c 
SRCS	   += rppi_switch.c 
SRCS	   += rppi_func.c 
SRCS	   += rppi_msgq.c 
SRCS	   += rppi_util.c

OBJS		= ${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LDFLAGS}

dep: ${SRCS}
	${CC} -M ${CFLAGS} < $?

clean:
	rm -f ${TARGET} ${OBJS} core*

install:
	@for i in ${TARGET}; do \
		echo Installing $$i in ${BIN_DIR}; \
		${CP} $$i ${BIN_DIR}; \
	done

# DO NOT DELETE
