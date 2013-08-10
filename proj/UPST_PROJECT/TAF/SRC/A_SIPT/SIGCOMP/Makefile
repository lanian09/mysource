
SHELL	= /bin/sh

CC 		= gcc
AR 		= ar 
RM 		= /bin/rm
DEPEND	= makedepend
LINT 	= lint

TOP_BIN = 

INCLUDES=  -I../SIGCOMP_INC/ -I/usr/include
INCLUDES+= -I/usr/include/glib-2.0
INCLUDES+= -I/usr/lib/glib-2.0/include
INCLUDES+= -I/usr/lib64/glib-2.0/include

DEFINES = 
#OPTIMIZE= -O3 -Wall
DEBUG   = -g3 -Wall
#DEBUG  += -DDEBUG -DMICRO_LOG

CFLAGS 	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG) -DHAVE_CONFIG_H -D_U_=""
#LFLAG	= -L/usr/lib -lglib-2.0 -lm
SFLAGS 	= $(INCLUDES) $(DEFINES) $(DEBUG)
DEPOPTS = $(INCLUDES) $(DEFINES) 

SRCS 	= \
		udvm.c \
		udvm_state.c \
		sha1.c \
		strutil.c \
		tvbuff.c \
		sigcomp.c \
		comp_lz77.c \
		comp_lzss.c

TARGET 	= libsigcomp.a 

OBJSV 	= $(SRCS:.c=s.o)
OBJCL 	= $(SRCS:.c=c.o)

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

all: $(TARGET) install

$(TARGET): $(SRCS) $(OBJSV) 
	$(RM) -f $@
	$(AR) clq $@ $(OBJSV)
	$(AR) ts $@ 
	#cp $@ ../../LIB

install:
	cp $(TARGET) ../

app:
	$(CC) -c $(CFLAGS) app.c
	$(CC) -o app app.c $(LFLAG) -L. -lsigcomp

clean:
	$(RM) -f *.s *.u *.o *.i core $(TARGET)

depend: $(SRCS)
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

lint:
	$(LINT) $(INCLUDES) -h -v -a -u -x -m $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

