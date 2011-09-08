#ifndef __ETH_CAPD_HEADER_FILE__
#define __ETH_CAPD_HEADER_FILE__

#define	MAX_BUF_SIZE	2000
#define MAX_MTU_SIZE	1520

#pragma pack(1)
typedef struct { 
	unsigned char	bRtxType;
	unsigned char	bReserve1;  
	unsigned char	bReserve2;	
	unsigned char	bReserve3;
	int				curtime;
	int				ucurtime; 
	int				datalen;
} T_CAPHDR;

#define	CAP_HDR_SIZE 	sizeof(T_CAPHDR)

#pragma pack(0)

#endif

