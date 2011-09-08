/**
	@file		cmd_get.h
	@author
	@date		2011-07-14
	@version	
	@brief		cmd_get.c 헤더파일
*/

#ifndef __CMD_GET_H__
#define __CMD_GET_H__

/**
	Include Headers
*/
// DQMS
#include "mmcdef.h"
#include "cmd_user.h"
#include "cmd_load.h"

#define MAX_TMR_REC				128
#define MAX_ENUM_LIST_ELEM		2048

#define MAX_PARSTR_LEN			64
#define TIME_OUT                25

typedef struct _RUN_TBL
{
	long long		llIndex[MAX_TMR_REC];
	short			exe_proc[MAX_TMR_REC];
	short			cmd_id[MAX_TMR_REC];
	short			msg_id[MAX_TMR_REC];
	short			sfd[MAX_TMR_REC];
	char			time_stamp[MAX_TMR_REC][40];
	char			user_name[MAX_TMR_REC][24];
	short			inv_proc[MAX_TMR_REC];
	short			time[MAX_TMR_REC];			/* 0이상이면 현재 실행중인 명령 */
	char			type[MAX_TMR_REC][12];
	short			period[MAX_TMR_REC];
	short			stat_TOTAL[MAX_TMR_REC];	/* 통계 결과의 전체 자료의 개수 */
	short			stat_TOT_NUM[MAX_TMR_REC];	/* 통계 결과의 변하지 않는 전체 개수
*/
	short			stat_tot_cnt[MAX_TMR_REC];	/* 하나의 통계 자료에 대한 전체 RECORD
 수 */
	short			stat_cur_cnt[MAX_TMR_REC];	/* 하나의 통계 자료에 대한 현재 RECORD
 수 */
	short			stat_tot_page[MAX_TMR_REC]; /* 하나의 통계 자료에 대한 전체 PAGE 수*/
	short			stat_cur_page[MAX_TMR_REC]; /* 하나의 통계 자료에 대한 현재 PAGE 수*/
	unsigned short	usmmlid[MAX_TMR_REC];
	short			blockcode[MAX_TMR_REC];	 /* 해당 프로세스의 Message Queue ID
*/
	unsigned char	ucbinflag[MAX_TMR_REC];
	time_t			stat_check_time[MAX_TMR_REC];	/* STAT CHECK TIME */
	mml_msg			stMML_MSG[MAX_TMR_REC];
	char			szCommandString[MAX_TMR_REC][256];
} RUN_TBL;


/**
 *	Declare functions
 */
extern char *pv_range_check(char *str, PARA_CONT *para);
extern char *cmd_get(In_Arg in_para[], char *outstr, mml_msg *mml, COM_TBL **cmd_ret);
extern int SetTimer(int esfd, mml_msg *snd_msg, COM_TBL *cmdptr, In_Arg in_para[]);
extern int isdigitstr(char *str);


#endif	/* __CMD_GET_H__ */
