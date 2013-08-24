CC		= gcc
CP		= cp
POS		= 

LIBS	=
INCS	=
DBGS	= -g
CFLAGS	= $(LIBS) $(INCS) $(DBGS) 

TARGET	= a.out

SRCS	= t1.c
OBJS	= $(SRCS:.c=.o)

#!----------------------------------------------
.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $<
.c.s:
	$(CC) $(CFLAGS) -c $<


all:	$(TARGET)

$(TARGET): $(SRCS) $(OBJS)
#@ echo Building $@ ...
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)
#	@ echo Building completed.

clean:
	rm -rf $(TARGET) $(OBJS)
	
