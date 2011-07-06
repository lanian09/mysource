CC		= cc
RM		= rm

TARGET	= run
TARGET2	= run2

SRCS	= t.c
OBJS	= $(SRCS:.c=.o)
# !--------------------

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

2: 
	make "TARGET=$(TARGET2)"

clean:
	$(RM) -rf $(TARGET) $(TARGET2) *.o


