
SHELL 		= /bin/sh

CC 			= gcc
AR 			= ar 
RM 			= /bin/rm
LINT 		= lint

BIN_DIR		= ../../LIB
LIB_DIR     = ../../../LIB
OAM_DIR     = ../../../OAM
INCLUDES    = -I./ -I../../INC -I../../../INC -I$(LIB_DIR)/INC -I$(OAM_DIR)/INC

DEFINES 	= 
OPTIMIZE	=
DEBUG   	= -g3 -DDEBUG -Wall

CFLAGS 		= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)
SFLAGS 		= $(INCLUDES) $(DEFINES) $(DEBUG)
DEPOPTS 	= $(INCLUDES) $(DEFINES) 

SRCS 		= InitDB.c BalanceTree.c Shm_dis.c mmdb_api.c 

TARGET 		= libGREENTRYSESS.a 

OBJSV 		= $(SRCS:.c=s.o)
OBJCL 		= $(SRCS:.c=c.o)

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

install:
	cp $(TARGET) $(BIN_DIR)

clean:
	$(RM) -f *.s *.u *.o *.i core $(TARGET)

depend: $(SRCS)
	$(DEPEND) -o.o -f Makefile -- $(DEPOPTS) -- $(SRCS)

lint:
	$(LINT) $(INCLUDES) -h -v -a -u -x -m $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.

