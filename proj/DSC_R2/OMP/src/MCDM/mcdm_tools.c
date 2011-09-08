#include "mcdm_proto.h"

extern char		trcBuf[4096], trcTmp[1024];
extern int		trcFlag, trcLogFlag;


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mcdm_ownMmcHdlrVector_qsortCmp (const void *a, const void *b)
{
	return (strcasecmp (((McdmOwnMmcHdlrVector*)a)->cmdName, ((McdmOwnMmcHdlrVector*)b)->cmdName));
} //----- End of mcdm_ownMmcHdlrVector_qsortCmp -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mcdm_ownMmcHdlrVector_bsrchCmp (const void *a, const void *b)
{
	return (strcasecmp ((char*)a, ((McdmOwnMmcHdlrVector*)b)->cmdName));
} //----- End of mcdm_ownMmcHdlrVector_bsrchCmp -----//



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mcdm_distrMmcTbl_qsortCmp (const void *a, const void *b)
{
	return (strcasecmp (((McdmDistribMmcTblContext*)a)->cmdName, ((McdmDistribMmcTblContext*)b)->cmdName));
} //----- End of mcdm_distrMmcTbl_qsortCmp -----//


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int mcdm_distrMmcTbl_bsrchCmp (const void *a, const void *b)
{
	return (strcasecmp ((char*)a, ((McdmDistribMmcTblContext*)b)->cmdName));
} //----- End of mcdm_distrMmcTbl_bsrchCmp -----//
