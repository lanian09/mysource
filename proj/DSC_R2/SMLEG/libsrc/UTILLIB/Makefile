#--------------------- Makefile which have no SubDir ---------------------
include ../../MkConfig.mk

SRCDIR 		= .
DSTDIR 		= ../../lib

TARGET		= all

SUBDIRS		= 

STD_INCDIR 	= 
STD_LIBDIR	=

LOC_INCDIR	= ../../include 
LOC_LIBDIR	=

GBL_INCDIR	=
GBL_LIBDIR	=

INC_DIRS	= -I$(LOC_INCDIR)
LIB_DIRS	=

#-------------------- Source & Objects & Libraries------------------------
PROGRAMS 	= libLogUtil.a
LIBS 		= 

CFLAGS 		= $(COMMON_FLAGS) $(INC_DIRS) $(DEFINES)
CPPFLAGS	= 
LDOPTIONS	=

DEFINES 	=

LIBRARIES	=

DEPLIBS1	= 

#SRCS1		= lwrite.c typecvt.c type_cvt.c trace.c duplicate.c
SRCS1		= lwrite.c typecvt.c type_cvt.c duplicate.c
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
