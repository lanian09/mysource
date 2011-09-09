include ../../MkConfig.mk
TOP_BIN = ../../bin

INCLUDES= -L../../lib -L/usr/local/lib -L${MYSQL_HOME}/lib -I../../include -I../../libsrc/COMM -I. -I${MYSQL_HOME}/include  -I../../libsrc/COMM 
#DEFINES        = -g3 -Wall -DFILTER
#DEFINES        = -g -Wall -DFILTER -DTEST
DEFINES	= -g -Wall -DFILTER 

#CFLAGS  = $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
CFLAGS  = $(INCLUDES) $(DEFINES)
#SRCS	= capd.c capd_init.c capd_msgq.c pcapfunc.c conf.c capd_mml.c
#SRCS	= main.c capd_init.c ana_msgq.c capd_msgq.c conf.c capd_mml.c rdrana_sql.c  rdr_session.c
SRCS	= main.c capd_init.c conf.c capd_mml.c rdrana_sql.c  rdr_session.c

TARGET  = RDRANA

OBJS    = $(SRCS:.c=.o)
#LIBS	= -lANZ_LIB -lIPC -lSHMUTIL -lLogUtil -lInitShm -lMIF -lcomm -lIPPOOLBIT -lHASHO -ltimerN -lhashg  -lSTGtimerN -lSTGhashg -lsocket  -lnsl -lmysqlclient -lz -lm 
LIBS	= -lANZ_LIB -lIPC -lSHMUTIL -lLogUtil -lInitShm -lcomm -lIPPOOLBIT -lHASHO -ltimerN -lhashg  -lsocket  -lnsl -lmysqlclient -lz -lm 

MAKEFILE= Makefile

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

install:
	cp $(TARGET) $(TOP_BIN)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBRARY) $(LIBS) $(LDFLAGS)

clean:
	/bin/rm -f *.o $(TARGET) core*

# DO NOT DELETE THIS LINE -- make depend depends on it.
