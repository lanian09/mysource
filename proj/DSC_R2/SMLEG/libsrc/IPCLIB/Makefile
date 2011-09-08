#--------------------- Makefile which have no SubDir ---------------------
include ../../MkConfig.mk

CP  		= /usr/bin/cp -f
SRCDIR 		= .
DSTDIR 		= ../../lib

TARGET		= all

SUBDIRS		= 

MAKEDEPEND 	= makedepend

STD_INCDIR 	= 
STD_LIBDIR	=

LOC_INCDIR	= ../../include 
LOC_LIBDIR	=

GBL_INCDIR	=
GBL_LIBDIR	=

INC_DIRS	= -I$(LOC_INCDIR)
LIB_DIRS	=

#-------------------- Source & Objects & Libraries------------------------
PROGRAMS 	= libIPC.a
LIBS 		= 

CFLAGS 		= $(COMMON_FLAGS) $(INC_DIRS) $(DEFINES) $(DEBUG)
CPPFLAGS	= 
LDOPTIONS	=

DEFINES 	=

LIBRARIES	=

DEPLIBS1	= 

SRCS1		= sem.c shm.c msgq.c
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
	${CP} $@ ${DSTDIR}
	${RM} *.o


clean::
	rm -f $(PROGRAMS) $(OBJS1) $(OBJS2)

install:: $(PROGRAMS) 
	@for i in $(PROGRAMS); do \
		echo Installing $$i in $(DSTDIR); \
		cp $$i $(DSTDIR); \
	done
