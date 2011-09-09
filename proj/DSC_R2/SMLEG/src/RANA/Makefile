include ../../MkConfig.mk

TOP_BIN	= ../../bin
TMF_BIN	= /tmfapp/BIN
TOP_LIB	= ../../lib
LIBRARY = -L$(TOP_LIB)

INCLUDES	= -I./ -I../../include -I../../libsrc/SM_API/include -I../../libsrc/SM_API/include/system -I../../libsrc/COMM
#INCLUDES = -L../../lib -I../../include -I./  -I../../libsrc/COMM
DEFINES		= -D_REENTRANT
DEFINES		+= #-DPRT_CALL_CNT
DEFINES		+= #-DPRT_SESS_CNT_LV1
DEFINES		+= #-DPRT_STATISTIC
DEFINES  	+= #-DPRINT -D__DEBUG -DPRINT

CFLAGS		= -g -Wall $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
#SRCS		= rana_init.c rana_main.c rana_msgq.c rana_session.c rana_mmc_hld.c rana_mmc_proc.c rana_tmo.c 
SRCS		= rana_init.c rana_main.c rana_msgq.c rana_session.c rana_mmc_proc.c rana_tmo.c 

TARGET		= RANA

OBJS		= $(SRCS:.c=.o)

LIB_PATH_ORACLE = 

#LIBS    	= -lANZ_LIB -lLogUtil -lInitShm -lcomm -lMMDBDESTIP -lMMDBDESTPORT -lIPC -lSHMUTIL -lMIF
LIBS    	= -lANZ_LIB -lLogUtil -lInitShm -lcomm -lHASHO -ltimerN -lhashg -lIPC -lSHMUTIL -lnifo -lsocket  -lnsl

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
	/bin/rm -f *.o $(TARGET) core* 

new : clean $(SRCS)

