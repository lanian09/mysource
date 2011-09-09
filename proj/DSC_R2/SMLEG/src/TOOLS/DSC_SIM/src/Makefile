#
#	$Id: Makefile,v 1.1.1.1 2011/04/19 14:13:43 june Exp $ 

#	Copyright (c) 2007-2008 upresto Inc, Korea
#	All rights reserved.

VPATH	= .:../h:../../h

CC		= gcc

INC_PATH	= -I./ -I../inc -I../../inc -I../libsrc/cgalib/inc/
LIB_PATH	= -L../lib -L../../lib -L../libsrc/cgalib/lib/

LIBS   	= -lhasho -lmemg -ltimerN -lhashg -luepoll

CFLAGS	= -g $(INC_PATH) -DLINUX -DDUMP
LFLAGS	=  -DLINUX -DDUMP

TARGET	= DSC_SIM

SRC     = conflib.c\
		  logutil.c\
		  mmc.c\
		  mmc_def.c\
		  mmc_func.c\
		  read_conf.c\
		  radius_encoding.c\
		  tcp_task.c\
		  session.c\
		  sim_main.c\
		  sim_sock.c


OBJS	= $(SRC:.c=.o)


all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS) $(LIB_PATH) $(LIBS)

clean:
	rm -f $(OBJS) $(TARGET)


