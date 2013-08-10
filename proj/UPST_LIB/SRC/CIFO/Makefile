#--------------------- Makefile which have no SubDir ---------------------
SHELL  		= /bin/sh
SRCDIR 		= .
DSTDIR 		= ../../LIB/

CC = cc

TARGET		= all

SUBDIRS		= 

MAKEDEPEND 	= makedepend

STD_INCDIR 	= 
STD_LIBDIR	=

LOC_INCDIR	= -I../../INC
LOC_LIBDIR	=

INC_DIRS	= $(LOC_INCDIR)
LIB_DIRS	= -L../../LIB/

#-------------------- Source & Objects & Libraries------------------------
PROGRAMS 	= libcifo.a
#LIBS 		= -lnifo

ifeq ($(AQUA_DEBUG),)
DEBUG   = -g3 -Wall
else
DEBUG   = $(AQUA_DEBUG)
endif

CFLAGS		= $(EXFLAG) $(INC_DIRS) $(DEFINES) $(DEBUG) $(OPTIMIZE) 
CPPFLAGS	= 
LDOPTIONS	=

LIBRARIES	=

SRCS1		= cifo.c cifo_conf.c 
OBJS1		= $(SRCS1:.c=.o)
HDRS1		= 

DEPLIBS2	= 
SRCS2		= 
OBJS2		=
HDRS2		=

#-------------------------------------------------------------------------

all:: $(PROGRAMS)

$(PROGRAMS): $(OBJS1) $(HDRS1)
	ar rcu $@ $(OBJS1)
	@for i in $(PROGRAMS); do \
		echo Installing $$i in $(DSTDIR); \
		cp $$i $(DSTDIR); \
	done

debug:
	${MAKE} "DEBUG = -DDEBUG ${DEBUG}"

clean::
	rm -f $(PROGRAMS) $(OBJS1) $(OBJS2)
