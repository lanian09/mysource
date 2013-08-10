CC	   = `which cc`
TARGET = main
SRCS   = t.c
OBJS   = ${SRCS:.c=.o}

#.c.o:
#t.o: t.c
.c.o:
	${CC} -c $<

all: ${TARGET}

${TARGET} : ${OBJS}
	gcc -o $@ ${OBJS}

clean:
	rm -rf ${TARGET} ${OBJS}
