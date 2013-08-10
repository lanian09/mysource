include ../../MkConfig.mk

TOP_BIN		= ../../bin
TOP_LIB		= ../../lib
LIBRARY 	=   -L$(TOP_LIB)

INCLUDES 	= -L../../lib -I../../include -I./  -I../../libsrc/COMM

# -D_REANA_	: TCP 8080에 대해 REANA로 보내는 루틴.
DEFINES  	+= -DPRINT -DBUFFERING
#DEFINES  	+= -DPRINT 
#DEFINES  	+= -DPRINT 

CFLAGS		= $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
SRCS		= pana_main.c  pana_init.c pana_msgq.c 

TARGET		= PANA

OBJS		= $(SRCS:.c=.o)

#LIBS    	= -lANZ_LIB -lIPC -lSHMUTIL -lLogUtil -lInitShm -lMMDBDESTIP -lMMDBDESTPORT -lMIF -lcomm -lIPPOOLBIT -lHASHO
LIBS    	= -lANZ_LIB -lIPC -lSHMUTIL -lLogUtil -lInitShm -lnifo -lcomm -lHASHO -lsocket  -lnsl

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

#$(SRCS):
#	$(GET) -s $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LIBRARY) $(LDFLAGS)
	${CP} $(TARGET) $(TOP_BIN)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	${CP} $(TARGET) $(TOP_BIN)

clean:
	${RM} *.o $(TARGET) core* 

new : clean $(SRCS)

