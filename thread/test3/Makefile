CFLAG = -D_REENTRANT
LIBS  = -lpthread
SRC   = thread.c
OBJ   = $(SRC:.c=.o)

TARGET= th

$(TARGET): $(OBJ)
	cc -g -o $@ $? $(LIBS)

clean:
	rm -rf $(TARGET) $(OBJ)
