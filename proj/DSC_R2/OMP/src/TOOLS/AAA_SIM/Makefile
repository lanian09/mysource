SHELL 		= /bin/sh
CC 			= gcc
AR 			= ar
RM 			= /bin/rm
DEPEND		= makedepend

TOP_LIB     = .
LIBRARY =   -L$(TOP_LIB)

INCLUDES 	= -I./ -I/usr/local/include/sasl
DEFINES  	= -DPRINT
OPTIMIZE 	= -O
DEBUG 		=

CFLAGS 		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)
DEPOPTS 	= $(INCLUDES) $(DEFINES)
HDRS 		=

SRCS        = aaa_sim.c radius_pkt.c lwrite.c
TARGET      = AAA_SIM

OBJS 		= $(SRCS:.c=.o)
EYES 		= $(SRCS:.c=.i)

LIBS    	= $(LIBRARY) -lMD5Lib -lsocket -lnsl

MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#

#
all: $(TARGET)

$(TARGET): $(OBJS) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	$(RM) -f *.u *.o *.i core $(TARGET)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(DEPOPTS) -- $(SRCS) 

# DO NOT DELETE THIS LINE -- make depend depends on it.

