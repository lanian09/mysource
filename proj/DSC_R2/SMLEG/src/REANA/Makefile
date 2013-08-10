include ../../MkConfig.mk

LEX 	= flex
TOP_BIN	= ../../bin
TMF_BIN	= /tmfapp/BIN
TOP_LIB	= ../../lib
LIBRARY = -L$(TOP_LIB)

INCLUDES = -L../../lib -I../../include -I./  -I../../libsrc/COMM
DEFINES  = -DPRINT -DYY_NO_UNPUT -DHEXADUMP

CFLAGS		= -g $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
SRCS		= re_init.c re_main.c re_msgq.c make_nidtid.c lex.parsedata.c

TARGET		= REANA

OBJS		= $(SRCS:.c=.o)

LIB_PATH_ORACLE = 

#LIBS    	= -lANZ_LIB -lLogUtil -lInitShm -lcomm -lMMDBDESTIP -lMMDBDESTPORT -lIPC -lSHMUTIL -lMIF
LIBS    	= -lANZ_LIB -lLogUtil -lInitShm -lcomm -lIPC -lSHMUTIL -lnifo -lsocket  -lnsl

MAKEFILE	= Makefile

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


all: $(TARGET) 

lex.parsedata.c:
	$(LEX) -P parsedata -i re_lex.l

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBRARY) $(LIBS) $(LDFLAGS)
	cp $(TARGET) $(TOP_BIN)
#	cp $(TARGET) $(TMF_BIN)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(TMF_BIN)

clean:
	/bin/rm -f *.o $(TARGET) core* lex.parsedata.c

new : clean $(SRCS)

