#include "mcdm_proto.h"

extern int		ixpcQid, lastAllocJobNo;
extern char		mySysName[COMM_MAX_NAME_LEN], myAppName[COMM_MAX_NAME_LEN];
extern char		trcBuf[4096], trcTmp[1024];
extern time_t	currentTime;
extern McdmJobTblContext		mcdmJobTbl[MCDM_NUM_TP_JOB_TBL];
extern int						lastAllocJobNo;
extern McdmDistribMmcTblContext	mcdmDistrMmcTbl[MCDM_MAX_DISTRIB_MMC];
extern int		numDistrMmc;
extern int		trcFlag, trcLogFlag;



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mcdm_allocJobTbl (void)
{
	int		i;

	for (i=lastAllocJobNo; i < MCDM_NUM_TP_JOB_TBL; i++) {
		if (!mcdmJobTbl[i].tpInd) {
			lastAllocJobNo = i+1;
			memset ((void*)&mcdmJobTbl[i], 0, sizeof(McdmJobTblContext));
			return i;
		}
	}
	for (i=0; i < lastAllocJobNo; i++) {
		if (!mcdmJobTbl[i].tpInd) {
			lastAllocJobNo = i+1;
			memset ((void*)&mcdmJobTbl[i], 0, sizeof(McdmJobTblContext));
			return i;
		}
	}
	return -1;

} //----- End of mcdm_allocJobTbl -----//



//------------------------------------------------------------------------------
// resBuf에 붙어있는 메모리 들을 free하고 mcdmJobTbl의 clear한다.
//------------------------------------------------------------------------------
void mcdm_deallocJobTbl (int jobNo)
{
	int		i;
	McdmResBufContext	*ptr, *next;

	for (i=0; i < SYSCONF_MAX_ASSO_SYS_NUM; i++)
	{
		ptr = mcdmJobTbl[jobNo].resBuf[i];
		while (ptr != NULL) {
			next = ptr->next;
			free(ptr);
			ptr = next;
		}
	}

	memset ((void*)&mcdmJobTbl[jobNo], 0, sizeof(McdmJobTblContext));

	return;

} //----- End of mcdm_deallocJobTbl -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void mcdm_checkJobTbl (void)
{
	int		i;

	for (i=0; i < MCDM_NUM_TP_JOB_TBL; i++) {
		if (mcdmJobTbl[i].tpInd && mcdmJobTbl[i].deadlineTime < currentTime) {
			mcdm_sendDistribMmcRes2MMCD (i);
			mcdm_deallocJobTbl (i);
		}
	}

	return;

} //----- End of mcdm_checkJobTbl -----//

