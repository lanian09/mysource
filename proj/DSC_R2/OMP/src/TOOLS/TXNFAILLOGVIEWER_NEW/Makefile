
SHELL		= /bin/sh
CC		= gcc
AR		= ar
RM		= /bin/rm
MV		= mv
DEPEND		= makedepend
LINT		= lint


TOP_LIB		= ../../../lib
TOP_BIN		= ../../../bin
LIBRARY 	=   -L$(TOP_LIB)


#INCLUDES   	=  -I../../../include -I../ -I../FTP_LIB/include
INCLUDES   	=  -I. -I../../../include 
DEFINES     	= 
OPTIMIZE    	= -O -g3
DEBUG       	= -DDEBUG -DDEBUG_G 



CFLAGS		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) 
 
SRCS		= srchmsg_main.c srchmsg_proc.c srchmsg_prt.c type_cvt.c  srchmsg_parsing.c

TARGET		= TXNFAILLOGVIEWER_NEW

OBJS		= $(SRCS:.c=.o)


LIBS    = $(LIBRARY)
#LIBS    = $(LIBRARY) -lLogUtil -lShmQueue -lMD5Lib -lHeadLog 


MAKEFILE	= Makefile

#
#-----------------------------------------------------------
#

#


#all: $(SRCS) $(TARGET) 
all: $(SRCS) $(TARGET) 

$(SRCS):

$(TARGET): $(SRCS) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) 
#	cp $(TARGET) $(TOP_BIN)


lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

#install:
	cp $(TARGET) $(TOP_BIN)

clean:
	/bin/rm -f *.o $(TARGET) core*

new : clean $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
