#--------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`
CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

BIN_DIR		=../
LIB_DIR		=../../../../LIB
OAM_DIR		=../../../../OAM
INC_DIRS	=-I. -I../ -I../../../INC -I../../../../INC  -I$(LIB_DIR)/INC -I${OAM_DIR}/INC
#INC_DIRS	+=-I/usr/lib/gcc/i486-linux-gnu/3.4.6/include
DEFINES		=-D_GNU_SOURCE -DAPPLOG
#DEFINES		+=-DMYSQL_RECONNECT
DEFINES		+=-DMYSQLCHARSET_EUCKR
OPTIMIZE	=-g3 -Wall

LIBS		=-L${LIB_DIR}/LIB
LIBRARY		=-lmysqlclient -lLOGLIB

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIBS} ${LIBRARY}

#-------------------- Source & Objects & Libraries------------------------
TARGET		=libSQLSET.a

SRCS		=db_api.c

OBJS		=${SRCS:.c=.o}
HDRS		=db_api.h
#-------------------------------------------------------------------------
.SUFFIXES: .o .c

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${AR} rcu $@ ${OBJS}

clean:
	${RM} -f ${TARGET} ${OBJS}

dep: ${SRCS}
	${CC} -M ${CFLAGS} < $?

install: ${TARGET}
	@for i in ${TARGET}; do \
		echo Installing $$i in ${BIN_DIR}; \
		${CP} $$i ${BIN_DIR}; \
	done
	@echo Installing ${HDRS} in ${BIN_DIR};
	${CP} ${HDRS} ${BIN_DIR};

# DO NOT DELETE
