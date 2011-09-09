OUT=sim
SRC=sim.c
OBJ=$(SRC:.c=.o)

CFLAGS=-I. -I/usr/local/include/stilib

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -o $@ $< -lmgmt

clean:
	$(RM) $(OBJ) $(OUT) a.out core	
