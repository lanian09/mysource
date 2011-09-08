#include "samd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

void dup_Thread(void);

void *run_DupThread(void*);

extern SFM_L3PD     *l3pd;
extern SFM_sfdb     *sfdb;

extern pthread_mutex_t  mutex;

#define INLINE_MAX		12		/* IN-LINE PORT 갯수 */
#define MONITOR_MAX		10		/* MONITOR PORT 갯수 */

#define MAX_CMD_NAME_LEN	512

// 0: X = 아무것도 안함
// 1: CHG = 절체 
// 2: A_CUT : ACTIVE SCE CUT OFF
// 3: S_CUT : STANDBY SCE CUT OFF
// 4: BYPASS : 모든 SCE 가 죽은경우 
// 5: ALL : ACTIVE SCE CUT OFF + 절체 

enum	ACTION  {X=0,CHG,A_CUT,S_CUT,BYPASS,ALL};
int	actRule[2][16] = {{X, X, X, X, CHG, S_CUT, S_CUT, X, CHG, S_CUT, A_CUT, A_CUT, CHG, CHG, ALL, BYPASS}, 
					 { X, CHG, CHG, CHG, X, A_CUT, S_CUT, ALL, X, S_CUT, S_CUT, CHG, X, A_CUT, X, BYPASS} };

char	*actCmd[2][MAX_CMD_NAME_LEN] = {
	{	"",
	"rsh -l root SCMA /opt/VRTSvcs/bin/hagrp -switch SCM_SG -to SCMB",
	"/DSC/SCRIPT/set-scea-cutoff-mode.sh",
	"/DSC/SCRIPT/set-sceb-cutoff-mode.sh",
	"/DSC/SCRIPT/set-scea-bypass-mode.sh",
	"/DSC/SCRIPT/set-sceb-bypass-mode.sh"
	},
	{	"",
	"rsh -l root SCMB /opt/VRTSvcs/bin/hagrp -switch SCM_SG -to SCMA",
	"/DSC/SCRIPT/set-sceb-cutoff-mode.sh",
	"/DSC/SCRIPT/set-scea-cutoff-mode.sh",
	"/DSC/SCRIPT/set-scea-bypass-mode.sh",
	"/DSC/SCRIPT/set-sceb-bypass-mode.sh"
	}
};
									
/*--------------------------------------------------------------
  Duplication 체크하여 절체 관리 
--------------------------------------------------------------*/
void dup_Thread(void)
{
    pthread_attr_t  thrAttr;
    pthread_t       thrId;
	//char trcBuf[1024] = {0x00,};
	int status;

    pthread_attr_init(&thrAttr);                                                   
    pthread_attr_setscope(&thrAttr, PTHREAD_SCOPE_SYSTEM);                         
    pthread_attr_setdetachstate(&thrAttr, PTHREAD_CREATE_DETACHED);                
    if(pthread_create(&thrId, &thrAttr, run_DupThread, NULL)) {                    
        sprintf (trcBuf, "[%s] Duplication pthread_create fail\n",__FUNCTION__);                     
        trclib_writeLogErr (FL,trcBuf);                                            
    }
	pthread_join(thrId, (void *) &status);
    pthread_attr_destroy(&thrAttr);
	return;

}

int	checkActive(char *actSys)
{
	int	ret1 = 0, ret2 = 0;
	switch (sfdb->sys[1].commInfo.systemDup.myStatus )
	{
		case 1 : 
			sprintf(actSys, "SCMA");
			ret1 = 1;
			break;
		case 2 :
			ret1 = 2;
			break;
		default:
			ret1 = -1;
	}
	switch (sfdb->sys[2].commInfo.systemDup.myStatus )
	{
		case 1 : 
			sprintf(actSys, "SCMB");
			ret2 = 1;
			break;
		case 2 :
			ret2 = 2;
			break;
		default:
			ret2 = -1;
	}
	return ret1*ret2;
}

int dupCheck(int index)	
{
	int	myAction = 0;
//	[MSB] b4 b3 b2 b1 [LSB]
	/* hwcom[x] 확인 해봐야 함. */ 
	int b4 = sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[3].status * 8; // e1000g3 SCMA
	int b3 = sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[5].status * 4; // e1000g5 SCMA
	int b2 = sfdb->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[3].status * 2; // e1000g3 SCMB
	int b1 = sfdb->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[5].status * 1; // e1000g5 SCMB

	myAction = b1+b2+b3+b4; // Action 테이블 index값 

	// ACTIVE가 SCMA 인경우 Action 테이블 1에서 찾는다. 
	if( index == 1 )
		return actRule[0][myAction];
	else
		return actRule[1][myAction];
}

void *run_DupThread(void *arg)
{
	int 	result = 0;
	int		index = 0;
	int		act = 0;
	char	szActSys[SYS_NAME_LEN] = {0,};
	FILE 	*fp = NULL;

	while(1)
	{
		fp = fopen("./DUP_LOG.txt", "w+");
		result = 0;
		act = 0;
		// system ACT / STB 체크 
		act = checkActive(szActSys);
		if( act == 2 ) // Active / Standby 정상 인 경우 szActSys에 ACTIVE SYSTEM 명이 있다. 
		{
			if(!strncmp(szActSys, "SCMA", 4))
				index = 0;
			else 
				index = 1;

			// 이중화 절체 조건 체크 
			// ACTIVE 시스템 index값을 보고 두 시스템의 TAPPING 상태값을  이용하여 Action을 결정한다. 
			result = dupCheck(index);	
			fprintf(fp,"\n ====== START DUP CHECK ===========\n");
			switch (result)
			{
				case X : 		// NOTHING
					break;
				case CHG : 		// SCMA <-> SCMB 절체 
//					system(actCmd[index][result]);
					fprintf(fp,"CHG: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][result]);
					break;
				case A_CUT : 	// ACTIVE SCE CUT OFF
//					system(actCmd[index][result]);
					fprintf(fp,"ACTIVE CUTOFF: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][result]);
					break;
				case S_CUT : 	// STANDBY SCE CUT OFF
//					system(actCmd[index][result]);
					fprintf(fp,"STANDBY CUTOFF: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][result]);
					break;
				case BYPASS : 	// ALL SCE BY PASS
//					system(actCmd[index][BYPASS]);
//					system(actCmd[index][BYPASS+1]);
					fprintf(fp,"BYPASS: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][BYPASS]);
					fprintf(fp,"BYPASS: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][BYPASS+1]);
					break;
				case ALL : 		// ACTIVE SCE CUT OFF + SCMA <-> SCMB 절체 
//					system(actCmd[index][A_CUT]);
//					system(actCmd[index][CHG]);
					fprintf(fp,"ALL: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][A_CUT]);
					fprintf(fp,"ALL: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][CHG]);
					break;
				default:
					break;
			}
		}
		else if( act == 4 ) // Dual Standby 상태 
		{
			fprintf(fp,"!! DUAL STANDBY !!\n");
		}
		else if( act == 1 ) // Dual Active 상태 
		{
			fprintf(fp,"!! DUAL ACTIVE !!\n");
		}
		else if( act == -1 ) // 한쪽이 UnKnown 상태 , 한쪽이 Active 상태 
		{
			fprintf(fp,"!! UNKNOWN STATUS + ACTIVE  !!\n");
		}
		else if( act == -2 ) // 한쪽이 UnKnown 상태 , 한쪽이 Standby 상태  
		{
			fprintf(fp,"!! UNKNOWN STATUS + STANDBY  !!\n");
		}
		fprintf(fp,"========= END DUP CHECK =============\n");

		commlib_microSleep(1000000);  
		fclose(fp);
	}
}

/*-------------------------------------------------------------------------------------------------
  절체 조건을 체크한다. ( 0 : alive, 1 : dead )
  -------------------------------------------------------------------------------------------------
  scma1	scmb1		scma2	scmb2
  ^		^			^		^
  |		|			|		|
  Director A		Director B	< director 포트 상태값을 기준으로 할 경우 > 
  a		a'			b	 	b'
  ----------------------------
  0		0			0		0						: X
  0		0			0		1		ACTIVE B 일때  	: 절체 한쪽 포트만 죽은 경우
  0		0			1		0				 		: X
  0		0			1		1				 		: SCEB CUT OFF
  0		1			0		0				 		: 절체 한쪽 포트만 죽은 경우 
  0		1			0		1				 		: 절체 두 포트 다 죽은 경우 
  0		1			1		0				 		: 서로 하나씩 죽은 경우 STAND BY SCE A CUT OFF
  0		1			1		1				 		: SCEB CUT OFF, 절체 두 포트 다 죽은 경우 
  1		0			0		0				 		: X
  1		0			0		1				 		: 서로 하나씩 죽은 경우 STAND BY SCE A CUT OFF
  1		0			1		0				 		: X
  1		0			1		1				 		: SCEB CUT OFF
  1		1			0		0				 		: 절체 하면 안됨. !!!
  1		1			0		1				 		: 절체 두 포트 다 죽은 경우 ,A로 가면 SCEA CUT OFFed
  1		1			1		0				 		: 절체 하면 안됨. !!!
  -------------------------------------------------------------------------------------------------------
  0		0			0		0						0 : X
  0		0			0		1						0 : X
  0		0			1		0		ACTIVE A 일 때  0 : 절체 한쪽 포트만 죽은 경우 
  0		0			1		1					 	0 : 절체 하면 안됨. !!! 
  0		1			0		0						0 : X
  0		1			0		1						0 : X
  0		1			1		0						: 서로 하나씩 죽은 경우 STAND BY SCE B CUT OFF
  0		1			1		1						: 절체 하면 안됨. !!!
  1		0			0		0						: 절체 한쪽 포트만 죽은 경우 
  1		0			0		1						: 서로 하나씩 죽은 경우 STANDBY SCE B CUT OFF
  1		0			1		0				 		: 절체 두 포트 모두 죽은 경우
  1		0			1		1				 		: 절체 두 포트 모두 죽은 경우 ,B로 가면 SCEB CUT OFFed
  1		1			0		0				 		2 : SCEA CUT OFF
  1		1			0		1				 		2 : SCEA CUT OFF, 
  1		1			1		0				 		5 : SCEA CUT OFF, 절체 두 포트 다 죽은 경우
  ----------------------------
  1		1			1		1				 		: By Pass 모드로 전환 
-------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------
  dir1	dir2		dir1	dir2	sys[1].commInfo.systemDup.myStatus (SCMA) : 1 : ACT, 2 : STBY
  |		|			|		|		sys[2].commInfo.systemDup.myStatus (SCMB) : 1 : ACT, 2 : STBY
  v		v			v		v
-------------------------------		sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcomm[k].status : 0 live
| SCM	A			SCM 	B  |	< SCM 의 TAPPING 포트 정보 값을 기준으로 할 경우 > 	
| a		b			a'	 	b' |
-------------------------------		sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcomm[k].status : 1 down

-------------------------------------------------------------------------------------------------
 b4		b3			b2		b1
-------------------------------------------------------------------------------------------------
  0		0			0		0				 		: X
  0		0			0		1		ACTIVE B 	  	: 절체 (한쪽 포트만 죽은 경우)
  0		0			1		0				 		: 절체 (한쪽 포트만 죽은 경우 )
  0		0			1		1				 		: 절체 ( 두 포트 다 죽은 경우 )
  0		1			0		0				 		: X
  0		1			0		1				 		: SCE B CUT OFF ( Director down )
  0		1			1		0				 		: STAND BY SCE A CUT OFF (서로 하나씩 죽은경우)
  0		1			1		1				 		: SCEB CUT OFF + 절체 ( 두 포트 다 죽은 경우 )
  1		0			0		0				 		: X
  1		0			0		1				 		:  STAND BY SCE A CUT OFF(서로 하나씩 죽은 경우)
  1		0			1		0				 		: STAND BY SCE A CUT OFF(서로 하나씩 죽은 경우 )
  1		0			1		1				 		: 절체 (두 포트 다 죽은 경우 ),A로 가면 SCEA CUT OFFed
  1		1			0		0				 		: X
  1		1			0		1				 		: SCE B CUT OFF ( Director down )
  1		1			1		0				 		: X ( 또는 SCE A를 CUT OFF )
  1		1			1		1				 		: By Pass 모드로 전환 
-------------------------------------------------------------------------------------------------
  0		0			0		0		ACTIVE A 		: X
  0		0			0		1						: X
  0		0			1		0						: X
  0		0			1		1						: X
  0		1			0		0		 				: 절체 한쪽 포트만 죽은 경우 
  0		1			0		1						: 서로 하나씩 죽은 경우 STAND BY SCE B CUT OFF
  0		1			1		0						: 서로 하나씩 죽은 경우 STAND BY SCE B CUT OFF
  0		1			1		1						: X ( 또는 SCE B를 CUT OFF )
  1		0			0		0				 		: 절체 한쪽 포트만 죽은 경우 
  1		0			0		1				 		: 서로 하나씩 죽은 경우 STAND BY SCE B CUT OFF
  1		0			1		0				 		: SCEA CUT OFF
  1		0			1		1				 		: SCEA CUT OFF
  1		1			0		0				 		: 절체 두 포트 다 죽은 경우
  1		1			0		1				 		: 절체 두 포트 다죽음. B로 가면 SCEB CUT OFF
  1		1			1		0				 		: SCEA CUT OFF, 절체 두 포트 다 죽은 경우
  1		1			1		1				 		: By Pass 모드로 전환 
-------------------------------------------------------------------------------------------------*/
/* 위 조건에 대해 취해야할 ACTION을 정의 
int	actRule[2][16] = {{X, X, X, X, CHG, S_CUT, S_CUT, X, CHG, S_CUT, A_CUT, A_CUT, CHG, CHG, ALL, BYPASS}, 
					 { X, CHG, CHG, CHG, X, A_CUT, S_CUT, ALL, X, S_CUT, S_CUT, CHG, X, A_CUT, X, BYPASS} };
*/
