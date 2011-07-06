TARGET=aa
SRCS = t.c
OBJS = ${SRCS:.c=.o}
CFLAG=-g3

${TARGET} : ${OBJS}
	cc ${CFLAG} ${OBJS} -o $@

clean :
	rm -rf ${TARGET} ${OBJS} test*

