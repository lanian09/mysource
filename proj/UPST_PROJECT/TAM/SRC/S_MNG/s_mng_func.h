#ifndef __S_MNG_FUNC_H__
#define __S_MNG_FUNC_H__

#include <mysql/mysql.h>	/* MYSQL */

#include "typedef.h"
#include "common_stg.h"
#include "filter.h"
#include "msgdef.h"			/* pst_MsgQ */
#include "mmcdef.h"			/* MAX_TMR_REC */
#include "db_struct.h"


#define DEF_IMSI_MIN_LEN		10
#define	DEF_IMSI_PATTERN_LEN	4
#define	DEF_IMSI_PATTERN_SIZE	(DEF_IMSI_PATTERN_LEN + 1)
#define	DEF_IMSI_PREFIX_LEN		5
#define	DEF_IMSI_PREFIX_SIZE	(DEF_IMSI_PREFIX_LEN + 1)

typedef struct _st_IMSIHash_Key {
	U8				sIMSI[DEF_IMSI_PATTERN_SIZE];
	U8				sReserved[3];
} st_IMSIHash_Key;
#define	DEF_IMSIHASH_KEY_SIZE		sizeof(st_IMSIHash_Key)

typedef struct _st_IMSIHash_Data {
	U8				sIRM[DEF_IMSI_PATTERN_SIZE];
	U8				sPrefix[DEF_IMSI_PREFIX_SIZE];
	U8				sReserved[5];
} st_IMSIHash_Data;
#define DEF_IMSIHASH_DATA_SIZE		sizeof(st_IMSIHash_Data)

#define MAX_MODEL_COUNT 1000
typedef struct _st_Model{
    U8  szModel[MAX_MODEL_SIZE];
    U32 uiCount;
} st_Model;

typedef struct _st_Model_Stat{
    st_Model stModel[MAX_MODEL_COUNT];
    U32 uiModelCnt;
} st_Model_Stat;

typedef struct _st_Mmlsg
{
    unsigned short src_func[MAX_TMR_REC];
    unsigned short src_proc[MAX_TMR_REC];
    unsigned short cmd_id[MAX_TMR_REC];
    unsigned short msg_id[MAX_TMR_REC];
}st_Mmlsg , *pst_Mmlsg;


extern int dRcv_Alm(pst_MsgQ pstMsgQ, USHORT usMID);
extern int Apply_Info_Equip(st_Info_Equip_List *pstInfoEquipList);
extern int Apply_Filter_Tmf(st_Tmf_Info *pstTmfInfo);
extern int Send_Chk(st_MsgQ *pstMsgQ);
extern int Apply_Filter_SvcInfo(st_SvcInfo_List *pstSvcInfoList);
extern int Apply_Filter_NAS(st_NAS_List *pstNASList);
extern int Apply_Filter_SCTP(st_SCTP_MMC *pstSCTPList);
extern int Apply_Sys_User_Info(st_User_Add_List *pstUserList);
extern int Apply_Filter_Thres(st_Thres_List *pstThresList);
extern int dSendTrcToNtaf(int dSysNo);
extern int dSendTimerToNtaf(int dSysNo);
extern int dReadTimerFile(TIMER_INFO *pstData);
extern int dWriteTimerFile(TIMER_INFO *pstData);
extern int dMakeIRMHash(void);
extern int dConvertIMSItoIRM(st_TraceList *pstTraceList);
extern int dSend_FltMsg_To_SubSystem(pst_MsgQ pstMsgQ, int dMid, int dSid, int dSysNo);

/* 단말 모델에 대한 가입자별 비율 표시를 위한 함수 */
extern int dCreateTBSql_ModelCustCnt(st_Model_Stat *postMS);
extern int dRecordModelCustCnt(st_Model_Stat *pstMS, char *pszModel, int len);

extern int dSendMsg_O_SVCMON(unsigned short huMsgID);
extern int dSendRPPI(unsigned short uhMsgID);
extern int dSendROAM(unsigned short uhMsgID);
extern int dSendMsg_SIDB(char *sDate, char *sFullPath, char *sFileName);
extern int dSendMTRACE(unsigned short uhMsgID);
extern int dReadFltSHM(st_MsgQ *pstMsgQ);
extern int dWriteFltSHM(st_MsgQ *pstMsgQ);
extern int dResChkInfo(pst_MsgQ pstMsgQ);
extern int dSend_NTAF_AlarmLevel(int dSysNo);
extern int dSendMsg_SI_SVCMON(char *sDate, unsigned char *sFullPath, unsigned char *sFileName);
extern int dCopy(char *sSourcePath, char *sDestinationPath);
extern int dSend_FltSCTP_Data(int dSysNo);
extern int dSend_FltSvc_Data(int dSysNo);
extern int dSend_FltIPPool_Data(int dSysNo);

#endif /* __S_MNG_FUNC_H__ */
