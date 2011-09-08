include ../../MkConfig.mk

TOP_BIN = 
TOP_LIB = ../../lib

INCLUDES =  -I../../include -I./ -I../../lib 
DEFINES  = 

CFLAGS = $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
SFLAGS = $(INCLUDES) $(DEFINES) $(COMMON_FLAGS)
DEPOPTS = $(INCLUDES) $(DEFINES) 

HDRS = 

SRCS = shmutil.c 

TARGET = libSHMUTIL.a 

OBJSV = $(SRCS:.c=s.o)
OBJCL = $(SRCS:.c=c.o)

#
#-----------------------------------------------------------
#

#
.SUFFIXES: .c .s s.o c.o p.o q.o .i .u .H
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(SFLAGS) -S $<
.cs.o:
	$(CC) -o $@ $(CFLAGS) -c $<
.cc.o:
	$(CC) -o $@ $(CFLAGS) -c $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<

all: $(TARGET) $(HADS)


$(TARGET): $(SRCS) $(HDRS) $(OBJSV) 
	$(RM) -f $@
	$(AR) rcu $@ $(OBJSV)
	$(AR) ts $@
	${CP} $@ ${TOP_LIB}
	${RM} *.o

clean:
	$(RM) -f *.s *.u *.o *.i core $(TARGET)

depend: $(SRCS)
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

lint:
	$(LINT) $(INCLUDES) -h -v -a -u -x -m $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
