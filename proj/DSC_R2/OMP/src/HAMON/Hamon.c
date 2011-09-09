#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <pthread.h>
#include <strings.h>

#include "loglib.h"
#include "sfmconf.h"
#include "sfm_snmp.h"
#include "comm_msgtypes.h"
#include "keepalivelib.h"
#include "commlib.h"

#define MYAPP	"HAMON"
#define MYVER	"R2.0.0" // "R1.1.0" -> "R2.0.0", 602 line if((fp..) !=NULL) 변경.
#define MYLOG   "LOG/OAM/hamon_log"                                                                  
#define MYERR   "LOG/OAM/hamon_err" 
#define MYCFG   "DATA/sysconfig"                                                                       
#define MYPID	10

#define TIMER	1000*5	// (msec)
#define SFMSZ	sizeof(SFM_sfdb)
#define SCESZ	sizeof(SFM_SCE)
#define BUFSIZE	1024*8


/** hamon 재기동 시 SetMaster로 SCMA로 절체 되는 문제점 보완 */
#define SCM_A	0
#define SCM_B	1
#define SCE_A	0
#define SCE_B	1
/** hamon 재기동 시 SetMaster로 SCMA로 절체 되는 문제점 보완 */

char 	MYSYS[BUFSIZE];
int  	MYQID, IXQID, MCQID;
int		LOGID;
int  	SFMID;
int  	SCEID;
T_keepalive KeepAlive;

SFM_sfdb	*pSFM = NULL;
SFM_SCE     *pSCE = NULL;

/** hamon 재기동 시 SetMaster로 SCMA로 절체 되는 문제점 보완 */
#define IDX_SCMA	1 // sfdb 의 인덱스 
#define IDX_SCMB	2 

#define ACTIVE		1
#define STANDBY		2
#define UNKNOWN		3

int	 	Master 		= SCM_A;
int	CheckMaster(void);
/** hamon 재기동 시 SetMaster로 SCMA로 절체 되는 문제점 보완 */

void hamonInit ();
void hamonTerm ();

extern void cbSetTimer(void (*func)(int, void*), int key, void* data, unsigned int msec);
extern int set_proc_version();

/** 위치가 좋지 않다. 0717 
#define SCM_A	0
#define SCM_B	1
#define SCE_A	0
#define SCE_B	1
*/

#define INIT			hamonInit ()
#define TERM			hamonTerm (__FILE__, __LINE__)
#define	LOG				logPrint

char _val[BUFSIZE];
pthread_t t_thCheckHA;

#define GETQKEY(p,s,x)	do { if(conflib_getNthTokenInFileSection(p,"APPLICATIONS",s,1,_val) >= 0) x=strtol(_val,0,0); } while(0);              
#define GETMKEY(p,s,x)	do { if(conflib_getNthTokenInFileSection(p,"SHARED_MEMORY_KEY",s,1,_val) >= 0) x=strtol(_val,0,0); } while(0);              

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	INITIALIZE FUNCTION
//
void SetMaster(int whose);

int my_system(const char *cmd)
{
	FILE *p;

	if ((p = popen(cmd, "w")) == NULL)
		return (-1);
	return (pclose(p));
}


void UserControlledSignal(int sign)
{
	TERM;
    exit(0);
}

void hamonInit ()
{
	char *env, path[BUFSIZE];
	int loopCnt, key;

	for(loopCnt=0; loopCnt<=255; loopCnt++) {
		switch(loopCnt) {
		case SIGTERM:
		case SIGINT :
		case SIGQUIT: signal(loopCnt, UserControlledSignal);	break;
		default		: sigignore(loopCnt);						break;
		}
	}

	setenv("LANG", "C", 1);
	if (!(env=getenv("MY_SYS_NAME"))) 		TERM; else strcpy (MYSYS, env);
	if (!(env=getenv("IV_HOME"))) 			TERM;
	if (set_proc_version(MYPID, MYVER)<0)	TERM;

    sprintf(path, "%s/%s.%s", env, MYLOG, MYSYS);
    if ((LOGID=loglib_openLog (path
		, LOGLIB_MODE_LIMIT_SIZE | LOGLIB_FLUSH_IMMEDIATE | LOGLIB_TIME_STAMP | LOGLIB_FNAME_LNUM)) < 0) TERM;

    sprintf(path, "%s/%s", env, MYCFG);
	GETQKEY(path, "HAMON",key); if((MYQID=msgget (key,IPC_CREAT|0666))<0) TERM;
	GETQKEY(path, "IXPC", key); if((IXQID=msgget (key,IPC_CREAT|0666))<0) TERM;
	GETQKEY(path, "MCDM", key); if((MCQID=msgget (key,IPC_CREAT|0666))<0) TERM;

	GETMKEY(path, "SHM_SFDB", key); 
	if((SFMID=(int)shmget(key,SFMSZ,IPC_CREAT|0666))>=0) if(!(pSFM=(SFM_sfdb*)shmat (SFMID,0,0))) TERM;

	GETMKEY(path, "SHM_SCE", key); 
	if((SCEID=(int)shmget(key,SCESZ,IPC_CREAT|0666))>=0) if(!(pSCE=(SFM_SCE*)shmat (SCEID,0,0))) TERM;

	if(keepalivelib_init("HAMON") < 0) TERM;
	bzero(&KeepAlive, sizeof(T_keepalive));

	LOG (LOGID, FL, "====================================\n");
	LOG (LOGID, FL, "%s PROCESS [SERVICE-READY] #####    \n", MYAPP);
	LOG (LOGID, FL, "====================================\n");

	/** hamon 재기동 시 SetMaster로 SCMA로 절체 되는 문제점 보완 */
	Master = CheckMaster();
	// Master 가 불명확한 경우에만 Master를 SCM_A로 셋팅하고 SetMaster를 호출한다. 
	if( Master == UNKNOWN )
	{
		Master = SCM_A;
		SetMaster(Master);
	}
	/** hamon 재기동 시 SetMaster로 SCMA로 절체 되는 문제점 보완 */
// SetMaster(SCM_A);
}

/** hamon 재기동 시 SetMaster로 SCMA로 절체 되는 문제점 보완 */
int	CheckMaster(void)
{
#if 0
	fprintf(stderr, "scma : %d / %d , scmb : %d / %d\n",
			pSFM->sys[IDX_SCMA].commInfo.systemDup.myStatus,
			pSFM->sys[IDX_SCMA].commInfo.systemDup.yourStatus,
			pSFM->sys[IDX_SCMB].commInfo.systemDup.myStatus,
			pSFM->sys[IDX_SCMB].commInfo.systemDup.yourStatus);
#endif
	// 명확하게 sfdb 구조체에 act/standby에 대한 값이 셋팅되어 있다면 
	// Master 를 셋팅하고 절체를 시키지 않는다 . 
	if( (pSFM->sys[IDX_SCMA].commInfo.systemDup.myStatus == ACTIVE) &&
			(pSFM->sys[IDX_SCMB].commInfo.systemDup.myStatus == STANDBY) )
	{
		Master = SCM_A;
	}
	else if( (pSFM->sys[IDX_SCMA].commInfo.systemDup.myStatus == STANDBY) &&
			(pSFM->sys[IDX_SCMB].commInfo.systemDup.myStatus == ACTIVE))  

	{
		Master = SCM_B;
	}
	else
	{
		Master = UNKNOWN;
	}
	return Master;
}
/** hamon 재기동 시 SetMaster로 SCMA로 절체 되는 문제점 보완 */


void hamonTerm (char* f, int l)
{
	LOG (LOGID, FL, "====================================\n");
	LOG (LOGID, FL, "%s PROCESS [STOP]  #####            \n", MYAPP);
	LOG (LOGID, FL, "====================================\n");
}

// Gathering HA Information
void cbTaskHA (int k, void *d)
{
	keepalivelib_increase();
	cbSetTimer(cbTaskHA, 1, NULL, TIMER);
}

#define A_1	(((pSFM->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[3].status)<<3)&0x0F)	// e1000g3 SCMA
#define A_2	(((pSFM->sys[1].specInfo.u.sms.hpuxHWInfo.hwcom[5].status)<<2)&0x0F)	// e1000g5 SCMA
#define B_1	(((pSFM->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[3].status)<<1)&0x0F)	// e1000g3 SCMB
#define B_2	(((pSFM->sys[2].specInfo.u.sms.hpuxHWInfo.hwcom[5].status)<<0)&0x0F)	// e1000g5 SCMB

// EVENT
#define NOP			0
#define TAP_A_DN	1
#define TAP_A_UP	2
#define TAP_B_DN	3
#define TAP_B_UP	4
#define TAP_D_DN	5
#define TAP_D_UP	6
#define SCE_A_DN	7
#define SCE_B_DN	8
#define VCS			9

// STATUS
#define FWD_AB		1
#define ONLY_A		2
#define ONLY_B		3
#define NO_CTRL		4

#define CUTOFF		0
#define BYPASS		1

unsigned char Bit 	= 0xFF;
int  	State 		= NO_CTRL;
int		OnFail[2] 	= { BYPASS, BYPASS };
// 0717 jjinri 위치가 좋지 않다. int	 	Master 		= SCM_A;
time_t 	tEvent;

#define SCE_UP		2
#define PA(d)		(pSCE->SCEDev[0].portStatus[d+2].status)
#define PB(d)		(pSCE->SCEDev[1].portStatus[d+2].status)

#define bGET(b) { b=((b<<4)&0xF0); b|=(A_1 + A_2 + B_1 + B_2); }
#define bPRE(b)	( (b >> 4) & 0x0F )
#define bCUR(b)	( b & 0x0F )
#define PRE		bPRE(Bit)
#define CUR		bCUR(Bit)
#define IS_SCE_A_DOWN	((PA(0)!=SCE_UP && PA(1)!=SCE_UP && PA(2)!=SCE_UP && PA(3)!=SCE_UP)? 1:0)
#define IS_SCE_B_DOWN	((PB(0)!=SCE_UP && PB(1)!=SCE_UP && PB(2)!=SCE_UP && PB(3)!=SCE_UP)? 1:0)
#define IS_SCE_A_UP		((PA(0)==SCE_UP && PA(1)==SCE_UP && PA(2)==SCE_UP && PA(3)==SCE_UP)? 1:0)
#define IS_SCE_B_UP		((PB(0)==SCE_UP && PB(1)==SCE_UP && PB(2)==SCE_UP && PB(3)==SCE_UP)? 1:0)

#define bTAP_A(b)	((b>>2 & 0x02) | (b>>1 & 0x01))
#define bTAP_B(b)	((b>>1 & 0x02) | (b>>0 & 0x01))
#define pTAP_A		bTAP_A(PRE)
#define pTAP_B		bTAP_B(PRE)
#define cTAP_A		bTAP_A(CUR)
#define cTAP_B		bTAP_B(CUR)
#define nSCM_A		((CUR>>3 & 0x01) + (CUR>>2 & 0x01))
#define nSCM_B		((CUR>>1 & 0x01) + (CUR>>0 & 0x01))
#define UP			0x00
#define DN			0x03

#define SCE_A_OPER_BYPASS		0
#define SCE_B_OPER_BYPASS		1
#define SCE_A_OPER_CUTOFF		2
#define SCE_B_OPER_CUTOFF		3
#define SCE_A_OPER_FORWARD		4
#define SCE_B_OPER_FORWARD		5
#define SCE_A_FAIL_BYPASS		6
#define SCE_B_FAIL_BYPASS		7
#define SCE_A_FAIL_CUTOFF		8
#define SCE_B_FAIL_CUTOFF		9
#define SCE_A_RECOVERY_OP		10
#define SCE_B_RECOVERY_OP		11
#define VCS_A_MASTER			12
#define VCS_B_MASTER			13
#define MAX_SCE_OP				14

#define EVENT_TIME_VALID	5

typedef struct _OP_t {
	char 	Cmd[BUFSIZE];
	char 	Desc[BUFSIZE];
} OP_t;

OP_t OP[MAX_SCE_OP+1] = {
	{"/DSC/SCRIPT/set-sce.sh 0 link bypass",			"SET SCE#A LINK BYPASS"},
	{"/DSC/SCRIPT/set-sce.sh 1 link bypass",			"SET SCE#B LINK BYPASS"},
	{"/DSC/SCRIPT/set-sce.sh 0 link cutoff",			"SET SCE#A LINK CUTOFF"},
	{"/DSC/SCRIPT/set-sce.sh 1 link cutoff",			"SET SCE#B LINK CUTOFF"},
	{"/DSC/SCRIPT/set-sce.sh 0 link forwarding",		"SET SCE#A LINK FORWARDING"},
	{"/DSC/SCRIPT/set-sce.sh 1 link forwarding",		"SET SCE#B LINK FORWARDING"},
	{"/DSC/SCRIPT/set-sce.sh 0 onfail bypass",			"SET SCE#A ON-FAIL BYPASS"},
	{"/DSC/SCRIPT/set-sce.sh 1 onfail bypass",			"SET SCE#B ON-FAIL BYPASS"},
	{"/DSC/SCRIPT/set-sce.sh 0 onfail cutoff",			"SET SCE#A ON-FAIL CUTOFF"},
	{"/DSC/SCRIPT/set-sce.sh 1 onfail cutoff",			"SET SCE#B ON-FAIL CUTOFF"},
	{"/DSC/SCRIPT/set-sce.sh 0 recovery operational",	"SET SCE#A FAIL-RECOVERY OPERATIONAL"},
	{"/DSC/SCRIPT/set-sce.sh 1 recovery operational",	"SET SCE#B FAIL-RECOVERY OPERATIONAL"},
	{"/DSC/SCRIPT/set-vcs.sh 0",						"SET SCM#A VCS MASTER"},
	{"/DSC/SCRIPT/set-vcs.sh 1",						"SET SCM#B VCS MASTER"},
	{"",""}
};

#define SYSCMD(d)	do { 																\
	char _buf[BUFSIZE];																	\
	switch(d) {																			\
	case SCE_A_FAIL_CUTOFF: OnFail[SCE_A] = CUTOFF; break;								\
	case SCE_B_FAIL_CUTOFF: OnFail[SCE_B] = CUTOFF; break;								\
	case SCE_A_FAIL_BYPASS: OnFail[SCE_A] = BYPASS; break;								\
	case SCE_B_FAIL_BYPASS: OnFail[SCE_B] = BYPASS; break;								\
	}																					\
	if(OP[d].Cmd[0] != '\0') { sprintf(_buf,"%s 2>&1 ", OP[d].Cmd);  my_system (_buf); }				\
	LOG (LOGID, FL, "%s\n", OP[d].Desc); 												\
} while(0);

void DoPrnStat()
{
   	LOG (LOGID, FL, "LEG#A[%c%c|%c%c], LEG#B[%c%c|%c%c] (TAP#A[%c%c=>%c%c], TAB#B[%c%c=>%c%c], SCM(M)=%s)\n"
				,(PRE>>3 & 0x01)? 'X':'O' ,(PRE>>2 & 0x01)? 'X':'O' 
				,(CUR>>3 & 0x01)? 'X':'O' ,(CUR>>2 & 0x01)? 'X':'O' 
				,(PRE>>1 & 0x01)? 'X':'O' ,(PRE>>0 & 0x01)? 'X':'O'
				,(CUR>>1 & 0x01)? 'X':'O' ,(CUR>>0 & 0x01)? 'X':'O'
				,(pTAP_A>>1)? 'X':'O' ,(pTAP_A&0x01)? 'X':'O' 
				,(cTAP_A>>1)? 'X':'O' ,(cTAP_A&0x01)? 'X':'O' 
				,(pTAP_B>>1)? 'X':'O' ,(pTAP_B&0x01)? 'X':'O' 
				,(cTAP_B>>1)? 'X':'O' ,(cTAP_B&0x01)? 'X':'O'
				,(nSCM_A<=nSCM_B)? "SCM#A":"SCM#B");

 	LOG (LOGID, FL, "SCE#A[%c%c|%c%c], SCE#B[%c%c|%c%c] \n"
		,(PA(0)==SCE_UP)? 'O':'X'
		,(PA(1)==SCE_UP)? 'O':'X'
		,(PA(2)==SCE_UP)? 'O':'X'
		,(PA(3)==SCE_UP)? 'O':'X'
		,(PB(0)==SCE_UP)? 'O':'X'
		,(PB(1)==SCE_UP)? 'O':'X'
		,(PB(2)==SCE_UP)? 'O':'X'
		,(PB(3)==SCE_UP)? 'O':'X'
	);
}

int ChkEventHA ()
{
	if(abs((int)difftime(tEvent, time(0)))<EVENT_TIME_VALID) {
   		LOG (LOGID, FL, "[%s] Event Time Not Passed\n", __FUNCTION__);
		return NOP;
	}
	bGET (Bit);

	if(PRE==CUR) {	// EXCEPTION HANDLE
		int flag=0;
		//fprintf(stderr, "PRE: %d State:%d, OnFail[SCE_A]: %d, S_SCE_B_DOWN: %d \n", PRE, State, OnFail[SCE_A], IS_SCE_B_DOWN);
		// 고려사항: 실제 TAP의 모니터링 링크는 정상으로 돌아와도, 이후에 서비스링크가 살기때문에
		// ON-FAILURE 시의 동작에 문제가 발생할수 있다. 그러므로 ON-FAILURE 상태를 확인해야 한다.
		if(State == FWD_AB) {
			if(OnFail[SCE_A]==BYPASS && IS_SCE_B_UP) 	{ flag=1; SYSCMD (SCE_A_FAIL_CUTOFF); }
			if(OnFail[SCE_B]==BYPASS && IS_SCE_A_UP) 	{ flag=1; SYSCMD (SCE_B_FAIL_CUTOFF); }
			if(OnFail[SCE_A]==CUTOFF && IS_SCE_B_DOWN) 	{ flag=1; SYSCMD (SCE_A_FAIL_BYPASS); }
			if(OnFail[SCE_B]==CUTOFF && IS_SCE_A_DOWN) 	{ flag=1; SYSCMD (SCE_B_FAIL_BYPASS); }
		}
		if(State == ONLY_A) {
			if(OnFail[SCE_A]==CUTOFF) { flag=1; SYSCMD (SCE_A_FAIL_BYPASS); }
		}
		if(State == ONLY_B) {
			if(OnFail[SCE_B]==CUTOFF) { flag=1; SYSCMD (SCE_B_FAIL_BYPASS); }
		}
		if(State == NO_CTRL) {
			if(OnFail[SCE_A]==CUTOFF) { flag=1; SYSCMD (SCE_A_FAIL_BYPASS); }
			if(OnFail[SCE_B]==CUTOFF) { flag=1; SYSCMD (SCE_B_FAIL_BYPASS); }
		}
		if(flag) DoPrnStat();

		return NOP;	// NO CHANGE, NO OPERATION
	}
	DoPrnStat();

	if(PRE==0x00 && CUR==0x0F)	return TAP_D_DN;
	if(PRE==0x0F && CUR==0x00)	return TAP_D_UP;

	if(cTAP_A != pTAP_A) {
		if(cTAP_A == UP) return TAP_A_UP;
		if(cTAP_A == DN) return TAP_A_DN;
	}
	if(cTAP_B != pTAP_B) {
		if(cTAP_B == UP) return TAP_B_UP;
		if(cTAP_B == DN) return TAP_B_DN;
	}
	return VCS;
}

int DnTapA (int Next)
{
	SYSCMD (SCE_A_OPER_CUTOFF);
	SYSCMD (SCE_B_FAIL_BYPASS);

	return Next;
}

int DnTapB (int Next)
{
	SYSCMD (SCE_B_OPER_CUTOFF);
	SYSCMD (SCE_A_FAIL_BYPASS);

	return Next;
}

int UpTapA (int Next)
{
	SYSCMD (SCE_A_OPER_FORWARD);
	SYSCMD (SCE_A_FAIL_BYPASS);

	// 2010.08.24 jjinri : NO_CTRL => ONLY_A 경우 SCE B를 CUTOFF 시켜준다. 
   	LOG (LOGID, FL, "[Event Handler: %s] DUAL DOWN OTHER SIDE CUTOFF\n", __FUNCTION__);
	SYSCMD (SCE_B_OPER_CUTOFF);

	return Next;
}

int UpTapB (int Next)
{
	SYSCMD (SCE_B_OPER_FORWARD);
	SYSCMD (SCE_B_FAIL_BYPASS);

	// 2010.08.24 jjinri : NO_CTRL => ONLY_B 경우 SCE A를 CUTOFF 시켜준다. 
   	LOG (LOGID, FL, "[Event Handler: %s] DUAL DOWN OTHER SIDE CUTOFF\n", __FUNCTION__);
	SYSCMD (SCE_A_OPER_CUTOFF);

	return Next;
}

int UpTapAB (int Next)
{
	SYSCMD (SCE_A_OPER_FORWARD);
	SYSCMD (SCE_B_OPER_FORWARD);

	if(!IS_SCE_B_DOWN) 	{ SYSCMD (SCE_A_FAIL_CUTOFF); }
	else				{ SYSCMD (SCE_A_FAIL_BYPASS); }  

	if(!IS_SCE_A_DOWN) 	{ SYSCMD (SCE_B_FAIL_CUTOFF); }
	else				{ SYSCMD (SCE_B_FAIL_BYPASS); }

	return Next;
}

int DnTapAB (int Next)
{
	SYSCMD (SCE_A_OPER_BYPASS);
	SYSCMD (SCE_B_OPER_BYPASS);
	SYSCMD (SCE_A_FAIL_BYPASS);
	SYSCMD (SCE_B_FAIL_BYPASS);

	return Next;
}

void SetMaster(int whose)
{
	Master = whose;
	if(whose) 	
	{ 
		SYSCMD(VCS_B_MASTER); 
   		LOG (LOGID, FL, "[%s] VCS B MASTER\n", __FUNCTION__);
	}
	else		
	{ 
		SYSCMD(VCS_A_MASTER); 
   		LOG (LOGID, FL, "[%s] VCS A MASTER\n", __FUNCTION__);
	}
}

typedef struct _FSM_t {
	int 	Prev ;
	int 	Event;
	int  	(*Func)(int); 
	int 	Next;
	char	Desc[BUFSIZE];
} FSM_t;

FSM_t FSM[15] = {
	{FWD_AB, 	TAP_A_DN, DnTapA, 	ONLY_B,  "TAP[A] DN (FWD_AB=>ONLY_B)"},
	{FWD_AB, 	TAP_B_DN, DnTapB, 	ONLY_A,  "TAP[B] DN (FWD_AB=>ONLY_A)"},
	{FWD_AB, 	TAP_D_DN, DnTapAB, 	NO_CTRL, "TAP[*] DN (FWD_AB=>NOCTRL)"},

	{ONLY_A, 	TAP_B_UP, UpTapAB, 	FWD_AB,  "TAP[B] UP (ONLY_A=>FWD_AB)"},
	{ONLY_A, 	TAP_A_DN, DnTapAB, 	NO_CTRL, "TAP[A] DN (ONLY_A=>NOCTRL)"},

	{ONLY_B, 	TAP_A_UP, UpTapAB, 	FWD_AB,  "TAP[A] UP (ONLY_B=>FWD_AB)"},
	{ONLY_B, 	TAP_B_DN, DnTapAB, 	NO_CTRL, "TAP[B] DN (ONLY_B=>NOCTRL)"},

	{NO_CTRL, 	TAP_D_UP, UpTapAB, 	FWD_AB,  "TAP[*] UP (NOCTRL=>FWD_AB)"},
	{NO_CTRL, 	TAP_A_UP, UpTapA, 	ONLY_A,  "TAP[A] UP (NOCTRL=>ONLY_A)"},
	{NO_CTRL, 	TAP_B_UP, UpTapB, 	ONLY_B,  "TAP[B] UP (NOCTRL=>ONLY_B)"},
	{0, 0, NULL, 0, ""}
};

int DoFsmHA(int Event)
{
	int i;

	if(!Event) 
		return NOP;

	for(i=0; FSM[i].Event; i++) {
		if(FSM[i].Prev  != State) continue;
		if(FSM[i].Event != Event) continue;

    	LOG (LOGID, FL, "%s \n", FSM[i].Desc);
		State = FSM[i].Func(FSM[i].Next);
		tEvent = time(0);
		return Event;
	}
	return NOP;
}

///////////////////////////////////////////////////////////////////
//
// POSIX Thread. (HA CHECK)
//
#define TIME0	1
void *thCheckHA (void *args)
{
    LOG (LOGID, FL, "POSIX THREAD [SERVICE-READY] #####\n");
	tEvent = time(0);

	while(1) {
		// EVENT GENERATOR
		if(DoFsmHA(ChkEventHA())) {
			if((Master == SCM_A) && (nSCM_B < nSCM_A)) SetMaster(SCM_B);
			if((Master == SCM_B) && (nSCM_A < nSCM_B)) SetMaster(SCM_A);
		}
		sleep (TIME0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
// MAN-MACHINE-COMMAND
// 	-- DIS-SCE-MODE
//
// OUTPUT: operational forwarding cutoff forwarding cutoff
#define DIS_SCE_A	"/DSC/SCRIPT/dis-sce.snmp 0"
#define DIS_SCE_B	"/DSC/SCRIPT/dis-sce.snmp 1"
#define SET_SCE		"/DSC/SCRIPT/set-sce.ex"
#define F_DIS_SCE_A	"/tmp/sce0-mode"
#define F_DIS_SCE_B	"/tmp/sce1-mode"
#define space		"    "
#define line1		"----------------------------------------------------------------------------"
#define line2		"============================================================================"
#define SPACE		space
#define LINE1		sprintf(buf, "%s%s\n",SPACE ,line1)
#define LINE2		sprintf(buf, "%s%s\n",SPACE ,line2)
//#define REMOVE(f)	do { sprintf(buf, "/usr/bin/rm %s 2>&1 > /dev/null",f); system(buf); } while(0);
#define REMOVE(f)	do { sprintf(buf, "/usr/bin/rm %s 2>&1 ",f); my_system(buf); } while(0);

void
MMCResSnd (IxpcQMsgType *rxIxpcMsg, char *Result, char resCode, char contFlag)
{
    GeneralQMsgType txGenQMsg;
    IxpcQMsgType    *txIxpcMsg 	= (IxpcQMsgType*)txGenQMsg.body;
    MMLResMsgType   *txResMsg 	= (MMLResMsgType*)txIxpcMsg->body;
    MMLReqMsgType   *rxReqMsg 	= (MMLReqMsgType*)rxIxpcMsg->body;

    bzero ((void*)&txIxpcMsg->head, sizeof(txIxpcMsg->head));

	// TX MESSAGE Fill-Out
    txGenQMsg.mtype = MTYPE_MMC_RESPONSE;
    strcpy (txIxpcMsg->head.srcSysName, rxIxpcMsg->head.dstSysName);
    strcpy (txIxpcMsg->head.srcAppName, rxIxpcMsg->head.dstAppName);
    strcpy (txIxpcMsg->head.dstSysName, rxIxpcMsg->head.srcSysName);
    strcpy (txIxpcMsg->head.dstAppName, rxIxpcMsg->head.srcAppName);
    txIxpcMsg->head.segFlag = 0;                                                                                  
    txIxpcMsg->head.seqNo 	= 1;                                                                                    
                                                                                                                  
    txResMsg->head.mmcdJobNo 	= rxReqMsg->head.mmcdJobNo;                                                          
    txResMsg->head.resCode 		= resCode;                                                                             
    txResMsg->head.contFlag 	= contFlag;                                                                           
    strcpy(txResMsg->head.cmdName, rxReqMsg->head.cmdName);                                                       
                                                                                                                  
	strcpy(txResMsg->body, Result);
    txIxpcMsg->head.bodyLen = sizeof(txResMsg->head) + strlen(txResMsg->body);                                    

    if (msgsnd(IXQID, (void*)&txGenQMsg, sizeof(txIxpcMsg->head) + txIxpcMsg->head.bodyLen, IPC_NOWAIT) < 0) {                                              
		LOG (LOGID, FL, "ERROR:%S, CMD=%s\n",strerror(errno), txResMsg->head.cmdName);
    }                                                                                                             

	return;
}                                                                                                                 

void _doDisSceMode (int flag, char *Result)
{
	char buf[BUFSIZE];
	char v1[BUFSIZE], v2[BUFSIZE], v3[BUFSIZE];

    sprintf(buf, "%s%-4s %-16s %-16s %-16s %-10s\n"
		,SPACE, "SCE", "ACTIVE", "FAILURE", "FAILURE-RECOVERY", "AUTO"); strcat(Result, buf); 
    LINE2;			strcat(Result, buf);

	REMOVE(F_DIS_SCE_A);
	REMOVE(F_DIS_SCE_B);

	// SCE#1
	if(flag==0 || flag==2) {
		FILE *fp;
		sprintf(buf, "%s > %s", DIS_SCE_A, F_DIS_SCE_A); my_system(buf);
		*v1 = *v2 = *v3 = 0;
		if((fp = fopen(F_DIS_SCE_A, "r")) != NULL) { fscanf(fp, "%s %s %s", v1, v2, v3); fclose(fp); }
    	sprintf(buf, "%s%-4s %-16s %-16s %-16s %-10s\n"
			,SPACE, "SCEA", v2, v3, v1, (pSFM->auto_sce_mode)? "MANUAL":"AUTO"); strcat(Result, buf); 
		if(flag == 2) { LINE1; strcat(Result, buf); }
	}
	// SCE#2
	if(flag==1 || flag==2) {
		FILE *fp;
		sprintf(buf, "%s > %s", DIS_SCE_B, F_DIS_SCE_B); my_system(buf);
		*v1 = *v2 = *v3 = 0;
		if((fp = fopen(F_DIS_SCE_B, "r")) != NULL) { fscanf(fp, "%s %s %s", v1, v2, v3); fclose(fp); }
    	sprintf(buf, "%s%-4s %-16s %-16s %-16s %-10s\n"
			,SPACE, "SCEB", v2, v3, v1, (pSFM->auto_sce_mode)? "MANUAL":"AUTO"); strcat(Result, buf); 
	}
    LINE2;			strcat(Result, buf);
}

///////////////////////////////////////////////////////////////////
// DIS-SCE-MODE : Run Sciprt, Parsing
void doDisSceMode (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	char Result[BUFSIZE];
	char buf[BUFSIZE];
	char v1[BUFSIZE];
	int flag;

    strcpy(v1, rxReqMsg->head.para[0].paraVal);

    if (!strcasecmp(v1, "SCEA")) 							flag = 0; 
    else if (!strcasecmp(v1, "SCEB"))						flag = 1;
    else 													flag = 2;

	bzero(Result, BUFSIZE);
    sprintf(buf, "%sSYSTEM = %s\n",SPACE, MYSYS);	strcat(Result, buf);
    sprintf(buf, "%sRESULT = SUCCESS\n",SPACE);		strcat(Result, buf);
    LINE2;			strcat(Result, buf);

	_doDisSceMode (flag, Result);
	MMCResSnd(rxIxpcMsg, Result, 0, 0);
}

///////////////////////////////////////////////////////////////////
// MMC : RELOAD-SCE
void doReloadSce  (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	char Result[BUFSIZE];
	char buf[BUFSIZE];
	char v1[BUFSIZE];
	int flag;

    strcpy(v1, rxReqMsg->head.para[0].paraVal);

    if (!strcasecmp(v1, "SCEA")) 							flag = 0; 
    else if (!strcasecmp(v1, "SCEB"))						flag = 1;
    else 													flag = 2;

	bzero(Result, BUFSIZE);
    sprintf(buf, "%sSYSTEM = %s\n",SPACE, MYSYS);	strcat(Result, buf);
    sprintf(buf, "%sRESULT = SUCCESS\n",SPACE);		strcat(Result, buf);
//    LINE2;			strcat(Result, buf);

	if(flag == 0 || flag == 2) { sprintf (buf, "%s %d %s", SET_SCE, 0, "reload"); my_system(buf); }
	if(flag == 1 || flag == 2) { sprintf (buf, "%s %d %s", SET_SCE, 1, "reload"); my_system(buf); }
	LOG (LOGID, FL, "%s\n", buf);

	MMCResSnd(rxIxpcMsg, Result, 0, 0);
}

///////////////////////////////////////////////////////////////////
// MMC : SET-SCE-MODE
void doSetSceMode (IxpcQMsgType *rxIxpcMsg)
{
	MMLReqMsgType   *rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;
	char Result[BUFSIZE];
	char buf[BUFSIZE];
	char v1[BUFSIZE], v2[BUFSIZE], v3[BUFSIZE];
	int flag;

	bzero(Result, BUFSIZE);

	// AUTO MODE CHECK 
    if (!pSFM->auto_sce_mode) {
    	sprintf (buf, "%sSYSTEM = %s\n",SPACE, MYSYS);	strcat(Result, buf);
        sprintf (buf, "%sRESULT = FAILURE\n",SPACE); strcat(Result, buf);
        sprintf (buf, "%s%s\n",SPACE, "현재설정이 자동모드로 설정되어 있습니다."); strcat(Result, buf);
        sprintf (buf, "%s%s\n",SPACE, "AUTO-SCE-MODE 명령어를 이용하여, 수동설정 후 사용하시기 바랍니다."); strcat(Result, buf);
        MMCResSnd(rxIxpcMsg, Result, -1, 0);
		printf(Result);
        return;
    }

    strcpy(v1, rxReqMsg->head.para[0].paraVal);
    strcpy(v2, rxReqMsg->head.para[1].paraVal);
    strcpy(v3, rxReqMsg->head.para[2].paraVal);

    if (!strcasecmp(v1, "SCEA")) 							flag = 0; 
    else if (!strcasecmp(v1, "SCEB"))						flag = 1;
    else 													flag = 2;

    sprintf(buf, "%sSYSTEM = %s\n",SPACE, MYSYS);	strcat(Result, buf);
    sprintf(buf, "%sRESULT = SUCCESS\n",SPACE);		strcat(Result, buf);
    LINE2;			strcat(Result, buf);

	if(flag == 0 || flag == 2) {
		 if (!strcasecmp(v2, "ACTIVE")) {
			sprintf (buf, "%s %d %s %s", SET_SCE, 0, "link"
				,(!strcasecmp(v3, "FORWARDING"))? "forwarding":(!strcasecmp(v3, "BYPASS"))? "bypass":"cutoff"); my_system(buf);
		 }
		 if (!strcasecmp(v2, "FAILURE")) {
			sprintf (buf, "%s %d %s %s", SET_SCE, 0, "onfail"
				,(!strcasecmp(v3, "BYPASS"))? "bypass":"cutoff"); my_system(buf);

			OnFail[SCE_A] = (!strcasecmp(v3, "BYPASS"))? BYPASS:CUTOFF;
		 }
		 if (!strcasecmp(v2, "RECOVERY")) {
			sprintf (buf, "%s %d %s %s", SET_SCE, 0, "recovery"
				,(!strcasecmp(v3, "OPERATIONAL"))? "operational":"non-operational"); my_system(buf);
		 }
	}
	if(flag == 1 || flag == 2) {
		 if (!strcasecmp(v2, "ACTIVE")) {
			sprintf (buf, "%s %d %s %s", SET_SCE, 1, "link"
				,(!strcasecmp(v3, "FORWARDING"))? "forwarding":(!strcasecmp(v3, "BYPASS"))? "bypass":"cutoff"); my_system(buf);
		 }
		 if (!strcasecmp(v2, "FAILURE")) {
			sprintf (buf, "%s %d %s %s", SET_SCE, 1, "onfail"
					,(!strcasecmp(v3, "BYPASS"))? "bypass":"cutoff"); my_system(buf);

			OnFail[SCE_B] = (!strcasecmp(v3, "BYPASS"))? BYPASS:CUTOFF;
		 }
		 if (!strcasecmp(v2, "RECOVERY")) {
			sprintf (buf, "%s %d %s %s", SET_SCE, 1, "recovery"
				,(!strcasecmp(v3, "OPERATIONAL"))? "operational":"non-operational"); my_system(buf);
		 }
	}
	_doDisSceMode (flag, Result);
	MMCResSnd(rxIxpcMsg, Result, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//	MAIN FUNCTION
//
int main()
{
	GeneralQMsgType rxGenQMsg;
	IxpcQMsgType *rxIxpcMsg;
	MMLReqMsgType *rxReqMsg;
	int n;

	INIT;

#ifdef TEST	// LOCAL TEST
	srand(getpid()|time(0));
#endif
	cbSetTimer(cbTaskHA, 1, NULL, TIMER);
	pthread_create(&t_thCheckHA, NULL, thCheckHA, NULL);

	// DO MAN-MACHINE-COMMAND
	while ((n = msgrcv(MYQID, &rxGenQMsg, sizeof(rxGenQMsg), 0, MSG_NOERROR))) {
		switch (rxGenQMsg.mtype) {
		case MTYPE_MMC_REQUEST :
            rxIxpcMsg = (IxpcQMsgType*)rxGenQMsg.body;
            rxReqMsg = (MMLReqMsgType*)rxIxpcMsg->body;

			// DIS-SCE-MODE
			if (!strcasecmp(rxReqMsg->head.cmdName, "dis-sce-mode"))
				doDisSceMode(rxIxpcMsg);
			// RELOAD-SCE
			else if (!strcasecmp(rxReqMsg->head.cmdName, "reload-sce"))
				doReloadSce(rxIxpcMsg);
			// SET-SCE-MODE
			else if (!strcasecmp(rxReqMsg->head.cmdName, "set-sce-mode"))
				doSetSceMode(rxIxpcMsg);
			break;
		}
	}
	return 1;
}

/*****************************************************************
-- 	Bit의 값은 SCM의 포트 A_1, A_2, B_1, B_2 값으로 셋팅된다. 
--	0 : Up
--	1 : Dn
-- SCE 제어는 DIRECTOR 포트를 기준으로 한다. 
-- DIRECTOR A 는 SCM의 A_1과 B_1 bit 값으로 계산되고
-- DIRECTOR B 는 SCM의 A_2과 B_2 bit 값으로 계산된다. 
-- 초기화 시에 Bit는 0xFF 로 셋팅되고 bGET 연산에 의해
-- PRE BIT 는 (1,1,1,1)로 셋팅되고,
-- CUR BIT 는 sfdb의 SCM 포트 상태값으로 셋팅된다.
-- FSM 의 초기 State 값은 NO_CTRL 상태로 초기화된다. 

-- 절체 판단은 SCM 포트를 기준으로 한다.
-- SCMA A : A_1, A_2 의 bit 값으로 계산한다. 
-- SCMA B : B_1, B_2 의 bit 값으로 계산한다. 
-- nSCMA 와 nSCMB 의 값이 높은 쪽이 문제가 많으므로 값이 작은쪽으로 
-- 절체를 한다. 

 PRE BIT		|	CUR BIT
 ---------------------------------
 A1	A2	B1	B2	|	A1	A2	B1	B2 	<--------	SCM 포트 상태 값
 8	7	6	5	|	4	3	2	1
 ---------------------------------
 1	1	1	1	|	1	0	1	0	: 	PRE STATE : NO_CTRL, Event : TAP_B_UP, NEXT STATE : ONLY B
 						-		-		

 					
 PRE BIT는 1111로 초기화 되고,
 CUR BIT는 sfdb 상태값으로 셋팅된다.

 위 경우는 SCMA 의 A1 포트가 DN, SCMB 의 B1 포트가 DN이 된 상태이며,
 SCMA A2 포트가 UP, SCMB B2 포트가 UP 된 상태이다. 

 DIRECTOR A는 A1, B1  두 포트와 맵핑 되므로 현재 죽어있는 상태이고,
 DIRECTOR B는 A2, B2 포트가 PRE[11]에서 CUR[00]으로 Up 되었으므로 DIRECTOR B는 살아있는 상태이다.

 PRE BIT와 CUR BIT를 비교했을 때 , A2, B2에 변화가 일어난 상황이므로
 이벤트 상으로 DIRECTOR B가 살아난 경우를 뜻한다. 

 FSM 구조체 정의에 의해 
 PRE STATE는 NO_CTRL 상태에서 TAP_B_UP 이벤트가 발생되어 다음 ONLY B 상태로 전이되는
 과정이고, 이때 UpTapB 이벤트 핸들러 함수가 호출된다. 

 UpTapB 함수 또는 UpTapA 함수는 오직 이전 상태가 NO_CTRL(Dual Down)시에만 호출된다. 
 이 이벤트 함수 호출시에 반대편 side를 CUTOFF 시켜주는 CMD 가 추가 되었다. 

**************************************************************************************************/




