include ../../MkConfig.mk

TOP_BIN = ../../bin

INCLUDES= -L../../lib -L/usr/loca/lib -I../../include -I. -I/usr/local/include/pcap -I../../libsrc/COMM

#DEFINES = -DPRINT_TRAFFIC -DBUFFERING
DEFINES = -DBUFFERING #-DPKT_MON_PRT 
#DEFINES = -DLOGDEBUG 

#CFLAGS  = $(COMMON_FLAGS) $(INCLUDES) $(DEFINES) -DPKT_MON_PRT
CFLAGS  = $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)

SRCS	= capd_main.c capd_init.c capd_msgq.c capd_dlpi.c capd_test.c conf.c

TARGET  = CAPD

OBJS    = $(SRCS:.c=.o)
LIBS	= -lInitShm -lLogUtil -lSHMUTIL -lIPC -lnifo -lcomm -lpcap -lsocket  -lnsl

MAKEFILE= Makefile

#
#-----------------------------------------------------------
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
