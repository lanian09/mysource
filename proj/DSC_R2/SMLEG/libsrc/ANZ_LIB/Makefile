include ../../MkConfig.mk

#SHELL = /bin/sh
#CC = gcc
#AR = ar 
#RM = /bin/rm
#DEPEND=makedepend
#LINT = lint
CP  = /usr/bin/cp -f

TOP_BIN = 
TOP_LIB = ../../lib

INCLUDES =  -I../../include -I./ 
DEFINES  = 

CFLAGS = $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
SFLAGS = $(INCLUDES) $(DEFINES) $(COMMON_FLAGS)
DEPOPTS = $(INCLUDES) $(DEFINES) 

HDRS = 

SRCS = Analyze_Inf.c VJ.c Analyze_Inf_Wap.c analyze_SIP.c analyze_SDP.c analyze_TSIP.c analyze_TSIP_UTIL.c \
		analyze_TSIP_DIC.c

TARGET = libANZ_LIB.a 

OBJSV = $(SRCS:.c=s.o)
OBJCL = $(SRCS:.c=c.o)

#
#-----------------------------------------------------------
#

#
.SUFFIXES: .c .s s.o c.o p.o q.o .i .u .H
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(SFLAGS) -S $<
.cs.o:
	$(CC) -o $@ $(CFLAGS) -DTCP_SERV -c $<
.cc.o:
	$(CC) -o $@ $(CFLAGS) -DTCP_CLI -c $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<

all: $(TARGET) $(HADS)


$(TARGET): $(SRCS) $(HDRS) $(OBJSV) 
	$(RM) -f $@
	$(AR) clq $@ $(OBJSV)
	$(AR) ts $@ 
	${CP} $@ ${TOP_LIB}
	$(RM) *.o

clean:
	$(RM) -f *.s *.u *.o *.i core $(TARGET)

depend: $(SRCS)
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

