#ifndef __SOCKIO_H__
#define __SOCKIO_H__

/*
	project 내 System 간 Socket 통신을 위한 각종 값을 정의 
*/

#define PORT_DEFINE			18000
#define MAX_ALMDBUF_LEN  69918720

#define S_PORT_ALMD			( PORT_DEFINE + 301 )
#define S_PORT_COND			( PORT_DEFINE + 302 )
#define S_PORT_MONITOR		( PORT_DEFINE + 303 )	/* DQMS 5MIN MONITORING PORT */
#define S_PORT_MMCD			( PORT_DEFINE + 500 )
#define S_PORT_NTAFTIF		( PORT_DEFINE + 600 )
#define S_PORT_NTAFUIF		( PORT_DEFINE + 700 )
/* R2.0.0 Add 2004.0406 (lander) */
#define S_PORT_MRDRIF		( PORT_DEFINE + 800 )
#define S_PORT_SBY			( PORT_DEFINE + 900 )	/* 20040416,poopee */

#define S_PORT_SI_LOG		( PORT_DEFINE + 1000 )
#define S_PORT_G_SI_LOG		( PORT_DEFINE + 1001 )
#define S_PORT_SI_SIG		( PORT_DEFINE + 1002 )
#define S_PORT_SI_SVC		( PORT_DEFINE + 1003 )
#define S_PORT_G_SI_SVC		( PORT_DEFINE + 1004 )

#define S_PORT_ACCOUNT		1813
#define S_PORT_QUD			49149
#define S_PORT_AAAIF		1814
#define S_PORT_AAAIF_GGSN	49147

#define MAGIC_NUMBER		0x3812121281282828LL
#define LISTEN_PORT_NUM2	16

#define DEF_BROAD_CAST		101	/* .....special.. */

typedef struct _st_SubSys{
#define DEF_DESC_SIZE_32
	int						dType;
	unsigned short int	usSysNo;
	unsigned int 		uiIP;
	int						dFlag;
	char					 szDesc[DEF_DESC_SIZE_32];
} st_subsys, *pst_subsys;

typedef struct _st_SubSys_Mng{
#define MAX_SUBSYS_COUNT 128
	st_subsys sys[MAX_SUBSYS_COUNT];
} st_subsys_mng, *pst_subsys_mng;

#define MAGIC_NUMBER 0x3812121281282828LL

typedef struct _st_NTAFTHeader		
{	
	 long long int		 llMagicNumber;		/* Magic Number 0x3812121281282828L*/
	 long long int		 llIndex;				/* Request for DB Index Used NTAFUIF */
	 unsigned short int 	usResult;				/* Result */
	 unsigned short int 	usSerial;
	 unsigned char		ucNTAMID;
	 unsigned char		ucNTAFID;
	 unsigned char		szReserved[2];		/* Reserved */
	 unsigned short int	usTotlLen;			 /* Packet Header + Packet Length */
	 unsigned short int	usBodyLen;			 /* Packet Body Length + Extended Len */
	unsigned short int	usExtLen;			/* Packet Extension Length or Error */
	 unsigned char		ucSvcID;
	 unsigned char 		ucMsgID;
}st_NTAFTHeader, *pst_NTAFTHeader;

#define NTAFT_HEADER_LEN			sizeof(st_NTAFTHeader)

#endif /* __SOCKIO_H__ */
