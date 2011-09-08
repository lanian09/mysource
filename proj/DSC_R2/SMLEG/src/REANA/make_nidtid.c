/**********************************************************
                 ABLEX Main-Memory DBMS

   Author   : LEE SANG HO
   Section  : IPAS Project
   SCCS ID  : @(#)make_nidtid.c	1.1
   Date     : 1/22/03
   Revision History :
        '03.  1. 15     Initial

   Description:
        Make NID & TID

   Copyright (c) ABLEX 2001
***********************************************************/

/**A.1*  File Inclusion ***********************************/

#include <stdio.h>
#include <time.h>
#include <ipaf_svc.h>

/**B.1*  Definition of New Constants *********************/

/**B.2*  Definition of New Type  **************************/

/**C.1*  Declaration of Variables  ************************/

time_t	dOldTime = 0;
UINT	uiSerial = 1;

/**D.1*  Definition of Functions  *************************/

/**D.2*  Definition of Functions  *************************/

int	dMakeNIDTID(UCHAR ucSysType, UCHAR ucSvcID, UCHAR ucMsgID, INT64 *pllNID, INT64 *pllTID)
{
	time_t		dNow;
	un_NID		unNID;
	un_TID		unTID;

	dNow = time(0);
	if(dOldTime != dNow)
	{
		uiSerial = 1;
		dOldTime = dNow;
	}
	else if(uiSerial > 50000)
		return -1;
		
	unNID.llNID = 0;
	unTID.llTID = 0;

	unNID.stNID.ucSysType = ucSysType;
	unNID.stNID.uiSerial = uiSerial;
//	unNID.stNID.stBuild = dNow;

	unTID.stTID.ucMsgID = ucMsgID;
	unTID.stTID.ucSvcID = ucSvcID;
	unTID.stTID.uiSerial = uiSerial++;
	unTID.stTID.stBuild = dNow;

	*pllNID = unNID.llNID;
	*pllTID = unTID.llTID;
	
	return 0;
} 

int	dMakeNID(UCHAR ucSysType, INT64 *pllNID)
{
	time_t		dNow;
	un_NID		unNID;

	dNow = time(0);
	if(dOldTime != dNow)
	{
		uiSerial = 1;
		dOldTime = dNow;
	}
	else if(uiSerial > 50000)
		return -1;
		
	unNID.llNID = 0;
	unNID.stNID.ucSysType = ucSysType;
	unNID.stNID.uiSerial = uiSerial++;
//	unNID.stNID.stBuild = dNow;
	memcpy( unNID.stNID.szReserved, &dNow, 2 );

	*pllNID = unNID.llNID;

	return 0;
} 

int	dMakeTID(UCHAR ucSvcID, UCHAR ucMsgID, INT64 *pllTID)
{
	time_t		dNow;
	un_TID		unTID;

	dNow = time(&dNow);
	if(dOldTime != dNow)
	{
		uiSerial = 1;
		dOldTime = dNow;
	}
	else if(uiSerial > 50000)
		return -1;
		
	unTID.llTID = 0;
	unTID.stTID.ucMsgID = ucMsgID;
	unTID.stTID.ucSvcID = ucSvcID;
	unTID.stTID.uiSerial = uiSerial++;
	unTID.stTID.stBuild = dNow;

	*pllTID = unTID.llTID;
	
	return 0;
}
