# -------------------- Makefile which have no SubDir ---------------------
SHELL       = /bin/sh

CC          = `which cc`
AR          = `which ar`
CP          = `which cp`
MV          = `which mv`
RM          = `which rm`

BIN_DIR     = ../../BIN
LIB_DIR     = ../../../LIB
OAM_DIR     = ../../../OAM

LIBS        = -L../../LIB -L$(LIB_DIR)/LIB

#KERNEL_VER = $(shell uname -r)
#KERNEL_DIR = /lib/modules/$(KERNEL_VER)/source/include

OPTIMIZE    =
INCLUDES    = -I. -I../../INC -I../../../INC -I$(OAM_DIR)/INC -I$(LIB_DIR)/INC
OPTIMIZE    = -g3 -Wall

DEFINES     =
DEFINES    += -D_RPPI_MULTI_                # Multi RPPI


CFLAGS      = ${OPTIMIZE} ${DEFINES} ${INCLUDES}

#-------------------- Source & Objects & Libraries------------------------
#LIBS    = -lnsl -lShmQueue -lLogUtil -lNSock -lShmUtil -lnifo -lmems -lSTGnifo -lSTGmems -lSTGcommon_stg
LIBRARY		= -lLOGLIB -lVERLIB -lFILELIB -lIPCLIB -lNSOCKLIB
LIBRARY	   += -lnifo -lgifo -lcifo -lSTGcommon_stg

LDFLAGS		= ${LIBS} ${LIBRARY}

TARGET		= SI_LOG 

SRCS		= 
SRCS	   += silog_main.c 
SRCS	   += silog_init.c 
SRCS	   += silog_sock.c 
SRCS	   += silog_func.c

OBJS		= $(SRCS:.c=.o)

#
#-----------------------------------------------------------
#

#
.SUFFIXES: .c .s .o .i .u
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(SFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<



all: $(TARGET) install

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

install :
	cp $(TARGET) $(BIN_DIR)

dep: $(SRCS)
	${CC} -M ${CFLAGS} < $?

clean:
	/bin/rm -f *.o $(TARGET) core* 

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
