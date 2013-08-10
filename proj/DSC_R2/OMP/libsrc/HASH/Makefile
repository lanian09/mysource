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
#LDFLAGS = /export/dsc/work/LGT_DSC/SMLEG/libsrc/MISALIGN/misalign.o

OPTIMIZE = -g3
#OPTIMIZE = -O

#MACHINE = -m64
MACHINE =
MACHINE = -m32         # for 32-bit App in 64-bit OS

#DEFINES = -DBUFFERING   # Turn on buffering message queue for CAPD, PANA
DEFINES = -Wall 

#COMMON_FLAGS = $(MACHINE) $(OPTIMIZE) -Wall -DSMALL_SESSION -DHASHO_TEST
COMMON_FLAGS = $(MACHINE) $(OPTIMIZE) -Wall -DSMALL_SESSION 





TOP_LIB	= ../../lib
LIBRARY = -L../

INCLUDES= -I. -I../../include
DEFINES =

TARGET	= hasho
TARGETLIB	= libHASHO.a

CFLAGS	= $(COMMON_FLAGS) $(INCLUDES) $(DEFINES) $(DEBUG)
SRCS	= hasho.c memg.c debug_func.c

OBJS	= $(SRCS:.c=.o)

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


all: $(TARGETLIB)
	$(RM) *.o

test: $(TARGET)

#$(SRCS):
#	$(GET) -s $@

$(TARGETLIB) : $(OBJS)
	$(RM) -f $@
	$(AR) clq $@ $(OBJS)
	$(AR) ts $@
	$(MV) *.a $(TOP_LIB)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) 

doc:
	doxygen

clean:
	/bin/rm -rf *.o $(TARGET) $(TARGET2) core* TEST_RESULT*.TXT MEMG

# DO NOT DELETE THIS LINE -- make depend depends on it.
