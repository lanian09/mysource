/** A.1 * File Include ********************************************************/

/* SYS HEADER */
#include <stdio.h>
#include <errno.h>
#include <string.h>	/* STRLEN(3), MEMSET(3), MEMCPY(3), STRSTR(3) */
/* LIB HEADER */
#include "filedb.h"
#include "loglib.h"
/* PRO HEADER */
#include "path.h"
/* TAM HEADER */
/* TAF HEADER */
/* OAM HEADER */
#include "almstat.h"
/* LOC HEADER */
#include "chsmd_mask.h"	/* dWriteMaskInfo() */
#include "chsmd_link.h"

/**B.1*  Definition of New Constants ******************************************/
char szLink[] = "Link detected: yes";
char szSpeed[] = "Speed: 1000Mb/s";
char szDuplex[] = "Duplex: Full";

st_linkdev stLinkDev[MAX_LINK_COUNT];

unsigned char ucOldChnl[MAX_CH_COUNT]; /* Init-value : -1 */

/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
extern pst_NTAM fidb;
extern char *SYS_CONFIG_FILE;

void Init_Chnl()
{
	int i;
	for( i = 0; i< MAX_CH_COUNT; i++ ){
		ucOldChnl[i] = fidb->NTAFChnl[i];
	}
}

int dChnl_Chk(void)
{
	int				i, dRet, maskflag;
	unsigned char	ucCurr;

	dRet	= 0;
	log_print(LOGN_INFO, " # NTAF CHNNEL ###############################");

	maskflag = 0;
	for(i = 0; i < MAX_CH_COUNT; i++)
	{
		ucCurr = fidb->NTAFChnl[i];

		if( (ucCurr != MASK) && (ucCurr != ucOldChnl[i])){
			Send_CondMess(1, LOCTYPE_CHNL, INVTYPE_CLIENT, i, ucCurr, ucOldChnl[i]);
		}

		if( (ucCurr & MASK) != ( ucOldChnl[i] & MASK ) ){
			maskflag++;
		}

		ucOldChnl[i] = ucCurr;
	}
	
	if( maskflag ){
		dWriteMaskInfo(0);
	}

	return dRet;
}

int Link_Check()
{
    int 			dRet;
    int 			i;
    int             dDevIdx;
    char            cResult;

    char     dCur_Status[MAX_LINK_COUNT];
    char     dOld_Status[MAX_LINK_COUNT];


	log_print( LOGN_INFO, " # LINK INFO ###########################");

	for(i = 0; i < MAX_LINK_COUNT; i++)
	{
		if( strlen(stLinkDev[i].szDevName) == 0 )
			continue;

		cResult = 0;
		dDevIdx = i;

		dRet = dEthCheck(stLinkDev[i].szDevName);
		if(dRet == 111)
			cResult = 1;

    	if(cResult == 1)
    	{
			log_print(LOGN_DEBUG, "LINK : NET NAME[%s] STS[UP  ]", stLinkDev[i].szDevName);
			dCur_Status[dDevIdx] = NORMAL;
		}
		else
		{
			log_print(LOGN_DEBUG, "LINK : NET NAME[%s] STS[DOWN]", stLinkDev[i].szDevName);
			dCur_Status[dDevIdx] = CRITICAL;
		}

		dOld_Status[dDevIdx] = fidb->link[dDevIdx];

		if( dOld_Status[dDevIdx] != dCur_Status[dDevIdx] )
		{
			if( (dCur_Status[dDevIdx] == NORMAL) || (dCur_Status[dDevIdx] == CRITICAL) || (dOld_Status[dDevIdx] != NOT_EQUIP))
			{
				if( (dOld_Status[dDevIdx] & MASK) != MASK )
				{
					Send_CondMess( SYSTYPE_TAM, LOCTYPE_PHSC, INVTYPE_CLIENT, dDevIdx, dCur_Status[dDevIdx], dOld_Status[dDevIdx] );
				}
			}
		}

		SetFIDBValue( &fidb->link[dDevIdx], dCur_Status[dDevIdx] );

		if( fidb->link[dDevIdx] == CRITICAL )
			log_print( LOGN_CRI, "[LINK] IDX[%d] : STATUS[0x%02x:CRITICAL]", dDevIdx, fidb->link[dDevIdx] );

	}
    return 0;
}




int dEthCheck(char *szNetStr)
{
	FILE		*fp = NULL;
    char        szBuf[256];
    char        szCommand[1024];
	char		*szPnt;
    int         dRet;
	int		    dLen;

	dRet = 0;

	memset(szCommand, 0, 1024);
	sprintf(szCommand, "/sbin/ethtool %s", szNetStr);

	fp = popen((const char *)(szCommand), "r");

	if(fp == NULL)
	{
		log_print(LOGN_CRI, "dEthCheck : %s failed in popen [%s]",
		szCommand, strerror(errno));
		return -1;
	}

	while(fgets(szBuf, 256, fp) != NULL)
	{
		szPnt = strstr(szBuf, "Speed");
		if(szPnt != NULL)
		{
			dLen = strlen(szSpeed);
            if(strncmp(szPnt, szSpeed, dLen) == 0)
                dRet += 1;
			else
				log_print(LOGN_INFO, "Speed[%s] [%s]", szPnt, szNetStr);
		}

        szPnt = strstr(szBuf, "Duplex");
        if(szPnt != NULL)
			dRet += 10;
#if 0
			dLen = strlen(szDuplex);
            if(strncmp(szPnt, szDuplex, dLen) == 0)
                dRet += 10;
			else
				log_print(LOGN_INFO, "Duplex[%s] [%s]", szPnt, szNetStr);
#endif

        szPnt = strstr(szBuf, "Link");
        if(szPnt != NULL)
        {
			dLen = strlen(szLink);
            if(strncmp(szPnt, szLink, dLen) == 0)
                dRet += 100;
			else
				log_print(LOGN_INFO, "Link[%s] [%s]", szPnt, szNetStr);
        }
	}

	log_print(LOGN_INFO, "LINK RESULT [%d]", dRet);
	pclose(fp);

	return dRet;
}

int dReadLinkDevInfo(void)
{
	FILE        *fp;
	int         dFlag, dLinkDevCnt;
	char        sType[8], sDevName[8], sBuf[256];
	st_linkdev  stTmpLinkDev[MAX_LINK_COUNT];

	dLinkDevCnt	= 0;
    memcpy(&stTmpLinkDev[0], &stLinkDev[0], sizeof(st_linkdev)*MAX_LINK_COUNT);
    memset(&stLinkDev[0], 0x00, sizeof(st_linkdev)*MAX_LINK_COUNT);

    if( (fp = fopen(FILE_SYS_CONFIG, "r")) == NULL)
    {
        log_print(LOGN_CRI, LH"[ERROR] FILE_SYS_CONFIG OPEN = %s", LT, FILE_SYS_CONFIG);
        return -1;
    }

    while(fgets(sBuf, 256, fp) != NULL)
    {
        if(sBuf[0] != '#')
		{
			log_print(LOGN_CRI,LH"SYNTAX ERROR FILE_SYS_CONFIG = %s", LT, FILE_SYS_CONFIG);
            break;
		}

        if(sBuf[1] == '#')
            continue;
        else if(sBuf[1] == 'E')
            break;
        else if(sBuf[1] == '@')
        {
            if(dLinkDevCnt > MAX_LINK_COUNT)
            {
                log_print(LOGN_CRI, LH"INVALID LINK DEVICE NAME NUMBER",LT);
                break;
            }

            if(sscanf(&sBuf[2], "%s %s %d", sType, sDevName, &dFlag) == 3)
			{
				if(strcmp(sType, "NETTYPE") == 0)
				{
					strcpy(stLinkDev[dLinkDevCnt].szDevName, sDevName);
					if( (dFlag!=0) && (dFlag!=1))
						dFlag = 0;

					stLinkDev[dLinkDevCnt].ucFlag = dFlag;
					log_print(LOGN_DEBUG, LH"LINK DEVICE IDX=%02d, NAME=%s", LT, dLinkDevCnt, stLinkDev[dLinkDevCnt].szDevName);
		            dLinkDevCnt++;
				}
			}
        }
    }
    fclose(fp);

    return 0;
}
