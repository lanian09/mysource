/**********************************************************
   Author   : Yuran Park
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 

   Description:
		Cid생성루틴

   Copyright (c) INFRA VALLEY, Inc.
***********************************************************/
/**A.1*  File Inclusion ***********************************/
/**B.1*  Definition of New Constants *********************/

#include "smpp.h"

#define DISCARD_SINCE 10
#define MAXCIDFORAP 20000

/**B.2*  Definition of New Type  **************************/

#define TIME_CODE	904633596

/**C.1*  Declaration of Variables  ************************/

int Cid_Base, Cid_Limit, cid_collect_flag;

/* Free List 관리용 */
static int 	freecidnext[MAXCIDFORAP + 4];
static char freecidstatus[MAXCIDFORAP + 4];
static int 	freecid;
static int 	freecidlast;

/**D.1*  Definition of Functions  *************************/

void InitCid(int start, int end)
{
	int i, le;

	/* 필요한 cid table의 크기를 계산한다 */ 
	le = end - start - 2;

	if (le < 1 || le >= MAXCIDFORAP) {
		le = MAXCIDFORAP - 2;
	}

	for (i = 0; i < 10;i++)
		freecidstatus[i] = 2;

	for (i = 10; i < le;i++) {
		freecidnext[i] = i + 1;
		freecidstatus[i] = 0;
	}

	freecidlast = i;
	Cid_Limit = i;

	freecid = 10;
	freecidnext[freecidlast] = 0;

	Cid_Base = start;

}

void Rebuild_Cid_Table()
{
	int	i;

	if (Cid_Limit <= 100 || Cid_Limit >= MAXCIDFORAP) {
		Cid_Limit = 1000;
	}

	for (i = 10; i < Cid_Limit;i++) {
		freecidnext[i] = i + 1;
		freecidstatus[i] = 0;
	}

	freecidlast = Cid_Limit;
	freecid = 10;
    freecidnext[freecidlast] = 0;
}

int NewCid()
{
	int i, n1, n2, cid;
	time_t now;

	now = time(&now);
	cid_collect_flag = 0;

	if (freecid < 1 || freecid >= MAXCIDFORAP) {
		Rebuild_Cid_Table();
	}

	if (freecidnext[freecid]) {
		/* 신규 cid를 활당 받는다 */
		cid = freecid;
		freecid = freecidnext[freecid];

		if (freecid < 1 || freecid >= MAXCIDFORAP) {
			Rebuild_Cid_Table();
			cid = freecid;
		}

		/* freecidstatus[cid]가 1이면 Node가 사용중임을 나타냄 */
		freecidstatus[cid] = 1;

		/* freecidnext[cid]는 사용중에는 alloc된 시간을 표시하고 */
		/* Free시에는 다음 free node를 가리킨다					*/
		freecidnext[cid] = now;
		return cid;
	} 

	/* 지금 사용할수 있는 Node가 없는 경우 쓰레기통을 한번 뒤진다 */

	/* Garbage Collection을 시작한다 */
	n1 = n2 = 0;
	for (i = 10; i < Cid_Limit;i++) {
		/* Lost Node */
   		if (freecidstatus[i] == 0 || freecidnext[i] > now) {
			freecid = i;
			freecidlast = i;
			freecidnext[i] = 0;
			freecidstatus[i] = 0;
			break;
		}
	}

	for (i++; i < Cid_Limit;i++) {
		/* Lost Node */
		if (freecidstatus[i] == 0 || freecidnext[i] > now) {
			freecidnext[i] = 0;
			freecidstatus[i] = 0;
			freecidnext[freecidlast] = i;
			freecidlast = i;
			n1++;
		}
	}

	/* 한개 이상의 cid가 재사용가능하게 된 경우 */
	if (n1) {
		cid_collect_flag = 1;			
		return NewCid();
	} 
	return 0;

#ifdef FOR_BILLID_TBL_MNG
이 루틴을 수행하면 호폭주 상황에서 심각한 장애를 발생시킬 수 있음.
-> cid 제한 용량 이상으로 호 유입시 호를 reject 처리하기 위함.
	
	/* 10초이상 지난 모든 node를 제거 */
	now = now - DISCARD_SINCE;
	for (i = 10; i < Cid_Limit; i++) {
   		if (freecidnext[i] < now) {
			freecid = i;
			freecidnext[i] = 0;
			freecidstatus[i] = 0;
			freecidlast = i;
			break;
		}
	}

	for (i++; i < Cid_Limit; i++) {
		if (freecidnext[i] < now) {
			freecidnext[i] = 0;
			freecidstatus[i] = 0;
			freecidnext[freecidlast] = i;
			freecidlast = i;
			n2++;
		}
	}
		
	if (n2) {
		cid_collect_flag = 1;
		return NewCid();
	}
	return 0;
#endif

}

void FreeCid(int cid)
{

	/* 규정된 범위에 속하는 cid만을 free시킴 */
	if (cid < 10 || cid > Cid_Limit)
		return;

	/* 반드시 freecidstatus가 0이 아니어야 하며 freecidnext도 TIME_CODE보다 커야 한다 */
	if (freecidstatus[cid] == 0 || freecidnext[cid] < TIME_CODE) {
		/* 이 경우 교환기등에서 실수하는 것으로 이중 Free가 되는 경우 이다 */
		/* cid table bug 
		freecidnext[cid] = 0;
		*/
		return;
	}

	freecidnext[cid] = 0;
	freecidstatus[cid] = 0;
	freecidnext[freecidlast] = cid;
	freecidlast = cid;
}

void ActiveCid(int cid)
{
	/* 규정된 범위에 속하는 cid만을 active시킴 */
	if (cid <= 0 || cid > Cid_Limit)
		return;

	freecidstatus[cid] = 1;
}

void InactiveCid(int cid)
{
	/* 규정된 범위에 속하는 cid만을 active시킴 */
	if (cid <= 0 || cid > Cid_Limit)
		return;

	freecidstatus[cid] = 0;
}


int IsValidCid(int cid)
{
	/* 규정된 범위에 속하는 cid만을 검사함 */
	if (cid <= 0 || cid > Cid_Limit)
		return 0;

	return freecidstatus[cid];
}
