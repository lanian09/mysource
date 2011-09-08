
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
SRCS 		= lex.rtsp_reqhdr.c lex.rtsp_reshdr.c lex.rtsp_resbody.c lex.rtsp_requrl.c
TARGET 		= libRtsp.a 

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

all: $(TARGET) $(HADS)


$(TARGET): $(SRCS) $(HDRS) $(OBJSV) 
	$(RM) -f $@
	$(AR) clq $@ $(OBJSV)
	$(AR) ts $@ 
	$(CP) $@ ../../LIB/LIB/

lex.rtsp_requrl.c :
	$(LEX) -Prtsp_requrl -i rtsp_requrl.l

lex.rtsp_reqhdr.c :
	$(LEX) -Prtsp_reqhdr -i rtsp_reqhdr.l

lex.rtsp_reshdr.c :
	$(LEX) -Prtsp_reshdr -i rtsp_reshdr.l

lex.rtsp_resbody.c :
	$(LEX) -Prtsp_resbody -i rtsp_resbody.l

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
	cp libHttp.a ../../LIB

# DO NOT DELETE THIS LINE -- make depend depends on it.

