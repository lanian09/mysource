
SHELL		= /bin/sh
CC			= gcc
AR			= ar
RM			= /bin/rm
MV			= mv
DEPEND		= makedepend
LINT		= lint


TOP_LIB		= ../../lib
TOP_BIN		= ../bin
LIBRARY =   -L$(TOP_LIB)


INCLUDES   	=  -I../../include
DEFINES     = 
OPTIMIZE    = -O -g3
#OPTIMIZE    = -m64 -O -g3
DEBUG       = -DDEBUG -DDEBUG_G -D__SUN__



CFLAGS		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) 
 
SRCS		= srchmsg_lib.c srchmsg_proc.c srchmsg_prt.c type_cvt.c 

TARGET		= libsrchudr.a

OBJS		= $(SRCS:.c=.o)


LIBS    = $(LIBRARY) -lsocket -lnsl
#LIBS    = $(LIBRARY) -lLogUtil -lShmQueue -lMD5Lib -lHeadLog 


MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#

#


#all: $(SRCS) $(TARGET) 
all: $(SRCS) $(TARGET) install

$(SRCS):
$(TARGET): $(SRCS) $(OBJS)
	ar rcus $@ $(OBJS)
#	cp $(TARGET) $(TOP_BIN)


install:
	cp $(TARGET) $(TOP_LIB)

clean:
	/bin/rm -f *.o $(TARGET) core*

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
