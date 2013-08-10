#--------------------- Makefile which have no SubDir ---------------------
#SHELL  		= /bin/ksh
SHELL  		= /usr/bin/bash
SRCDIR 		= .
DSTDIR 		= ../

CC = gcc

TARGET		= all

SUBDIRS		= 

MAKEDEPEND 	= makedepend

STD_INCDIR 	= 
STD_LIBDIR	=

LOC_INCDIR	= 

GBL_INCDIR	=
GBL_LIBDIR	=

INC_DIRS	= -I$(LOC_INCDIR)
LIB_DIRS	=

#-------------------- Source & Objects & Libraries------------------------
PROGRAMS 	= libMD5Lib.a
LIBS 		= 

CFLAGS 		= $(DEFINES) $(INC_DIRS) -g
CPPFLAGS	= 
LDOPTIONS	=

DEFINES 	= #-DDEBUG

LIBRARIES	=

DEPLIBS1	= 

#SRCS1		= md5.c
SRCS1		= md5c.c
OBJS1		= $(SRCS1:.c=.o)
HDRS1		= 


DEPLIBS2	= 
SRCS2		= 
OBJS2		=
HDRS2		=

#-------------------------------------------------------------------------

all:: $(SRCS1) $(PROGRAMS) 

$(SRCS1):

libMD5Lib.a: $(OBJS1) $(HDRS1)
	ar rcu $@ $(OBJS1)
	cp $@ $(DSTDIR)/

clean::
	rm -f $(PROGRAMS) $(OBJS1) 

