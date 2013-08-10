
SHELL 	= /bin/sh

CC 	= gcc
AR 	= ar 
RM 	= /bin/rm
DEPEND	= makedepend
LINT 	= lint

INCLUDES=  -I../../INC -I.
DEBUG   = -g3 -Wall 

CFLAGS 	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)
SFLAGS 	= $(INCLUDES) $(DEFINES) $(DEBUG)
DEPOPTS = $(INCLUDES) $(DEFINES) 

HDRS 	= 

LDFLAGS = -L../../../LIB/LIB -lLOGLIB
SRCS 	= Analyze_Inf11.c analyze_PPP.c VJ.c

TARGET 	= libANZ_LIB.a 

OBJS 	= $(SRCS:.c=.o)

#
#-----------------------------------------------------------
#

#
.SUFFIXES: .c .s s.o c.o p.o q.o .i .u .H
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(SFLAGS) -S $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<

all: $(TARGET) $(HADS)


libANZ_LIB.a: $(SRCS) $(HDRS) $(OBJS) 
	$(RM) -f $@
	$(AR) clq $@ $(OBJS)
	$(AR) ts $@ 
	cp $@ ../../LIB

clean:
	$(RM) -f *.s *.u *.o *.i core $(TARGET)

depend: $(SRCS)
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

lint:
	$(LINT) $(INCLUDES) -h -v -a -u -x -m $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

