# 
# Makefile for UDR store & search in DB(Mysql)
#
#  2006.09.21

CC		= gcc
RM		= rm
HOME		= ../../..
MYSQL_HOME	=	/opt/mysql/mysql


INC 		=  -I$(HOME)/include -I$(MYSQL_HOME)/include


BINDIR = $(HOME)/bin
LIBDIR = -L$(MYSQL_HOME)/lib

TARGET	=  UDRDB

LIBS = -lnsl -lsocket -lmysqlclient -lz -lm -lpthread

CFLAGS = -g $(INC) -D__SUN__ -D_DEBUG_PRINT

SRCS	=   udrdb_main.c \
			udrdb_init.c \
			udrdb_dbapi.c \
			udrdb_util.c \
			type_cvt.c

OBJS	= $(SRCS:.c=.o)

all	: $(TARGET)

$(TARGET): $(OBJS) $(LIBS)
	@echo "=============================================="
	@echo " Compile $(TARGET) .....
	@echo "=============================================="
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LIBDIR) $(LIBS)

install:
	@echo "=============================================="
	@echo " Copy $(BINDIR) .....
	@echo "=============================================="
	$(CP) $(TARGET) $(BINDIR)

clean:
	/bin/rm -f $(OBJS) $(TARGET) $(PROGRAMS) core

