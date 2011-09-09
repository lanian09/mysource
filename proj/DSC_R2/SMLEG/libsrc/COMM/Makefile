include ../../MkConfig.mk

CC		=gcc
RM		=rm -f
CP      = /usr/bin/cp -f
DEPEND	=/usr/openwin/bin/makedepend
LINT	=lint
CFLAG	=-g3 -Wall -O2 #-DTRC_TIMER
DEFINES	=-DDEBUG #-DTRU64 #-DRESTRICTED_CLIENT #-DSOREUSEPORT #-DTCPNODELAY

TOP_LIB = ../../lib
INC_DIR	=-I. -I../../include
LIB_DIR	=-L../../lib
LIBS	=
CFLAGS	=$(COMMON_FLAGS) $(CFLAG) $(DEFINES) $(INC_DIR)
#CFLAGS	=$(CFLAG) $(DEFINES) $(INC_DIR)

SRCS	=conflib.c \
			loglib.c \
			socklib.c \
			keepalivelib.c \
			trclib.c \
			comm_util.c \
			comm_proc.c \
	    	check_run_myproc.c \
			strutil.c \
			ping.c


#			proc_version.c \
#yhshin HZ¹®Á¦.            comm_proc.c

OBJS	= $(SRCS:.c=.o)

AOUT	= libcomm.a

#
#---------------------------------------------------------------------
#
all:: $(AOUT) install

$(AOUT): $(OBJS)
	ar rcus $@ $(OBJS)
	${CP} $(AOUT) $(TOP_LIB)
	${RM} *.o

install:
	${CP} $(AOUT) $(TOP_LIB)

dep: ${SRCS}
	${DEPEND} -o.o -f Makefile -- ${INC_DIRS} -- ${SRCS}

clean:
	rm -f $(OBJS) $(AOUT) core
