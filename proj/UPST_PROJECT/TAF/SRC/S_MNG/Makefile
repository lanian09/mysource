# -------------------- Makefile which have no SubDir ---------------------
SHELL		=/bin/sh

MAKE		=`which make`
CC			=`which cc`
AR			=`which ar`
MAKEDEPEND	=`which makedepend`
MAKEFILE	=Makefile

CP			=`which cp`
MV			=`which mv`
RM			=`which rm`

BIN_DIR 	= ../../BIN
LIB_DIR 	= ../../../LIB
OAM_DIR 	= ../../../OAM

LIBS    	= -L../../LIB -L$(LIB_DIR)/LIB
LIBS	   += -L/usr/lib64/mysql

#KERNEL_VER	= $(shell uname -r)
#KERNEL_DIR	= /lib/modules/$(KERNEL_VER)/source/include
#GCC_DIR 	= /usr/lib/gcc/x86_64-redhat-linux/4.1.2/include   

OPTIMIZE	=-g3 -Wall
#OPTIMIZE	+=-fno-strict-aliasing

INCLUDES	=-I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
#INCLUDES   += -I$(KERNEL_DIR) -I$(GCC_DIR)

DEFINES		=

ifeq ($(SYSTYPE), RP)
DEFINES    += -DRP
endif

ifeq ($(SYSTYPE), PI)
DEFINES    += -DPI -DREGENERATE_FLT
 
endif

ifeq ($(SYSTYPE), PIOLD)
DEFINES += -DPIOLD
endif

CFLAGS		=${OPTIMIZE} ${DEFINES} ${INCLUDES}


#-------------------- Source & Objects & Libraries------------------------
TARGET		=S_MNG

SRCS		=
SRCS	   += fltmng_func.c
SRCS	   += fltmng_file.c
SRCS	   += fltmng_log.c
SRCS	   += fltmng_init.c
SRCS	   += fltmng_main.c

OBJS		=${SRCS:.c=.o}

#LIBRARY		=-lInitShm -lLogUtil -lSHMUTIL -lnifo -lSTGnifo -lmems -lSTGmems -lmysqlclient
LIBRARY		= -lLOGLIB -lVERLIB -lUTILLIB -lFILELIB -lIPCLIB -lDBLIB
LIBRARY	   += -lnifo -lgifo -lcifo
LIBRARY    += -lmysqlclient

LDFLAGS		=${LIBS} ${LIBRARY}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

.c.o:
	${CC} ${CFLAGS} -c $<

all: ${TARGET} install

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LDFLAGS}

dep: ${SRCS}
	${CC} -M ${CFLAGS} -c $?
	#${MAKEDEPEND} -o.o -f ${MAKEFILE} -- ${DEFINES} ${INCLUDES} -- $?

clean:
	${RM} -f ${TARGET} ${OBJS} core*

install:
	@for i in ${TARGET}; do \
		echo Installing $$i in ${BIN_DIR}; \
		${CP} $$i ${BIN_DIR}; \
	done

# DO NOT DELETE
