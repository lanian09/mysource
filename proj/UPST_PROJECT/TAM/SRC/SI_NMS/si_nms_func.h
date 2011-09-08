#ifndef _SI_NMS_FUNC_H_
#define _SI_NMS_FUNC_H_

/**
 *	Define constants
 */
#define	FILE_NAME_LEN				80
#define	FILE_NUM_5MIN				576		/*	5분 단위 3시간 => 48시간: 2006.08.29		*/
#define	FILE_NUM_HOUR				168		/*	1시간 단위 48시간 => 1주일: 2006.08.29		*/

#define EQUIP_MAX_COUNT				9

/**
 *	Declare functions
 */
extern int dGetIPAddr(char *conf_file, char *primary_addr, int dMaxLen);
extern int dValidIP(char *IPaddr);
extern int dIsReceivedMessage(pst_MsgQ pstMsgQ);
extern int dSaveFileList(char *FileName, int dSfd, int dPeriod);
extern int dMakeOIDFile(MYSQL *pstMySQL, st_atQueryInfo *pstAtQueryInfo, char *sFileName);
extern int dGetABSMinute(int r_min);
extern int dGetOidTime(int dPeriodic, char *sOidTime);
extern int dReadNTAFName(st_NTafName *pstNTafName);
extern void DisplayFileList(void);
extern struct tm *stTmNow(void);
extern void GetAlarmStr(char *sAlias, unsigned char ucLocType, unsigned char ucInvType, char *psBuf);
extern int dCheck_Channel(int hdFlag, unsigned int uiIP);

#endif	/* _SI_NMS_FUNC_H_ */
