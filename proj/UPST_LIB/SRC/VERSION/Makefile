#--------------------- Makefile which have no SubDir ---------------------
SHELL  		= /bin/sh
SRCDIR 		= .
DSTDIR 		= ../../LIB

CC = gcc

TARGET		= all

SUBDIRS		= 

STD_INCDIR 	= 
STD_LIBDIR	=

LOC_INCDIR	= -I../../INC 
LOC_LIBDIR	= 

GBL_INCDIR	= -I../../../INC -I../../../../INC
GBL_LIBDIR	=

#OPTIMIZE    = -O -O2

#INC_DIRS	= $(LOC_INCDIR) $(GBL_INCDIR)
INC_DIRS	= $(LOC_INCDIR)
LIB_DIRS	=

#-------------------- Source & Objects & Libraries------------------------
PROGRAMS 	= libVERLIB.a
LIBS 		= 

DEFINES     =
DEBUG       = -g -Wall -fPIC
DEBUG      += -DDEBUG

CFLAGS 		= $(EXFLAG) $(INC_DIRS) $(DEFINES) $(DEBUG) $(OPTIMIZE)
CPPFLAGS	= 
LDOPTIONS	=


LIBRARIES	=

DEPLIBS1	= 

SRCS1		= version.c
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

clean::
	rm -f $(PROGRAMS) $(OBJS1) $(OBJS2)
