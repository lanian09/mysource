CC = gcc
INCS_DIR = 
LIBS_DIR =

HDRS = uepoll.h 
SRCS = uepoll.c
OBJS = $(SRCS:.c=.o)
TARGET = libuepoll.a

INCS = 
LIBS =
CFLAGS = -g3 -Wall

all: $(TARGET)

$(TARGET): $(OBJS)
	ar r $@ $(OBJS)
	install -d ../../lib
	install -d ../../inc
	cp $@ ../../lib
	cp $(HDRS) ../../inc


$(OBJS):: $(HDRS)

clean:
	$(RM) $(TARGET) $(OBJS)


