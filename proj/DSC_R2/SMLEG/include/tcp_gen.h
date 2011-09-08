#ifndef __TMF_TCP_GEN_HEADER_FILE__
#define __TMF_TCP_GEN_HEADER_FILE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
//#include <pcap/pcap.h>

#pragma pack(1)
#define LISTEN_PORT_NUM 	5

#define POLLREAD		110
#define POLLWRITE		120

#define Tcp_Read_Err		-1011
#define Tcp_Read_Succ		 1011
#define Tcp_Connect_Err		-1012
#define Tcp_Connect_Succ	 1012
#define Tcp_No_Event		-1111
#define Tcp_Invalid_Msg		-1222
#define Tcp_Fatal_Error		-1223

#define CONNECTION_PORT		170
#define MESSAGE_PORT		180

#define MH_TIMEOUT		10

#define MAX_RECORD  		32

#define PACKET_HEAD_LEN		32

/*
#define PACKET_BODY_LEN		8192
*/
#define PACKET_BODY_LEN		40000

typedef struct {
    ulong   ip_addr;
    int     sfd;
    int     retrycnt;
    char    adminid[24];
} Tuple;

typedef struct {
    short   siDataLen;
    short   siTmfNo;
    short   siSvcType;
    short   siSvcCode;
    long    lTid;
    long    lReserved;
    char    szOperID[16];
} MY_TCP_HEAD;

typedef struct {
    MY_TCP_HEAD     head;
    char            data[PACKET_BODY_LEN];
} MY_TCP;

#pragma pack()
#endif

