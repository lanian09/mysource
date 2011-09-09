#ifndef __MLOG_INIT_H__
#define __MLOG_INIT_H__

#include "typedef.h" 
#include "common_stg.h"

#define CILOG_PATH              START_PATH"/CILOG/"

#define MAX_LOCCODE_LEN		    8
#define IS_FOUND_LOCCODE		0x10
#define DEF_WEBLOG_ON 			1
#define DEF_WEBLOG_OFF 			0
#define IS_FOUND_WEBLOG			0x02
#define IS_FOUND_SYSNO			0x01
#define MAX_FTYPE_LEN           8
#define MAX_FTYPE_SIZE          (MAX_FTYPE_LEN + 1)
#define MAX_FNAME_SIZE		    128
#define MAX_DATETIME_LEN   	    14
#define MAX_DATETIME_SIZE  	    (MAX_DATETIME_LEN + 1)
#define MAKEFILE_TIMEOUT	    60
#define FILEFLUSH_CNT		    100
#define MAX_SEQ_LEN        	    4
#define MAX_SEQ_SIZE       	    (MAX_SEQ_LEN + 1)
#define MAX_FILEMNG_SIZE 	    sizeof(FILEMNG)
#define CILOG_PATH_SIZE         strlen(CILOG_PATH)
#define SNLOG_PATH_SIZE		    strlen(SNLOG_PATH)
#define SNLOG_PATH			    START_PATH"/SNLOG/"
#define WEBLOG_PATH			    START_PATH"/WEBLOG/"
#define WEBLOG_PATH_SIZE	    strlen(WEBLOG_PATH)
#define FIN_FILENAME_SIZE   	42
#define REAL_FILE_LEN			40				/* ex) SKAS9_FUSRCAL_ID0001_T20061101021000.DAT */

enum {
	FTYPE_TCP,    FTYPE_HTTP,  FTYPE_PAGE,  FTYPE_RPPI,  FTYPE_RTSP, 
	FTYPE_VOD,    FTYPE_VT,    FTYPE_IM,    FTYPE_SIP,   FTYPE_MSRP, 
	FTYPE_ERPPI,  FTYPE_FTP,
	FTYPE_MTCP,   FTYPE_MHTTP, FTYPE_MPAGE, FTYPE_MRPPI, FTYPE_MRTSP,
	FTYPE_MVOD,   FTYPE_MVT,   FTYPE_MIM,   FTYPE_MSIP,  FTYPE_MMSRP,
	FTYPE_MERPPI, FTYPE_MSIG,  FTYPE_DIA, 
	FTYPE_WEB,    FTYPE_NET,   FTYPE_ITC,   FTYPE_IHT,
	MAX_FILETYPE_CNT
};

typedef struct _st_FILEINFO {
	U32     uiUseFlag;                      /**< This Type Data File Use Flag : 1 = Use, 0 = Not Use */
	FILE    *pDataFile;                     /**< Data File Pointer */
	U32     uiRecordCnt;                    /**< Record Count in File */
	U8      szFileType[MAX_FTYPE_SIZE];     /**< File Type = NAVTCP, NAVTRN, NAVPAG, USRCAL, CONTRN, ONLINE */
	U8      szDataFile[MAX_FNAME_SIZE];  /**< DATA File Name */
	U8      szFinFile[MAX_FNAME_SIZE];   /**< FIN File Name */
} FILEINFO, *pFILEINFO;

typedef struct _st_FILEMNG {
	U16     usSeq;                          /**< File Sequence Number */
	U16     usSystemID;                     /**< System ID = 9 */
	time_t  tTime;                          /**< UTC Time */
	struct tm stTime;                      /**< Local Time Information */
	U8       szDateTime[MAX_DATETIME_SIZE]; /**< DateTime String for Local Time */
    FILEINFO pstFILEINFO[MAX_FILETYPE_CNT]; /**< FILEINFO Memory */
} FILEMNG, *pFILEMNG;

extern void SetUpSignal(void);
extern void UserControlledSignal(S32 isign);
extern void IgnoreSignal(S32 isign);
extern S32 dInitMLOG(stMEMSINFO **pMEMSINFO);
extern void FinishProgram(pFILEMNG pstFILEMNG);
extern S32 dGetSYSCFG(void);
extern int dSendFileName(FILEINFO *pFILEINFO);
extern int dCloseFileMng(pFILEMNG pstFILEMNG);
extern int isRoam(int flag);
extern int dInitFileMng(pFILEMNG pstFILEMNG);
extern int dMakeFileMng(time_t tSetTime, pFILEMNG pstFILEMNG);
extern int dMapFileInfo(U32 dFileType, pFILEINFO *ppstFILEINFO, pFILEMNG pstFILEMNG);
extern void CILOG_LOG_WEB_P1(FILE *fp, LOG_RPPI *pthis);
extern S32 dConvertSIGNALtoDIAMETER(LOG_SIGNAL *pSig, DB_LOG_DIAMETER *pDia);



#endif /* __MLOG_INIT_H__ */
