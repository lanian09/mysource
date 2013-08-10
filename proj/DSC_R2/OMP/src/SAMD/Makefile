SHELL		= /bin/sh

LEX     = flex

CC			= cc
GCC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
#CFLAG		= -g -DDEBUG_DUP -DDEBUG
#CFLAG		= -g -Wall -DDEBUG -D_GET_SNMP_TRAP_
CFLAG		= -g -Wall -D_GET_SNMP_TRAP_
#CFLAG		= -g -Wall -DTAP_TEST #-DDEBUG
CFLAG_TRU64	= -g -DTRU64

INC_PATH	=  -I./ -I../ -I../../include -I../../libsrc/COMM \
 		   -I/usr/local/include/net-snmp \
 		   -I/usr/sfw/include/ \
		   -I/usr/sfw/include/net-snmp \
		   -I/usr/sfw/include/net-snmp/library

LIB_PATH	= -L../../lib -L/usr/local/lib -L/usr/lib -L/usr/local/ssl/lib

LIBS 		= -lkstat -lcomm -lnsl -lsocket -ladm -lelf -lcrypto -lpthread -lnetsnmp -lgen -lm  -ladm
LIBS_TRU64	= -lcomm -lmach

SRCS		=	samd_init.c \
				samd_main.c \
				samd_qclear.c \
				samd_ping.c \
				samd_stat.c \
				samd_rxtxmsg.c \
				samd_proc.c \
				samd_disk.c \
				samd_sys.c \
				samd_que.c \
				samd_tru64_disk.c \
				samd_tru64_sys.c \
				samd_ntp.c \
				samd_util.c \
				samd_mmc_proc.c


OBJS		= $(SRCS:.c=.o)

AOUT		= samd

DEST		= ../../bin

all:: $(AOUT) $(AOUT2) install


lex.parsedata.c:
	$(LEX) -P parsedirector -i re_lex.l

#$(OBJS): $(@:.o=.c)
.SUFFIXES: .c .o
.c.o:
#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) $(CFLAG_TRU64) $(INC_PATH) -c $< ; \
#	else \
#		$(GCC) $(CFLAG) $(INC_PATH) -c $< ; \
#	fi

	$(GCC) $(CFLAG) $(INC_PATH) -c $< 

$(AOUT): $(OBJS)
#	if [ `uname` = "OSF1" ] ; then \
#		$(CC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS_TRU64) ; \
#	else \
#		$(GCC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS) ; \
#	fi
	$(GCC) -o $(AOUT) $(OBJS) $(LIB_PATH) $(LIBS) 

samd_snmp: samd_snmp.c
	$(GCC) -g $(INC_PATH) $(LIB_PATH) -o $@ $? $(LIBS)

install:
	cp $(AOUT) $(DEST)

clean:
	rm -f $(OBJS) $(AOUT) core
