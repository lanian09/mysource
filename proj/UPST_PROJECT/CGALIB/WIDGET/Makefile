
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

SRCS 		= lex.widgetmsgtype.c lex.widgetack.c
TARGET 		= libWIDGET.a
TEST		= widgetmsgtype

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

all: $(TARGET)

test: $(TEST)

$(TARGET): $(SRCS) $(OBJSV) 
	$(RM) -f $@
	$(AR) clq $@ $(OBJSV)
	$(AR) ts $@ 
	$(CP) $@ ../../LIB/LIB/

$(TEST): $(SRCS)
	$(CC) -o widgetmsgtype lex.widgetmsgtype.c $(CFLAGS) -DTEST

lex.widgetack.c :
	$(LEX) -Pwidgetack -i widgetack.l

lex.widgetmsgtype.c :
	$(LEX) -Pwidgetmsgtype -i widgetmsgtype.l

clean:
	$(RM) -f *.c *.s *.u *.o *.i *.c *.h core $@ $(TARGET) $(TEST)

depend: $(SRCS)
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

lint:
	$(LINT) $(INCLUDES) -h -v -a -u -x -m $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

