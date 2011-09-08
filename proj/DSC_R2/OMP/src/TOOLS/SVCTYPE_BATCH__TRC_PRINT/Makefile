CC = gcc
RM = rm -f

CFLAGS = -g -c -DDEBUG1

INCDIR = -I./ -I../../../include -I../../../libsrc/COMM
LIBDIR = -L../../../lib
LIBS   = -lcomm -lsocket -lnsl

TRSRCS = trace_main.c \
	   trace_util.c
TROBJS = $(TRSRCS:.c=.o)

SBSRCS = svctype_main.c \
		 svctype_util.c \
		 svctype_error.c

SBOBJS = $(SBSRCS:.c=.o)

all : TRC_PRINT SVCTYPE_BATCH

TRC_PRINT: $(TROBJS)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)

SVCTYPE_BATCH: $(SBOBJS)
	$(CC) -o $@ $^ $(LIBDIR) $(LIBS)

.c.o :
	$(CC) $(CFLAGS) $(INCDIR) $<

clean:
	$(RM) TRC_PRINT SVCTYPE_BATCH $(TROBJS) $(SBOBJS)
