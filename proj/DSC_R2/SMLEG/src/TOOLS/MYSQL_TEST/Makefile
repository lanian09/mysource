CC		= gcc
LINT	= lint

INSTALL = /usr/sbin/install

INCLUDE =  -I. -I${MYSQL_HOME}/include


BINDIR = ../../bin
LOC_LIBDIR = -L../../lib -L${MYSQL_HOME}/lib

PROGRAMS	= db_backup 

LIBRARIES = -lmysqlclient -lz -lm 

LDOPTIONS = $(LOC_LIBDIR) $(LIBRARIES)

CFLAGS = -g -DDEBUG $(INCLUDE) #-DUNIX_SVR4 -DSYSTEMV -DUNIX -DSOLARIS -DMULTITHREAD -DSOLARIS_SPARC -g 

SRCS	=   inittest.c \

OBJS	= $(SRCS:.c=.o)

#-----------------------------------------------------------

all:: $(PROGRAMS) install

$(PROGRAMS): $(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDOPTIONS)
#	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LDOPTIONS)

install:
	cp $(PROGRAMS) $(BINDIR)

clean:
	rm -f $(OBJS) $(PROGRAMS) core

lint:
#	$(LINT) $(INCLUDES) -h -v -a -u -x $(SRCS)
	$(LINT) $(INCLUDES) -h -v -a -u -x -n -l $(SRCS)

# DO NOT DELETE

