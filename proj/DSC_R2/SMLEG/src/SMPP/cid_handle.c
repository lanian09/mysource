/**********************************************************
   Author   : Yuran Park
   SCCS ID  : %W%
   Date     : %G%
   Revision History : 

   Description:
		Cid������ƾ

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

/* Free List ������ */
static int 	freecidnext[MAXCIDFORAP + 4];
static char freecidstatus[MAXCIDFORAP + 4];
static int 	freecid;
static int 	freecidlast;

/**D.1*  Definition of Functions  *************************/

void InitCid(int start, int end)
{
	int i, le;

	/* �ʿ��� cid table�� ũ�⸦ ����Ѵ� */ 
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
		/* �ű� cid�� Ȱ�� �޴´� */
		cid = freecid;
		freecid = freecidnext[freecid];

		if (freecid < 1 || freecid >= MAXCIDFORAP) {
			Rebuild_Cid_Table();
			cid = freecid;
		}

		/* freecidstatus[cid]�� 1�̸� Node�� ��������� ��Ÿ�� */
		freecidstatus[cid] = 1;

		/* freecidnext[cid]�� ����߿��� alloc�� �ð��� ǥ���ϰ� */
		/* Free�ÿ��� ���� free node�� ����Ų��					*/
		freecidnext[cid] = now;
		return cid;
	} 

	/* ���� ����Ҽ� �ִ� Node�� ���� ��� ���������� �ѹ� ������ */

	/* Garbage Collection�� �����Ѵ� */
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

	/* �Ѱ� �̻��� cid�� ���밡���ϰ� �� ��� */
	if (n1) {
		cid_collect_flag = 1;			
		return NewCid();
	} 
	return 0;

#ifdef FOR_BILLID_TBL_MNG
�� ��ƾ�� �����ϸ� ȣ���� ��Ȳ���� �ɰ��� ��ָ� �߻���ų �� ����.
-> cid ���� �뷮 �̻����� ȣ ���Խ� ȣ�� reject ó���ϱ� ����.
	
	/* 10���̻� ���� ��� node�� ���� */
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

	/* ������ ������ ���ϴ� cid���� free��Ŵ */
	if (cid < 10 || cid > Cid_Limit)
		return;

	/* �ݵ�� freecidstatus�� 0�� �ƴϾ�� �ϸ� freecidnext�� TIME_CODE���� Ŀ�� �Ѵ� */
	if (freecidstatus[cid] == 0 || freecidnext[cid] < TIME_CODE) {
		/* �� ��� ��ȯ���� �Ǽ��ϴ� ������ ���� Free�� �Ǵ� ��� �̴� */
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
	/* ������ ������ ���ϴ� cid���� active��Ŵ */
	if (cid <= 0 || cid > Cid_Limit)
		return;

	freecidstatus[cid] = 1;
}

void InactiveCid(int cid)
{
	/* ������ ������ ���ϴ� cid���� active��Ŵ */
	if (cid <= 0 || cid > Cid_Limit)
		return;

	freecidstatus[cid] = 0;
}


int IsValidCid(int cid)
{
	/* ������ ������ ���ϴ� cid���� �˻��� */
	if (cid <= 0 || cid > Cid_Limit)
		return 0;

	return freecidstatus[cid];
}
