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

#define INLINE_MAX		12		/* IN-LINE PORT ���� */
#define MONITOR_MAX		10		/* MONITOR PORT ���� */

#define MAX_CMD_NAME_LEN	512

// 0: X = �ƹ��͵� ����
// 1: CHG = ��ü 
// 2: A_CUT : ACTIVE SCE CUT OFF
// 3: S_CUT : STANDBY SCE CUT OFF
// 4: BYPASS : ��� SCE �� ������� 
// 5: ALL : ACTIVE SCE CUT OFF + ��ü 

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
  Duplication üũ�Ͽ� ��ü ���� 
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
	/* hwcom[x] Ȯ�� �غ��� ��. */ 
	int b4 = sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[3].status * 8; // e1000g3 SCMA
	int b3 = sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[5].status * 4; // e1000g5 SCMA
	int b2 = sfdb->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[3].status * 2; // e1000g3 SCMB
	int b1 = sfdb->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[5].status * 1; // e1000g5 SCMB

	myAction = b1+b2+b3+b4; // Action ���̺� index�� 

	// ACTIVE�� SCMA �ΰ�� Action ���̺� 1���� ã�´�. 
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
		// system ACT / STB üũ 
		act = checkActive(szActSys);
		if( act == 2 ) // Active / Standby ���� �� ��� szActSys�� ACTIVE SYSTEM ���� �ִ�. 
		{
			if(!strncmp(szActSys, "SCMA", 4))
				index = 0;
			else 
				index = 1;

			// ����ȭ ��ü ���� üũ 
			// ACTIVE �ý��� index���� ���� �� �ý����� TAPPING ���°���  �̿��Ͽ� Action�� �����Ѵ�. 
			result = dupCheck(index);	
			fprintf(fp,"\n ====== START DUP CHECK ===========\n");
			switch (result)
			{
				case X : 		// NOTHING
					break;
				case CHG : 		// SCMA <-> SCMB ��ü 
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
				case ALL : 		// ACTIVE SCE CUT OFF + SCMA <-> SCMB ��ü 
//					system(actCmd[index][A_CUT]);
//					system(actCmd[index][CHG]);
					fprintf(fp,"ALL: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][A_CUT]);
					fprintf(fp,"ALL: SYS[%d],ACT[%d],CMD[%d]\n",index,result,actCmd[index][CHG]);
					break;
				default:
					break;
			}
		}
		else if( act == 4 ) // Dual Standby ���� 
		{
			fprintf(fp,"!! DUAL STANDBY !!\n");
		}
		else if( act == 1 ) // Dual Active ���� 
		{
			fprintf(fp,"!! DUAL ACTIVE !!\n");
		}
		else if( act == -1 ) // ������ UnKnown ���� , ������ Active ���� 
		{
			fprintf(fp,"!! UNKNOWN STATUS + ACTIVE  !!\n");
		}
		else if( act == -2 ) // ������ UnKnown ���� , ������ Standby ����  
		{
			fprintf(fp,"!! UNKNOWN STATUS + STANDBY  !!\n");
		}
		fprintf(fp,"========= END DUP CHECK =============\n");

		commlib_microSleep(1000000);  
		fclose(fp);
	}
}

/*-------------------------------------------------------------------------------------------------
  ��ü ������ üũ�Ѵ�. ( 0 : alive, 1 : dead )
  -------------------------------------------------------------------------------------------------
  scma1	scmb1		scma2	scmb2
  ^		^			^		^
  |		|			|		|
  Director A		Director B	< director ��Ʈ ���°��� �������� �� ��� > 
  a		a'			b	 	b'
  ----------------------------
  0		0			0		0						: X
  0		0			0		1		ACTIVE B �϶�  	: ��ü ���� ��Ʈ�� ���� ���
  0		0			1		0				 		: X
  0		0			1		1				 		: SCEB CUT OFF
  0		1			0		0				 		: ��ü ���� ��Ʈ�� ���� ��� 
  0		1			0		1				 		: ��ü �� ��Ʈ �� ���� ��� 
  0		1			1		0				 		: ���� �ϳ��� ���� ��� STAND BY SCE A CUT OFF
  0		1			1		1				 		: SCEB CUT OFF, ��ü �� ��Ʈ �� ���� ��� 
  1		0			0		0				 		: X
  1		0			0		1				 		: ���� �ϳ��� ���� ��� STAND BY SCE A CUT OFF
  1		0			1		0				 		: X
  1		0			1		1				 		: SCEB CUT OFF
  1		1			0		0				 		: ��ü �ϸ� �ȵ�. !!!
  1		1			0		1				 		: ��ü �� ��Ʈ �� ���� ��� ,A�� ���� SCEA CUT OFFed
  1		1			1		0				 		: ��ü �ϸ� �ȵ�. !!!
  -------------------------------------------------------------------------------------------------------
  0		0			0		0						0 : X
  0		0			0		1						0 : X
  0		0			1		0		ACTIVE A �� ��  0 : ��ü ���� ��Ʈ�� ���� ��� 
  0		0			1		1					 	0 : ��ü �ϸ� �ȵ�. !!! 
  0		1			0		0						0 : X
  0		1			0		1						0 : X
  0		1			1		0						: ���� �ϳ��� ���� ��� STAND BY SCE B CUT OFF
  0		1			1		1						: ��ü �ϸ� �ȵ�. !!!
  1		0			0		0						: ��ü ���� ��Ʈ�� ���� ��� 
  1		0			0		1						: ���� �ϳ��� ���� ��� STANDBY SCE B CUT OFF
  1		0			1		0				 		: ��ü �� ��Ʈ ��� ���� ���
  1		0			1		1				 		: ��ü �� ��Ʈ ��� ���� ��� ,B�� ���� SCEB CUT OFFed
  1		1			0		0				 		2 : SCEA CUT OFF
  1		1			0		1				 		2 : SCEA CUT OFF, 
  1		1			1		0				 		5 : SCEA CUT OFF, ��ü �� ��Ʈ �� ���� ���
  ----------------------------
  1		1			1		1				 		: By Pass ���� ��ȯ 
-------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------
  dir1	dir2		dir1	dir2	sys[1].commInfo.systemDup.myStatus (SCMA) : 1 : ACT, 2 : STBY
  |		|			|		|		sys[2].commInfo.systemDup.myStatus (SCMB) : 1 : ACT, 2 : STBY
  v		v			v		v
-------------------------------		sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcomm[k].status : 0 live
| SCM	A			SCM 	B  |	< SCM �� TAPPING ��Ʈ ���� ���� �������� �� ��� > 	
| a		b			a'	 	b' |
-------------------------------		sfdb->sys[1].specInfo.u.sms.hpuxHWInfo.hwcomm[k].status : 1 down

-------------------------------------------------------------------------------------------------
 b4		b3			b2		b1
-------------------------------------------------------------------------------------------------
  0		0			0		0				 		: X
  0		0			0		1		ACTIVE B 	  	: ��ü (���� ��Ʈ�� ���� ���)
  0		0			1		0				 		: ��ü (���� ��Ʈ�� ���� ��� )
  0		0			1		1				 		: ��ü ( �� ��Ʈ �� ���� ��� )
  0		1			0		0				 		: X
  0		1			0		1				 		: SCE B CUT OFF ( Director down )
  0		1			1		0				 		: STAND BY SCE A CUT OFF (���� �ϳ��� �������)
  0		1			1		1				 		: SCEB CUT OFF + ��ü ( �� ��Ʈ �� ���� ��� )
  1		0			0		0				 		: X
  1		0			0		1				 		:  STAND BY SCE A CUT OFF(���� �ϳ��� ���� ���)
  1		0			1		0				 		: STAND BY SCE A CUT OFF(���� �ϳ��� ���� ��� )
  1		0			1		1				 		: ��ü (�� ��Ʈ �� ���� ��� ),A�� ���� SCEA CUT OFFed
  1		1			0		0				 		: X
  1		1			0		1				 		: SCE B CUT OFF ( Director down )
  1		1			1		0				 		: X ( �Ǵ� SCE A�� CUT OFF )
  1		1			1		1				 		: By Pass ���� ��ȯ 
-------------------------------------------------------------------------------------------------
  0		0			0		0		ACTIVE A 		: X
  0		0			0		1						: X
  0		0			1		0						: X
  0		0			1		1						: X
  0		1			0		0		 				: ��ü ���� ��Ʈ�� ���� ��� 
  0		1			0		1						: ���� �ϳ��� ���� ��� STAND BY SCE B CUT OFF
  0		1			1		0						: ���� �ϳ��� ���� ��� STAND BY SCE B CUT OFF
  0		1			1		1						: X ( �Ǵ� SCE B�� CUT OFF )
  1		0			0		0				 		: ��ü ���� ��Ʈ�� ���� ��� 
  1		0			0		1				 		: ���� �ϳ��� ���� ��� STAND BY SCE B CUT OFF
  1		0			1		0				 		: SCEA CUT OFF
  1		0			1		1				 		: SCEA CUT OFF
  1		1			0		0				 		: ��ü �� ��Ʈ �� ���� ���
  1		1			0		1				 		: ��ü �� ��Ʈ ������. B�� ���� SCEB CUT OFF
  1		1			1		0				 		: SCEA CUT OFF, ��ü �� ��Ʈ �� ���� ���
  1		1			1		1				 		: By Pass ���� ��ȯ 
-------------------------------------------------------------------------------------------------*/
/* �� ���ǿ� ���� ���ؾ��� ACTION�� ���� 
int	actRule[2][16] = {{X, X, X, X, CHG, S_CUT, S_CUT, X, CHG, S_CUT, A_CUT, A_CUT, CHG, CHG, ALL, BYPASS}, 
					 { X, CHG, CHG, CHG, X, A_CUT, S_CUT, ALL, X, S_CUT, S_CUT, CHG, X, A_CUT, X, BYPASS} };
*/
