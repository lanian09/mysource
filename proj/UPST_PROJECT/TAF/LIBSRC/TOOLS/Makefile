#--------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

CC			=`which cc`
AR			=`which ar`

CP			=`which cp`
RM			=`which rm`

OAM_DIR		= ../../../OAM
LIB_DIR		= ../../../LIB
LIBS		= -L$(LIB_DIR)/LIB

INCLUDES	=-I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
DEFINES		=
OPTIMIZE	=
DEBUGS		=-g3 -Wall

LIBRARY		=-lLOGLIB

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INCLUDES}
LDFLAGS		=${LIBS} ${LIBRARY} 

DEST_DIR	=../../LIB
#-------------------- Source & Objects & Libraries------------------------
TARGET		=libTOOLS.a

SRCS		=
SRCS	   += platform_type.c
#SRCS       += url.c

OBJS		=${SRCS:.c=.o}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${AR} rcu $@ $?

clean:
	${RM} -f ${TARGET} ${OBJS}

install: ${TARGET}
	@for i in ${TARGET}; do \
		echo Installing $$i in ${DEST_DIR}; \
		${CP} $$i ${DEST_DIR}; \
	done

# DO NOT DELETE
