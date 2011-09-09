CC		= gcc
LINT	= lint

INSTALL = /usr/sbin/install

#INCLUDE =  -I. -I../../../include -I../../../libsrc/COMM -I/opt/mysql/mysql/include
#INCLUDE =  -I. -I../../../include -I../../../libsrc/COMM -I${MYSQL_HOME}/include
INCLUDE =  -I. -I../../../include


BINDIR = ../../../bin
#LOC_LIBDIR = -L../../lib -L/opt/mysql/mysql/lib
#LOC_LIBDIR = -L../../../lib -L${MYSQL_HOME}/lib
LOC_LIBDIR = -L../../../lib

PROGRAMS	= legsim 

#20041202.mnpark
#LIBRARIES = -lcomm -lnsl -lsocket -lmysqlclient -lm
LIBRARIES = 

LDOPTIONS = $(LOC_LIBDIR) $(LIBRARIES)

#####################################################################################
# DEFINES INFO
#   -D_ISSUE : DELETE 기준 : 7일전 DATA 삭제, 
#	-DTEST	 : DELETE 기준 : 현재시간 전 DATA 삭제, 5분 주기 
# EX) DEFINES =  -DISSUE  -DTEST
#####################################################################################

CFLAGS = -Wall -g -DDEBUG -DDELAY $(DEFINES) $(INCLUDE)#-DUNIX_SVR4 -DSYSTEMV -DUNIX -DSOLARIS -DMULTITHREAD -DSOLARIS_SPARC -g

SRCS	=   legsim_init.c legsim_main.c

OBJS	= $(SRCS:.c=.o)

#stmd_periodic_rule_cnt.c \
#stmd_update_rule.c \
#stmd_update_pur.c \

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

