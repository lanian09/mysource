include ../../MkConfig.mk

TOP_LIB	= ../../lib
LIBRARY = -L../
INCLUDES= -I. -I../../include
DFINES =

TARGETLIB 	= libMIF.a

CFLAGS	= $(COMMON_FLAGS) $(INCLUDES) $(DEFINES)
SRCS	= nifo.c nifo_func.c mems.c debug_func.c


TEST = nifotest

OBJS	= $(SRCS:.c=.o)

MAKEFILE= Makefile

#
#-----------------------------------------------------------
#

#
.SUFFIXES: .c .s .o .i .u
.s.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(SFLAGS) -S $<
.c.o:
	$(CC) $(CFLAGS) -c $<
.c.u:
	$(CC) $(CFLAGS) -j $<
.c.i:
	$(CC) $(CFLAGS) -P $<


all: $(TARGETLIB)

#$(SRCS):
#	$(GET) -s $@

$(TARGETLIB) : $(OBJS)
	$(RM) -f $@
	$(AR) clq $@ $(OBJS)
	$(AR) ts $@
	$(MV) $@ $(TOP_LIB)
	$(RM) *.o

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) $(TOP_LIB)

doc:
	doxygen

nifotest: $(OBJS)
	$(CC) -DMIF_TEST $(CFLAGS) -o nifotest nifo.o nifo_func.o mems.o debug_func.o $(LIBRARY) $(LIBS)

clean:
	/bin/rm -rf *.o $(TARGET) core* TEST_RESULT*.TXT MEMG

# DO NOT DELETE THIS LINE -- make depend depends on it.
