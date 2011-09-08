include ../../MkConfig.mk
DSTDIR 		= ../../lib

TARGET		= all

SUBDIRS		= 

STD_INCDIR 	= 
STD_LIBDIR	=

LOC_INCDIR	= -I../../include -I..
LOC_LIBDIR	=

GBL_INCDIR	= -I../../../include 
GBL_LIBDIR	=

#OPTIMIZE		= -O -O2
OPTIMIZE    = -fpic

#INC_DIRS		= $(LOC_INCDIR) $(GBL_LIBDIR)
INC_DIRS		= $(LOC_INCDIR)
LIB_DIRS		=

#-------------------- Source & Objects & Libraries------------------------
PROGRAMS 		= libnifo.a
LIBS 		= 

DEBUG   		= -g3 -Wall

CFLAGS		= $(EXFLAG) $(INC_DIRS) $(DEFINES) $(DEBUG) $(OPTIMIZE)
CPPFLAGS	= 
LDOPTIONS	=


LIBRARIES	=

DEPLIBS1	= 

SRCS1		= nifo.c mems.c stMEMSINFO_Prt.c stMEMSNODEHDR_Prt.c st_MEMSZONE_Prt.c nifo_conf.c
OBJS1		= $(SRCS1:.c=.o)
HDRS1		= 

DEPLIBS2	= 
SRCS2		= 
OBJS2		=
HDRS2		=

#-------------------------------------------------------------------------

all: $(PROGRAMS)

$(PROGRAMS): $(OBJS1) $(HDRS1)
	ar rcu $@ $(OBJS1)
	@for i in $(PROGRAMS); do \
		echo Installing $$i in $(DSTDIR); \
		cp $$i $(DSTDIR); \
	done

clean:
	rm -f $(PROGRAMS) $(OBJS1) $(OBJS2)
