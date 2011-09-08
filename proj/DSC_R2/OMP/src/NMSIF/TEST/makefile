SQL_INC	=	/usr/local/mysql/include
SQL_LIB	=	/usr/local/mysql/lib

all:
	gcc -o get_alm get_alm.c -I$(SQL_INC) -L$(SQL_LIB) -lmysqlclient -lz -lm -lpthread -lsocket -lnsl
	gcc -o get_stat get_stat.c -I$(SQL_INC) -L$(SQL_LIB) -lmysqlclient -lz -lm -lpthread -lsocket -lnsl
	gcc -o get_fname get_fname.c -g

clean:
	\rm -f *.o get_alm get_stat get_fname core
