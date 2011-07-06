CC = cc
DEFINE = -Wall
CFLAGS = -g
PROGRAM = bst

SRCS = bst_main.c bst_func.c
OBJS = $(SRCS:.c=.o)
#---------------------------------------
all :: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(DEFINE)

clean :: 
	rm -rf $(PROGRAM) $(OBJS)




