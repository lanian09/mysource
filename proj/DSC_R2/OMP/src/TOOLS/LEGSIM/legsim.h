#ifndef __LEG_H__
#define __LEG_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#include <commlib.h>
#include <sysconf.h>
#include <comm_msgtypes.h>
#include <comm_almsts_msgcode.h>
#include <stmconf.h>
#include <stm_msgtypes.h>
#include <omp_filepath.h>
#include <mysql_db_tables.h>
#include <mysql/mysql.h>
//#include <proc_version.h>
#include <nmsif.h>


// stmd_main.c
extern int send2FIMD(char *buff);
extern int InitSys(void);

#endif
