
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

#DEFINES		+= -DYY_NO_UNPUT

HDRS 		= 
SRCS 		= lex.sip_trans_key.c lex.sip.c lex.sip_clientip.c
SRCS		+=lex.sipheader.c lex.sip_contentlen.c lex.sip_model.c
SRCS		+=lex.sip_min.c lex.sip_invite.c lex.sip_ctype.c lex.sip_service.c
TARGET 		= libSIP.a 

OBJSV 	= $(SRCS:.c=s.o)
OBJCL 	= $(SRCS:.c=c.o)

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

lex.sip_ctype.c :
	$(LEX) -Psip_ctype -i sip_ctype.l

lex.sip_service.c :
	$(LEX) -Psip_service -i sip_service.l

lex.sip_invite.c :
	$(LEX) -Psip_invite -i sip_invite.l

lex.sip_min.c :
	$(LEX) -Psip_min -i sip_min.l

lex.sip_model.c :
	$(LEX) -Psip_model -i sip_model.l

lex.sip_trans_key.c :
	$(LEX) -Psip_trans_key -i sip_trans_key.l

lex.sip.c :
	$(LEX) -Psip -i sip.l

lex.sip_clientip.c :
	$(LEX) -Psip_clientip -i sip_clientip.l

lex.sipheader.c :
	$(LEX) -Psipheader -i sipheader.l

lex.sip_contentlen.c :
	$(LEX) -Psip_contentlen -i sip_contentlen.l

clean:
	$(RM) -f *.s *.u *.o *.i *.c *.h core $(TARGET)

depend: $(SRCS)
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

lint:
	$(LINT) $(INCLUDES) -h -v -a -u -x -m $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

