# 
# Makefile for CDR LOG Parsing
#
#  2006.09.21

CC		= gcc
RM		= rm
CP		= cp
HOME		= ../../..


INC 		=  -I$(HOME)/include -I.


BINDIR 		= $(HOME)/bin

TARGET		=  CDRLOG_VIEW

LIBS 		= -lsocket -lnsl

CFLAGS 		= -g $(INC) -D__SUN__

SRCS		=   cdrlog_main.c \
			cdrlog_proc.c \
			cdrlog_config.c \
			type_cvt.c

OBJS		= $(SRCS:.c=.o)

all	: $(TARGET)

#$(TARGET): $(OBJS) $(LIBS)
$(TARGET): $(OBJS) 
	@echo "=============================================="
	@echo " Compile $(TARGET) .....
	@echo "=============================================="
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LIBS)

install:
	@echo "=============================================="
	@echo " Copy $(BINDIR) .....
	@echo "=============================================="
	$(CP) $(TARGET) $(BINDIR)

clean:
	/bin/rm -f $(OBJS) $(TARGET) core

