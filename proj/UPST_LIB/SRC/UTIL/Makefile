#--------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`
MAKEFILE	=`which make`

CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

INC_DIRS	=-I../../INC -I.
DEFINES		=
OPTIMIZE	=-g3 -Wall -fPIC

LIB_DIRS	=
LIBS		=

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INC_DIRS}
LDFLAGS		=${LIB_DIRS} ${LIBS}

DEST_DIR	=../../LIB
DEST_INC	=../../INC
#-------------------- Source & Objects & Libraries------------------------
TARGET		=libUTILLIB.a

SRCS		=utillib.c

OBJS		=${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${AR} rcu $@ ${OBJS}

clean:
	${RM} -f ${TARGET} ${OBJS}

dep: ${SRCS}
	${CC} -M ${CFLAGS} -c $?
	
install: ${TARGET}
	@for i in ${TARGET}; do \
		echo Installing $$i in ${DEST_DIR}; \
		${CP} $$i ${DEST_DIR}; \
	done

# DO NOT DELETE
