LIBS = -lpthread
SRCS = thread.c
OBJS = $(SRCS:.c=.o)
CC   = cc

TARGET = thd

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) -g3 $(SRCS) $(LIBS)

clean :
	rm -rf $(OBJS) $(TARGET) core

