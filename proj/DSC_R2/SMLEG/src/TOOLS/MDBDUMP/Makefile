
SHELL	= /bin/sh
CC		= gcc
AR		= ar
RM		= /bin/rm
MV		= mv
DEPEND	= makedepend
LINT	= lint

TOP_BIN	= ../../BIN
#TMF_BIN	= /tmfapp/BIN
TMF_BIN	= ../../bin
#TOP_LIB	= /data/bsdpkg/BMTPKG/IPAS_L1.0.0/LIB
TOP_LIB	= ../../lib
LIBRARY =   -L$(TOP_LIB)

INCLUDES = -I../../include
DEFINES  = -DPRINT
OPTIMIZE = -O
ifeq ($(IPAF_DEBUG),)
#DEBUG   = -g3 -DDEBUG -Wall
#DEBUG   = -g3 -DDEBUG_SVCFILTER -Wall
DEBUG    = -g3 -m64 -Wall -DDEBUG
else
DEBUG    = $(IPAF_DEBUG)
endif

# for S4000
M_FLAG		= 
CFLAGS		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) $(M_FLAG)
SRCS		= shmmb_print.c \
              mmcr_destipapi.c \
              mmcr_destportapi.c \
              mmcr_ipalloc.c \
              mmcr_ipbtree.c \
              mmcr_portalloc.c \
              mmcr_portbtree.c \
              mmcr_sem.c \
              mmcr_svcoptapi.c \
              mmcr_svcoptalloc.c \
              mmcr_svcoptbtree.c 
              
#              mmcr_destip.c \
#              mmcr_destport.c \

TARGET		= shmmb_print

OBJS		= $(SRCS:.c=.o)

LIB_PATH_ORACLE = 

LIBS = -lMMDBDESTIP -lMMDBDESTPORT

MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#

#


all: $(TARGET) 

#$(SRCS):
#	$(GET) -s $@


$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LIBRARY) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $<

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(TMF_BIN)

clean:
	/bin/rm -f *.o $(TARGET) $(OBJS) core* 


# DO NOT DELETE THIS LINE -- make depend depends on it.
