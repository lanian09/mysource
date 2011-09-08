OUT	= hamon
SRC = Hamon.c Timer.c
OBJ = $(SRC:.c=.o)

CC	= gcc

INC	= -I./ -I../ -I../../include -I../../libsrc/COMM -I/usr/sfw/include/
LIB = -L../../lib -L/usr/local/lib -L/usr/lib -L/usr/local/ssl/lib
CFLAGS = -g	-Wall $(INC)
LDFLAGS= $(LIB) -lkstat -lcomm -lnsl -lsocket -ladm -lelf -lcrypto -lpthread -lgen -lm

DEST = ../../bin

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)
	cp $(OUT) $(DEST)

install:
	cp $(OUT) $(DEST)

clean:
	rm -f $(OBJ) $(OUT) core
