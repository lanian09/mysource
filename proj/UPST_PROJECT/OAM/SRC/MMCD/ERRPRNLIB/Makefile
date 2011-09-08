#--------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`
LD			=`which ld`

CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

LIB_HOME	=../../../../LIB
INC_DIRS	=-I. -I../../../INC -I../../../../INC -I${LIB_HOME}/INC
OPTIMIZE	=-g3 -fPIC -Wall

LIB_DIRS	=
LIBS		=

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../../BIN
#-------------------- Source & Objects & Libraries------------------------
TARGET		=libsmsprn.so

SRCS		=RET_PRT.c OSTAT_PRT.c ERR_PRT.c
OBJS		=${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${LD} -shared -export-dynamic -o $@ ${OBJS} -lc

clean:
	${RM} -f ${TARGET} ${OBJS}

dep: ${SRCS}
	${CC} -M ${CFLAGS} < $?

install: ${TARGET}
	@for i in ${TARGET}; do \
		echo Installing $$i in ${DEST_DIR}; \
		${CP} $$i ${DEST_DIR}; \
	done

# DO NOT DELETE
