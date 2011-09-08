SHELL	= /bin/sh

CC		= cc
AR		= ar 
RM		= /bin/rm

LIB_DIR = ../../../../LIB
INCLUDES= -I. -I../ -I../../../INC -I$(LIB_DIR)/INC
DEFINES =
OPTIMIZE=
#OPTIMIZE+= -O -g3
DEBUG	= -g3 -Wall

CFLAGS	= $(OPTIMIZE) $(INCLUDES) $(DEFINES) $(DEBUG)
DEPOPTS = $(INCLUDES) $(DEFINES) 

HDRS	=  

SRCS	= asso_tree.c asso_alloc.c asso_api.c lib_init.c stack_list.c lib_api.c
TARGET	= libASSOSTACK.a

OBJS	= $(SRCS:.c=.o) 


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

install:
	cp $(TARGET) ../

clean:
	$(RM) -f *.u *.o *.i core $(TARGET)

# DO NOT DELETE THIS LINE -- make depend depends on it.
