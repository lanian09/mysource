INC_DIR	=	-I.. -I../../../include -I../../../libsrc/COMM -I${MYSQL_HOME}/include

LIB_DIR	=	../../../lib

all:
	gcc -o nmsim nmsim.c socket.c $(INC_DIR) -lsocket -lnsl -g
	gcc -o bsd_app bsd_app.c $(INC_DIR) -L$(LIB_DIR) -lcomm -g

clean:
	\rm -f core nmsim bsd_app *.o core
