# -------------------- Makefile which have no SubDir ---------------------
SHELL		= /bin/sh

MAKE		= `which make`
CC			= `which cc`
AR			= `which ar`
CP			= `which cp`
MV			= `which mv`
RM			= `which rm`

BIN_DIR		= ../../BIN
LIB_DIR		= ../../../LIB
OAM_DIR		= ../../../OAM

LIBS		= -L./ -L../../LIB -L${LIB_DIR}/LIB
LIBS	   += -L/usr/lib64/mysql

#KERNEL_VER = $(shell uname -r)
#KERNEL_DIR = /lib/modules/$(KERNEL_VER)/source/include
#GCC_DIR    = /usr/lib/gcc/x86_64-redhat-linux/4.1.2/include   

OPTIMIZE    = -g3 -Wall
#OPTIMIZE   += -fno-strict-aliasing

INCLUDES    = -I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
#INCLUDES   += -I$(KERNEL_DIR) -I$(GCC_DIR)

CFLAGS		= ${OPTIMIZE} ${DEFINES} ${INCLUDES}

#-------------------- Source & Objects & Libraries------------------------
TARGET		= S_MNG

SRCS		=
SRCS	   += s_mng_func.c
SRCS	   += s_mng_flt.c 
SRCS	   += s_mng_msg.c 
SRCS	   += s_mng_util.c 
SRCS	   += s_mng_mon.c 
SRCS	   += s_mng_dft.c
SRCS	   += s_mng_init.c 
SRCS	   += s_mng_mmc.c 
SRCS	   += s_mng_mmc2.c 
SRCS	   += s_mng_main.c

OBJS		= ${SRCS:.c=.o}

#LIBS		= -lMySQLDB -lShmUtil -lLogUtil -lShmQueue -lmysqlclient -lnifo -lSTGnifo -lhasho -lmemg -lSTGhasho -lmems -lSTGmems -lSTGmemg -lm -lz
LIBRARY		= -lLOGLIB -lVERLIB -lDBLIB -lIPCLIB -lFILELIB -lUTILLIB
LIBRARY    += -lnifo -lgifo -lcifo -lhasho
LIBRARY	   += -lmysqlclient	-lSQLSET -lTOOLS -lm -lz

LDFLAGS		=${LIBS} ${LIBRARY}
#-------------------------------------------------------------------------
.SUFFIXES: .c .o

#all2:
#	@echo " >>> NOT COMPLETE"

.c.o:
	${CC} ${CFLAGS} -c $<

all: lib ${TARGET} install

${TARGET}: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} ${LDFLAGS}

lib :
	${MAKE} -C SQLSET

dep: ${SRCS}
	${CC} -M ${CFLAGS} -c $?

clean:
	${MAKE} -C SQLSET $@
	rm -f ${TARGET} ${OBJS} core* *.a

install:
	@for i in ${TARGET}; do \
		echo Installing $$i in ${BIN_DIR}; \
		${CP} $$i ${BIN_DIR}; \
	done

# DO NOT DELETE
