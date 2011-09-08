SHELL   = /bin/sh
CC      = gcc
AR      = ar
RM      = rm -f
CP      = cp -f
MV      = mv -f

BIN_DIR     = ../../LIB
LIB_DIR     = ../../../LIB
OAM_DIR     = ../../../OAM
INCLUDES    = -I./ -I../../INC -I../../../INC -I$(LIB_DIR)/INC -I$(OAM_DIR)/INC

OPTIMIZE 	= -g3

MACHINE 	=
#MACHINE 	= -m32     # for 32-bit App in 64-bit OS

DEFINES 	= -DBUFFERING   # Turn on buffering message queue for CAPD, PANA

COMMON_FLAGS = $(MACHINE) $(OPTIMIZE) -Wall -DSMALL_SESSION

DEFINES     =
DEBUG 		=

CFLAGS 		= $(COMMON_FLAGS) $(INCLUDES) $(DEFINES) $(DEBUG)

SRCS 		= ippoolbit_api.c	
TARGET 		= libIPPOOLBIT.a

OBJS 		= $(SRCS:.c=.o) 


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

$(TARGET): $(SRCS) $(OBJS) $(HDRS)
	$(RM) -f $(TARGET)
	$(AR) rcu $@ $(OBJS)

install :
	cp $(TARGET) ${BIN_DIR}

clean:
	$(RM) -f *.u *.o *.i core $(TARGET)

depend: $(SRCS) 
	${CC} -M ${CFLAGS} < $?

# DO NOT DELETE THIS LINE -- make depend depends on it.
