include ../../MkConfig.mk

CP      = /usr/bin/cp -f
TOP_LIB	= ../../lib
LIBRARY = -L../

INCLUDES= -I. -I../../include
DEFINES =

TARGET	= hasho
TARGETLIB	= libHASHO.a

CFLAGS	= $(COMMON_FLAGS) $(INCLUDES) $(DEFINES) $(DEBUG)
SRCS	= hasho.c memg.c debug_func.c

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

test: $(TARGET)

#$(SRCS):
#	$(GET) -s $@

$(TARGETLIB) : $(OBJS)
	$(RM) -f $@
	$(AR) clq $@ $(OBJS)
	$(AR) ts $@
	$(CP) *.a $(TOP_LIB)
	$(RM) *.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

lint:
	lint $(INCLUDES) $(CFLAGS) -h -v -a -u -x -m $(SRCS)

depend: $(SRCS)
	$(DEPEND) -o.o -f $(MAKEFILE) -- $(INCLUDES) -- $(SRCS)

install:
	cp $(TARGET) 

doc:
	doxygen

clean:
	/bin/rm -rf *.o $(TARGET) $(TARGET2) core* TEST_RESULT*.TXT MEMG

# DO NOT DELETE THIS LINE -- make depend depends on it.
