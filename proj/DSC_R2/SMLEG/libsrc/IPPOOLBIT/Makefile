include ../../MkConfig.mk

CP 			= /usr/bin/cp -f
TOP_LIB		= ../../lib
INCLUDES 	= -I./ -I../../include -I../../lib
DEFINES     =
DEBUG 		=

CFLAGS = $(COMMON_FLAGS) $(INCLUDES) $(DEFINES) $(DEBUG)
DEPOPTS = $(INCLUDES) $(DEFINES) 

HDRS =  

SRCS = ippoolbit_api.c	
TARGET = libIPPOOLBIT.a

OBJS = $(SRCS:.c=.o) 

EYES = $(SRCS:.c=.i)

LIBS	= 


#
#-----------------------------------------------------------
#

#
.SUFFIXES: .c .s .o .i .u
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(CFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<

all: $(TARGET)

$(TARGET): $(SRCS) $(OBJS) $(HDRS)
	$(RM) -f $(TARGET)
	$(AR) rcu $@ $(OBJS)
	${CP} $@ ${TOP_LIB}
	${RM} *.o

clean:
	$(RM) -f *.u *.o *.i core $(TARGET)

depend: $(SRCS) 
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
