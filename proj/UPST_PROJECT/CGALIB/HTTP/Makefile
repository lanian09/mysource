
SHELL		= /bin/sh

CC 			= `which gcc`
AR 			= `which ar`
RM 			= `which rm`
CP			= `which cp`
DEPEND		= makedepend
LINT 		= lint
LEX			= flex
YACC		= bison

TOP_BIN 	= 

INCLUDES    = -I./ -I../ -I../../INC -I../../LIB/INC -I../../TAF/INC
DEFINES 	= 
#OPTIMIZE	= -O3 -Wall
ifeq ($(AQUA_DEBUG),)
DEBUG   	= -g3 -Wall
else
DEBUG   	= $(AQUA_DEBUG)
endif

DEBUG		+= -DYY_NO_UNPUT

CFLAGS 		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)
SFLAGS 		= $(INCLUDES) $(DEFINES) $(DEBUG)
DEPOPTS 	= $(INCLUDES) $(DEFINES) 

HDRS 		= 

SRCS 		= httpheader.tab.c lex.httpheader.c	lex.httphdrinfo.c
SRCS		+=lex.httpctype.c lex.httphost.c lex.ssl.c lex.mms_from.c
SRCS		+=lex.mms_to.c lex.mms_from_dn.c lex.mms_to_dn.c lex.a2g_dn.c
SRCS		+=lex.vod_dn.c lex.httpctypeinfo.c

TARGET 		= libHttp.a 

OBJSV 		= $(SRCS:.c=s.o)
OBJCL 		= $(SRCS:.c=c.o)

#
#-----------------------------------------------------------
#

#
.SUFFIXES: .l .y .c .s s.o c.o p.o q.o .i .u .H
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(SFLAGS) -S $<
.cs.o:
	$(CC) -o $@ $(CFLAGS) -c $<
.cc.o:
	$(CC) -o $@ $(CFLAGS) -c $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<

all: $(TARGET) install


$(TARGET): $(SRCS) $(HDRS) $(OBJSV) 
	$(RM) -f $@
	$(AR) clq $@ $(OBJSV)
	$(AR) ts $@ 

install:
	$(CP) ${TARGET} ../../LIB/LIB/

lex.vod_dn.c :
	$(LEX) -Pvod_dn -i vod_dn.l

lex.a2g_dn.c :
	$(LEX) -Pa2g_dn -i a2g_dn.l

lex.mms_from_dn.c :
	$(LEX) -Pmms_from_dn -i mms_from_dn.l

lex.mms_to_dn.c :
	$(LEX) -Pmms_to_dn -i mms_to_dn.l

lex.mms_to.c :
	$(LEX) -Pmms_to -i mms_to.l

lex.mms_from.c :
	$(LEX) -Pmms_from -i mms_from.l

lex.ssl.c :
	$(LEX) -Pssl -i ssl.l

lex.httphost.c :
	$(LEX) -Phttphost -i httphost.l

lex.httpctype.c :
	$(LEX) -Phttpctype -i httpctype.l

lex.httphdrinfo.c :
	$(LEX) -Phttphdrinfo -i httphdrinfo.l

lex.httpctypeinfo.c :
	$(LEX) -Phttpctypeinfo -i httpctypeinfo.l

lex.httpheader.c :
	$(LEX) -Phttpheader -i httpheader.l

httpheader.tab.c :
	$(YACC) -phttpheader -d httpheader.y

clean:
	$(RM) -f *.s *.u *.o *.i *.c *.h core $(TARGET)

depend: $(SRCS)
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

lint:
	$(LINT) $(INCLUDES) -h -v -a -u -x -m $(SRCS)

http:
	$(RM) -f libHttpHeader.a
	$(YACC) -phttpheader -d httpheader.y
	$(LEX)	-Phttpheader httpheader.l
	$(LEX)	-Phttphdrinfo httphdrinfo.l
	$(CC) -c -Wall lex.httpheader.c httpheader.tab.c lex.httphdrinfo.c
	$(AR) clq libHttp.a lex.httpheader.o httpheader.tab.o lex.httphdrinfo.o
	cp libHttp.a ../../../LIB

# DO NOT DELETE THIS LINE -- make depend depends on it.

