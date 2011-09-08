#ifndef TRACE_PROTO_H
#define TRACE_PROTO_H

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sysconf.h>
#include <socklib.h>
#include <conflib.h>

#define LOOPBACK_IP		"127.0.0.1"	

void usage(void);
int get_cond_port_number(void);
int connect_to_peer(char *ipAddr, int portNum);
int read_from_fd(int fd, char *buff, int readsize);
void parse_sock_header(SockLibHeadType *sockHeader, int *sizeToRead, int *traceFlag);
int if_imsi_match(char *buff, char *imsi, char **posTrace);
char *current_time(void);
int read_trace(int sock, FILE *fp, char *imsi);
FILE *open_trace_file(char *path, char *imsi);
int trace_main(char *hostIp, int portNum, char *imsi, int outflag, char *path);



#endif /* TRACE_PROTO_H */
