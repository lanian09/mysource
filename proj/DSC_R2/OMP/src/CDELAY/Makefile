#include ../../MkConfig.mk
SHELL   = /bin/sh
CC      = gcc
AR      = ar
RM      = rm -f
CP      = cp -f
MV      = mv -f
DEPEND  = makedepend
MAKE    = /usr/local/bin/make
MAKEFILE    = Makefile
#LDFLAGS    = /home2/yhshin/work/lgtdsc/smleg/libsrc/MISALIGN/misalign_64.o
#LDFLAGS = /home/dsc/WORK/LGT_DSC/SMLEG/libsrc/MISALIGN/misalign.o

OPTIMIZE = -g3
#OPTIMIZE = -O

#MACHINE = -m64
MACHINE =
MACHINE = -m32         # for 32-bit App in 64-bit OS

#DEFINES = -DBUFFERING   # Turn on buffering message queue for CAPD, PANA
DEFINES = -Wall

COMMON_FLAGS = $(MACHINE) $(OPTIMIZE) -Wall -DSMALL_SESSION




TOP_BIN = ../../bin

INCLUDES= -I. -I../../include -I../../libsrc/COMM  -I/usr/local/include/pcap -I../../libsrc/HASH/ -I${MYSQL_HOME}/include
LIBINC= -L../../lib -L/usr/loca/lib  -L${MYSQL_HOME}/lib

#DEFINES	= -g3 -Wall -DFILTER -DTEST
DEFINES	= -g3 -Wall -DFILTER

#CFLAGS  = $(COMMON_FLAGS) $(LIBINC) $(INCLUDES) $(DEFINES)
CFLAGS  = $(LIBINC) $(INCLUDES) $(DEFINES)
#SRCS	= capd.c capd_init.c capd_msgq.c capd_test.c pcapfunc.c conf.c
SRCS	= main.c mmc_hld.c mmc_proc.c

TARGET  = cdelay

OBJS    = $(SRCS:.c=.o)
#LIBS	= -lInitShm -lLogUtil -lSHMUTIL -lIPC -lMIF -lcomm -lpcap -lsocket  -lnsl
LIBS	= -lcomm -lpcap -lsocket  -lmysqlclient -lz -lm -lnsl -lHASHO

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
