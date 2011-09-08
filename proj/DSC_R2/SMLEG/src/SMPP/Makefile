include ../../MkConfig.mk

CC			= gcc
RM			= rm -f
DEPEND		= makedepend
LINT		= lint
C64FLAGS	= 
#CFLAG		= -g -DINTEL -Wall #-DDEBUG1 #-DMMDB_DEBUG #-Wall -D_REENTRANT #-DALONE_TEST

INC_PATH 	= -I. -I../../include -I../../libsrc/COMM  -I${MYSQL_HOME}/include

LIB_PATH 	= -L../../lib -L${MYSQL_HOME}/lib

DEFINES		= -DDEBUG

CFLAG       =  -g -Wall -m32 $(DEFINES)

LIBS		= -lLogUtil -lIPC -lInitShm -lcomm -lsocket -lnsl -lmysqlclient -lz -lm

MAKEFILE    = Makefile

SRCS		=	cid_handle.c \
				smpp_init.c \
				smpp_log.c \
				smpp_main.c \
				smpp_mmc.c \
				smpp_msgQ.c \
				smpp_proto.c \
				smpp_socket.c \
				smpp_sql.c \
				smpp_util.c \
				smpp_trace.c

OBJS		= $(SRCS:.c=.o)

AOUT		= SMPP

DEST		= ../../bin

all:: $(AOUT) install

$(OBJS): $(@:.o=.c)
	$(CC) $(C64FLAGS) $(CFLAG) $(INC_PATH) -c $(@:.o=.c)

$(AOUT): $(OBJS)
	$(CC) -o $(AOUT) $(C64FLAGS) $(OBJS) $(LIB_PATH) $(LIBS) $(LDFLAGS)

shmmb_print:
	$(CC) -c  $(CFLAG)  $(INC_PATH) shmmb_print.c
	$(CC) -o shmmb_print shmmb_print.o $(OBJS) $(LIB_PATH) $(LIBS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INC_PATH) --$(LIB_PATH) -- $(SRCS)

install:
	\cp $(AOUT) $(DEST)

clean:
	\rm -f $(OBJS) $(AOUT) core
