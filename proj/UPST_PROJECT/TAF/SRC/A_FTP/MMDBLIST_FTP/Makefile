SHELL		= /bin/sh

CC			= gcc
AR			= ar 
RM			= /bin/rm
DEPEND		= makedepend

LIB_DIR     = ../../../../LIB
OAM_DIR		= ../../../../OAM
INCLUDES	= -I./ -I../../../INC -I../../../../INC -I$(LIB_DIR)/INC -I$(OAM_DIR)/INC
DEFINES     =
OPTIMIZE 	= -g3 -Wall
DEBUG 		=

CFLAGS 		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)
DEPOPTS 	= $(INCLUDES) $(DEFINES) 

SRCS 		= BalanceTree.c mmdb_alloc.c mmdb_api.c mmdb_init.c mmdblist_api.c
TARGET 		= libMMDBLISTFTP.a

OBJS 		= $(SRCS:.c=.o) 

EYES 		= $(SRCS:.c=.i)

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

all: $(TARGET) install

$(TARGET): $(SRCS) $(OBJS)
	$(RM) -f $(TARGET)
	$(AR) rcu $@ $(OBJS)

install:
	cp $(TARGET) ../

clean:
	$(RM) -f *.u *.o *.i core $(TARGET)

depend: $(SRCS) 
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
