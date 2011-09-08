#ifndef __IPAF_SHM2_HEADER_FILE__
#define __IPAF_SHM2_HEADER_FILE__

#include <inttypes.h>
#include <time.h>

#pragma pack(1)

#define NOT_EQUIP   0x00
#define STOP		0x01
#define MASK		0x02
#define NORMAL		0x03
#define MINOR		0x04
#define MAJOR		0x05
#define CRITICAL	0x06

#define MASK_VALUE	128

typedef struct {
    long mtype;
    char mtext[4096];
} mesg_t;

typedef struct _st_BDF {
    char power[2];
    char port[6];
} st_BDF, *pst_BDF;

typedef struct _st_BDF_TOT {
    st_BDF  stBDF[16];
} st_BDF_TOT, *pst_BDF_TOT;

typedef struct _st_CurrVal {
    long long  llCur;
    long long  lMax;
} st_CurrVal, *pst_CurrVal;

typedef struct _st_ProcInfo {
    int     pid;
    time_t  when;
} st_ProcInfo, *pst_ProcInfo;

#define MAX_IPAF_LINK      	4
#define MAX_IPAF_SW_BLOCK  	24

/*
* DEFINITION MMDB INDEX FOR FIDB
*
#define DEF_MMDB_SESS		0
#define DEF_MMDB_OBJ		1
#define DEF_MMDB_CDR		2

#define DEF_MMDB_CALL		5
#define DEF_MMDB_WAP1		6
#define DEF_MMDB_WAP2		7	
#define DEF_MMDB_HTTP		8
#define DEF_MMDB_WICGS      9
#define DEF_MMDB_JAVA       10
#define DEF_MMDB_VOD        11
#define DEF_MMDB_VODU       12
*/


typedef struct _st_IPAF {
	time_t   tUpTime;
    int      dReserv;
    unsigned char   cpu;
    unsigned char   mem;
    unsigned char   queue;
    unsigned char   disk;

    unsigned char   sess;			/* 20040323,poopee */
	unsigned char	mirrorsts[2];   /* 20040414,poopee, 0x03:NORMAL,0x06:CRITICAL */
	unsigned char	reserved1[1];

    unsigned char   link[MAX_IPAF_LINK];
	unsigned char	reserved2[4];
	unsigned char   mpsw[MAX_IPAF_SW_BLOCK];

	unsigned char    hwfan[6];
	unsigned char    hwntp[2];
	unsigned char    hwpwr[2];
    unsigned char    hwdisk[2];

	unsigned char    hwpwrcnt;
    unsigned char    hwdiskcnt;
    unsigned char    hwfancnt;
    unsigned char    hwntpcnt;

    st_CurrVal  cpusts;
    st_CurrVal  memsts;
    st_CurrVal  quests;
    st_CurrVal  disksts;

	st_CurrVal	sesssts;	/* 20040319,poopee */

    st_ProcInfo mpswinfo[MAX_IPAF_SW_BLOCK];

	unsigned int    rsrcload[16];   /* 0:MMDB_SESS, 1:MMDB_OBJ, 2:MMDB_CDR */
                                    /* 5:MMDBLIST, 6:MMDBLIST_ME, 7:MMDBLIST_KUN, 8:MMDBLIST_ADS, 9:MMDBLIST_MARS 10:MMDBLIST_MACS */
									/* 11:MMDBLIST_WICGS 12:VODMANA, 13:VODDANA 14:VODANA 15:VODUDP */ 
} st_IPAF, *pst_IPAF;

typedef struct _st_MASKIPAF {
    unsigned char    link[MAX_IPAF_LINK];       /*  4 */
    unsigned char    hwntp[2];
    unsigned char    hwpwr[2];
    unsigned char    hwdisk[2];
    unsigned char    hwfan[6];
    unsigned char    mpsw[MAX_IPAF_SW_BLOCK];   /* 24 */
} st_MASKIPAF, *pst_MASKIPAF;


#pragma pack()
#endif


