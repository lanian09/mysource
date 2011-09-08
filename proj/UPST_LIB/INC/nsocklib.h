#ifndef __N_SOCKLIB_H__
#define __N_SOCKLIB_H__

#include <time.h>           /* time_t */
#include <sys/types.h>      /* fd_set */
#include "typedef.h"      /* fd_set */
#include "clisto.h"      /* fd_set */

/* ERROR CODE DEFINE */
#define E_GENERIC -1
#define E_SOCKET  -2
#define E_REUSEADDR -3
#define E_LINGER    -4
#define E_NONBLOCK  -5
#define E_BIND      -6
#define E_LISTEN    -7
#define E_ACCEPT    -8
#define E_SET_RCVBUF -9
#define E_SET_SNDBUF -10
#define E_GET_FLAGS  -11
#define E_SOCK_CLOSE -12
#define E_MAX_TUPPLE -13
#define E_NO_ENTRY   -14
#define E_NO_ENTRY2  -15
#define E_ADD_TUP_IN_CONN -21

#define SUCCESS 0
#define DIS_CONN_SOCK 1


//define
#define MAX_RECORD              8 	/* MAXIMUN TAF COUNT */

#define LISTEN_PORT_NUM         20
#define DEF_MAX_BUFLEN          8192*10000 	/* Process Socket Buffer */
#define DEF_MAX_SOCK_SIZE       8192*1000	/* Client Socket Buffer */

#define DEF_TIMER               5			/* Check Client Socket */

#define MAX_SUBDESC_SIZE        32
#define MAX_SUBSYS_NUM          8

#define SIDB_DATE_SIZE          9
#define SIDB_DATE_LEN			(SIDB_DATE_SIZE-1)
#define RNES_PKT_SIZE           129         /* rnes packet size */
#define RNES_NE_HEARTBEAT		0x41		/* ASCII A : NE의 활성상태를 알리는 HEART-BEAT 전송시 사용 */
#define RNES_NE_FILENAME		0x46		/* ASCII F : NE의 File명을 전송시 사용 */
#define RNES_NMS_HEARTBEAT      0x49        /* ASCII I : NMS의 활성상태를 알리는 HEART-BEAT 전송시 사용 */
#define RNES_NMS_ACK            0x59        /* ASCII Y : NE의 통계파일을 FTP로 정상수집후 ACK 전송시 사용 */


typedef struct _stNetTuple
{
#define SOCKET_OPEN  1
#define SOCKET_CLOSE 0
    time_t          tLastTime; /* Last Update Time */
    int             dSfd;      /* Socket File Descriptor */
    int             dStatus;   /* Status */
	int             dIdx;      /* Client Index */
    unsigned int    uiIP;      /* Client IP */
    int             dBufSize;  /* Read Buffer Size */
	int             dFront;	   /* Write Start Index */
	int             dRear;	   /* Write End Index */
    char            szBuf[DEF_MAX_BUFLEN]; /* Read Buffer */
    char            szWBuf[DEF_MAX_BUFLEN]; /* Write Buffer */
} stNetTuple, *pstNetTuple;

typedef struct _st_ClientInfo
{
    time_t          tLastTime;  /* LAST UPDATE TIME */
    int             dSfd;       /* SOCKET FD VALUE */
    int             dSysNo;     /* Rcv  Sock  SYSTEM NO */
    unsigned int    uiIP;       /* CLIENT IP */
    int             dBufSize;   /* READ BUFFER INDEX */
    int             dFront;     /* WRITE START INDEX */
    int             dRear;      /* WRITE END INDEX */
    char            szBuf[DEF_MAX_BUFLEN];  /* READ BUFFER */
    char            szWBuf[DEF_MAX_BUFLEN]; /* WRITE BUFFER */
    short           dLastFlag;	/* SOCKET_OPEN/CLOSE STATUS */
} st_ClientInfo, *pst_ClientInfo;

#define DEF_CLIENT_INFO_LEN sizeof(st_ClientInfo)

typedef struct _st_FD_Info
{
    fd_set  Rfds;                   /* READ FD_SET */
    fd_set  Wfds;                   /* WRITE FD_SET */
    int dMaxSfd;                    /* MAX FD VALUE */
    int dSrvSfd;                    /* SERVER FD VALUE */
} st_FDInfo, *pst_FDInfo;

#define DEF_FD_INFO_LEN sizeof(st_FDInfo)

typedef struct _st_SubSysInfo
{
	int             dType;                      /* Sub System Type */
	int             dNo;                        /* Sub System Number */
	unsigned int    uiIP;                       /* Sub Sustem IP Address */
	int             dFlag;                      /* Sub System Active Flag */
	char            szDesc[MAX_SUBDESC_SIZE];   /* Sub System Description & Alias */
} st_SubSysInfo, *pst_SubSysInfo;

typedef struct _st_SubSysInfoList
{                           
	int             dCount;
	st_SubSysInfo   stInfo[MAX_SUBSYS_NUM];     
} st_SubSysInfoList, *pst_SubSysInfoList;

#define DEF_SUBSYSINFOLIST_SIZE     sizeof(st_SubSysInfoList)

typedef struct _st_Send_Info_
{
	int             cnt;
	OFFSET          offset_Data;                                                           
} st_Send_Info, *pst_Send_Info;                                                            

#define DEF_RETRANS_TIME            (60*1)

typedef struct _st_SI_DB_
{           
	time_t      send_time;
	char        date[SIDB_DATE_SIZE];
	char        filename[RNES_PKT_SIZE];
	char        name[RNES_PKT_SIZE];
} st_SI_DB, *pst_SI_DB;

typedef struct _st_sidb_node {
	clist_head  list;
	st_SI_DB    stSIDB;
} st_sidb_node, *pst_sidb_node;


#define SIDB_NODE_SIZE          sizeof(st_sidb_node)
#define SIDB_NODE_CNT           3001

#define LGT_NMS_PORT_RNES       6200
#define LGT_NMS_PORT_DB         6300

/* SI_DB, SI_SVCMON에서 사용(외부와의 연결 개념으로 nsock에 포함시킴) */
#define cmem_head_init(infoptr, ptr)                    CINIT_LIST_HEAD(infoptr, ptr)
#define cmem_for_each_start(infoptr, pos, head)         clist_for_each_start(infoptr, pos, head)
#define cmem_for_each_end(infoptr, pos, head)           clist_for_each_end(infoptr, pos, head)
#define cmem_entry(ptr, type, member)                   clist_entry(ptr, type, member)     
#define cmem_offset(infoptr, ptr)                       clisto_offset(infoptr, ptr)
#define cmem_ptr(infoptr, offset)                       (U8 *)clisto_ptr(infoptr, offset) 

extern int init_sock(int *dSrvSfd, int dPort, fd_set *Rfds, int *Numfds);
extern int accept_sock(stNetTuple *stNet, int sock_sfd, fd_set *Rfds, int *Numfds, int dSockBuf, int *pdPos);
extern int add_tup_in_conntable (stNetTuple *stNet, int dSfd, unsigned long ulIP, fd_set *fdSet, int *NumFds, int dSrvSfd);
extern int add_tup_in_polltable(int fd, fd_set *Rfds, int *Numfds);
extern int del_tup_in_conntable (stNetTuple *stNet, int del_sfd);
extern int del_tup_in_polltable (stNetTuple *stNet, int fd, fd_set *fdSet, int *NumFds, int dSrvSfd);
extern int disconn_sock(stNetTuple *stNet, int dSfd, fd_set *fdSet, int *NumFds, int dSrvSfd);
extern int dInitSockFd(st_FDInfo *stFD, int dPort);
extern int dAcceptSockFd(st_ClientInfo *stSock, st_FDInfo *stFD, int *pdPos);
extern int dDisConnSock(st_ClientInfo *stSock, int dIdx, st_FDInfo *stFD);
extern int dAddSockInTable(st_ClientInfo *stSock, int dSfd, unsigned int uiIP, st_FDInfo *stFD);
#endif /* __N_SOCKLIB_H__ */
