/**
 *	Include headers
 */
#include <errno.h>
#include <string.h>
#include <signal.h>

// TOP
#include "common_stg.h"
#include "commdef.h"
#include "path.h"
#include "sshmid.h"
#include "procid.h"

// LIB
#include "config.h"			/* MAX_SW_COUNT */
#include "mems.h"
#include "memg.h"
#include "cifo.h"
#include "nifo.h"
#include "gifo.h"
#include "loglib.h"
#include "ipclib.h"
#include "filelib.h"
#include "utillib.h"

// OAM
#include "almstat.h"		/* st_GEN_INFO */

// TAF
#include "debug.h"			/* DEBUG_INFO */
#include "ippool_bitarray.h"

// .
#include "prea_flt.h"
#include "prea_frag.h"
#include "prea_init.h"

/**
 *	Declare var.
 */
st_SVCONOFF_DATA    stSVCONOFFINFO[MAX_SERVICE_CNT];
int			gATCPCnt = 0;
int			gARPCnt = 0;
int			gAINETCnt = 0;

extern stCIFO	*gpCIFO;
extern S32	JiSTOPFlag;
extern S32	FinishFlag;

extern S32	dSysTypeInfo;

//extern T_GENINFO		*gen_info;
extern st_GEN_INFO		*gen_info;
extern pst_IPPOOLLIST	pstIPPOOLBIT;
extern DEBUG_INFO		*pDEBUGINFO;

/**
 *	Declare func.
 */
extern int dInit_IPPOOLBIT( int dKey );
extern int dSetIPPOOLList( UINT uiIPAddress, UINT uiNetMask, UCHAR ucStatus, pst_IPPOOLLIST pstIPPOOL );
extern void invoke_del_ipfrag(void *p);

/**
 *	Implement func.
 */
int dInitPREAProc(stMEMSINFO **pMEMSINFO, stHASHOINFO **pIPFRAGHASH, stTIMERNINFO **pIPFRAGTIMER, stHASHOINFO **pLPREAHASH, stHASHOINFO **pLPREASCTP, stHASHOINFO **pROAMHASH, st_SYSCFG_INFO *pSYSCFG)
{
	S32		dRet;

	SetUpSignal();

	/* nifo 메모리 할당 */
	if((*pMEMSINFO = nifo_init_zone((U8*)"PRE_A", SEQ_PROC_PRE_A, FILE_NIFO_ZONE)) == NULL) {
		log_print(LOGN_CRI, LH"nifo_init_zone NULL", LT);
		return -1;
	}

	//GIFO를 사용하기 위한 group 설정
    gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
    if( gpCIFO == NULL ){
        log_print(LOGN_CRI, LH"FAILED IN gifo_init_group. cifo=%s, gifo=%s",
                LT, FILE_CIFO_CONF, FILE_GIFO_CONF);
        return -2;
    }

	gATCPCnt = get_block_num(FILE_MC_INIT, "A_TCP");
    log_print(LOGN_CRI, "INIT., A_TCP: ProcCount[%d]", gATCPCnt);

	gARPCnt = get_block_num(FILE_MC_INIT, "A_RP");
    log_print(LOGN_CRI, "INIT., A_RP: ProcCount[%d]", gARPCnt);

	gAINETCnt = get_block_num(FILE_MC_INIT, "A_INET");
    log_print(LOGN_CRI, "INIT., A_INET: ProcCount[%d]", gAINETCnt);


	if((*pLPREAHASH = hasho_init(S_SSHM_LPREA_HASH, TAG_KEY_LPREA_CONF_SIZE, TAG_KEY_LPREA_CONF_SIZE, LPREA_CONF_SIZE, CONF_PREA_CNT, 0 )) == NULL ) {
		log_print(LOGN_CRI, LH"hasho_init LPREA_CONF NULL", LT);
		return -11;
	}

	if((*pLPREASCTP = hasho_init(S_SSHM_LPREA_SCTP, TAG_KEY_LPREA_SCTP_SIZE, TAG_KEY_LPREA_SCTP_SIZE, LPREA_SCTP_SIZE, CONF_PREA_CNT, 0 )) == NULL ) {
        log_print(LOGN_CRI, LH"hasho_init LPREA_SCTP NULL", LT);
        return -12;
    } 

	if((*pIPFRAGHASH = hasho_init(S_SSHM_IP_FRAG, IP_FRAG_KEY_SIZE, IP_FRAG_KEY_SIZE, IP_FRAG_SIZE, IP_FRAG_HASH_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init IP_FRAG NULL", LT);
		return -13;
	}

	if((*pROAMHASH = hasho_init(0, DEF_ROAMHASHKEY_SIZE, DEF_ROAMHASHKEY_SIZE, DEF_ROAMHASHDATA_SIZE, DEF_ROAMHASH_CNT, 0)) == NULL) {
		log_print(LOGN_CRI, LH"hasho_init ROAM HASH NULL", LT);
		return -14;
	}

	if((*pIPFRAGTIMER = timerN_init(IP_FRAG_HASH_CNT, IP_FRAG_COMMON_SIZE)) == NULL) {
		log_print(LOGN_CRI, LH"timerN_init NULL", LT);
		return -15;
	}

	vIPFRAGTimerReConstruct(*pIPFRAGHASH, *pIPFRAGTIMER);

	log_print(LOGN_CRI, "pMEMSINFO=%p:%p pLPREAHASH=%p:%p pIPFRAGHASH=%p:%p pIPFRAGTIMER=%p:%p",
			pMEMSINFO, *pMEMSINFO, pLPREAHASH, *pLPREAHASH, pIPFRAGHASH, *pIPFRAGHASH, pIPFRAGTIMER, *pIPFRAGTIMER);

	if((dRet = dGetSYSCFG(pSYSCFG)) < 0) {
		log_print(LOGN_CRI, LH"dGetSYSCFG dRet=%d", LT, dRet);
		return -16;
	}

	dRet = dGetSYSTYPE();
	if( dRet < 0 ) {
		log_print( LOGN_CRI, LH"dGetSYSTYPE() dRet:%d", LT, dRet);
		return -17;
	}

	dRet = dInit_IPPOOLBIT( S_SSHM_IPPOOL_BIT );
    if( dRet < 0 ) {
        log_print( LOGN_CRI, "ERROR IN dInit_IPPOOLBIT dRet:%d", dRet );
        return -18;
    }

	//ReadData(*pLPREAHASH);
	Read_MNData(*pROAMHASH);
	Read_SVRData(*pLPREAHASH);
	Read_SCTPData(*pLPREASCTP);
	Read_SVC_ONOFF();

	if( (dRet = shm_init(S_SSHM_GENINFO, sizeof(st_GEN_INFO), (void**)&gen_info)) < 0 ){
		log_print( LOGN_CRI, "ERROR IN shm_init(S_SSHM_GENINFO=0x%x) dRet:%d", S_SSHM_GENINFO, dRet );
		return -19;
	}

	if( (dRet = shm_init(S_SSHM_UTIL, sizeof(DEBUG_INFO), (void**)&pDEBUGINFO)) < 0 ){
		log_print( LOGN_CRI, "ERROR IN shm_init(S_SSHM_UTIL=0x%x) dRet:%d", S_SSHM_UTIL, dRet );
		return -20;
	}

	log_print(LOGN_CRI, "PRE_A initialized SUCCESS");
	
	return 0;
}

S32 dGetSYSTYPE()
{
	FILE		*fa;
	char		szBuf[1024];
	char		szType1[64], szType2[64];
	char		szInfo[64];
	int			i;

	if( (fa = fopen( FILE_SYS_CONFIG, "r" )) == NULL ) {
		log_print( LOGN_CRI, "LOAD SYSTEM TYPE : %s FILE OPEN FAIL (%s)", FILE_SYS_CONFIG, strerror(errno) );
		return -1;
	}

	i = 1;
	while( fgets( szBuf, 1024, fa ) ) {
		if( szBuf[0] != '#' ) {
			log_print( LOGN_CRI, "FAIL IN %s : %s FILE %d ROW FORMAT ERROR", __FUNCTION__, FILE_SYS_CONFIG, i );
			break;
		}

		i++;

		if( szBuf[1] == '#' )
			continue;
		else if( szBuf[1] == 'E' )
			break;
		else if( szBuf[1] == '@' ) {
			if( sscanf( &szBuf[2], "%s %s %s", szType1, szType2, szInfo ) == 3 ) {
				if( strcmp(szType1, "SYS") == 0 && strcmp(szType2, "TYPE") == 0 ) {
					if( strcmp( szInfo, "RP" ) == 0 ) {
						dSysTypeInfo = RP_FLAG;
					}
					else if( strcmp( szInfo, "PI" ) == 0 ) {
						dSysTypeInfo = PI_FLAG;
					}
				}
			}
		}
	}

	fclose( fa );

	if( dSysTypeInfo == 0 )
		return -2;

	return 1;
} 

S32 dGetSYSCFG(st_SYSCFG_INFO *pSYSCFG)
{
	FILE *fa;
	char szBuf[1024];
	char szType[64];
	char szInfo[64];
	int i;
	int dRet;


	dRet = -1;
	fa = fopen( FILE_SYS_CONFIG, "r" );
	if(fa == NULL) {
		log_print(LOGN_CRI, "LOAD SYSTEM CONFIG : %s FILE OPEN FAIL (%s)", FILE_SYS_CONFIG, strerror(errno));
		return -1;
	}

	i = 0;
	while(fgets(szBuf, 1024, fa) != NULL)
	{
		if(szBuf[0] != '#') {
			log_print(LOGN_WARN,"FAILED IN dGetSYSCFG() : %s File [%d] row format error", FILE_SYS_CONFIG, i);
			break;
		}

		i++;

		if(szBuf[1] == '#') continue;
		else if(szBuf[1] == 'E') break;
		else if(szBuf[1] == '@')
		{
			if(sscanf(&szBuf[2], "%s %s", szType, szInfo) == 2) 
			{
				if(strcmp(szType, "RANGE") == 0) 
				{
					pSYSCFG->range = Get_TAG_DEF_ALL_RANGE(szInfo);
					log_print(LOGN_CRI, "LOAD RANGE : [%d]", pSYSCFG->range);
				}
				else if(strcmp(szType, "TOTALMOD") == 0)
				{
					pSYSCFG->mod = atoi(szInfo);
					log_print(LOGN_CRI, "LOAD MOD : [%d]", pSYSCFG->mod);
				}
				else if(strcmp(szType, "VALIDMOD") == 0)
				{
					dRet = atoi(szInfo);
					pSYSCFG->bit[dRet] = 1;
					log_print(LOGN_CRI, "LOAD SUB MOD : [%d]", dRet);
				}
			}
		}
	}/* while */

	fclose(fa);

	return 0;
}

void Read_MNData(stHASHOINFO *pROAMHASH)
{
	int         		dRet;
    int         		start = 0;
    FILE        		*fp;
    char        		buf[BUFSIZ];
    char        		*bufcmp;
    char        		parse[BUFSIZ];
    LPREA_CONF  		LPREACONF;
	
	stHASHONODE			*pHASHNODE;
	st_ROAMHash_Key		stROAMHashKey;
	st_ROAMHash_Key		*pKey = &stROAMHashKey;
	st_ROAMHash_Data	stROAMHashData;
	st_ROAMHash_Data	*pData = &stROAMHashData;

	/* SHARED MEMORY INITIALIZE : pstIPPOOLBIT */
    _IPPOOL_ZERO( pstIPPOOLBIT );

	/* mnip hash reset */
	hasho_reset(pROAMHASH);
	

	if( (fp = fopen( FILE_FLT_MNIP, "r" )) == NULL ) {
		log_print( LOGN_CRI, LH"OPEN ERROR[%s][%s]", LT, FILE_FLT_MNIP, strerror(errno) );
		exit(0);
	}

	while( fgets( buf, BUFSIZ, fp ) ) {
		switch(buf[0]){
        case '(':
            if(start != 0) {
                log_print(LOGN_CRI, LH"FILE Format Error(%s)", LT, parse);
                parse[0] = 0x00;
            }
            start = 1;
            sprintf(parse, "%s", buf);
            break;
		case ')':
            if(start == 1) {
                sprintf(parse, "%s%s", parse, buf);

                bufcmp = &buf[2];

                if(!strncmp(bufcmp, "LPREA", strlen("LPREA"))) {
                    memset(&LPREACONF, 0x00, LPREA_CONF_SIZE);
                    LPREA_CONF_GRASP_LEX(parse, strlen(parse), (char *)&LPREACONF);

                    if( dSysTypeInfo == RP_FLAG ) {
                        if( LPREACONF.RpPiFlag == RP_FLAG && (LPREACONF.SysType == TYPE_PCF || LPREACONF.SysType == TYPE_LAC) ) {
                            /* ADD IPPOOLBIT ARRAY : RP INFO */
                            dRet = dSetIPPOOLList(LPREACONF.SIP, LPREACONF.NetMask, 1, pstIPPOOLBIT );
                            if( dRet < 0 )
                                log_print( LOGN_CRI, LH"ERROR IN dSetIPPOOLList dRet:%d", LT, dRet );

							if(LPREACONF.SysType == TYPE_LAC) {
								pKey->ip = LPREACONF.SIP;
								pKey->reserved = 0;
								pData->systype = LPREACONF.SysType;
								pData->reserved = 0;

								if((pHASHNODE = hasho_add(pROAMHASH, (U8 *)pKey, (U8 *)pData)) == NULL) {
									log_print(LOGN_CRI, LH"ERROR MNIP hasho_add NULL ", LT);
								}
							}
                        }
                    }
                    else if( dSysTypeInfo == PI_FLAG ) {
                        if( LPREACONF.RpPiFlag == PI_FLAG ) {
                            if( LPREACONF.SysType == TYPE_PDSN || LPREACONF.SysType == TYPE_MNIP || LPREACONF.SysType == TYPE_LNS || LPREACONF.SysType == TYPE_CRX || LPREACONF.SysType == TYPE_PDIF) {
                                /* ADD IPPOOLBIT ARRAY : PI INFO */
                                dRet = dSetIPPOOLList(LPREACONF.SIP, LPREACONF.NetMask, 1, pstIPPOOLBIT );
                                if( dRet < 0 )
                                    log_print( LOGN_CRI, LH"ERROR IN dSetIPPOOLList dRet:%d", LT, dRet );

								if(LPREACONF.SysType == TYPE_LNS || LPREACONF.SysType == TYPE_CRX) {
									pKey->ip = LPREACONF.SIP;
									pKey->reserved = 0;
									pData->systype = LPREACONF.SysType;
									pData->reserved = 0;

									if((pHASHNODE = hasho_add(pROAMHASH, (U8 *)pKey, (U8 *)pData)) == NULL) {
										log_print(LOGN_CRI, LH"ERROR MNIP hasho_add NULL ", LT);
									}
								}
                            }
                        }
                    }
                }
                start = 0;
            } else {
                log_print(LOGN_CRI, LH"FILE Format Error(%s)", LT, buf);
            }
            break;
        default:
            if(start == 1) {
                sprintf(parse, "%s%s", parse, buf);
            }
            break;
        }
    }

	fclose(fp);
}

void Read_SVRData( stHASHOINFO *pLPREAHASH )
{
    U32         uiCount;
    int         start = 0;
    FILE        *fp;
    char        buf[BUFSIZ];
    char        *bufcmp;
    char        parse[BUFSIZ];
    LPREA_CONF  LPREACONF;

	stHASHONODE *pHASHNODE;

	/* SHARED MEMORY INITIALIZE : pstIPPOOLBIT, pLPREAHASH */
	hasho_reset( pLPREAHASH );

	if( (fp = fopen( FILE_FLT_SVR, "r" )) == NULL ) {
        log_print( LOGN_CRI, LH"OPEN ERROR[%s][%s]", LT, FILE_FLT_SVR, strerror(errno) );
        exit(0);
    }

    while(fgets(buf, BUFSIZ, fp)){
        switch(buf[0]){
        case '(':
            if(start != 0) {
                log_print(LOGN_CRI, LH"FILE Format Error(%s)", LT, parse);
                parse[0] = 0x00;
            }
            start = 1;
            sprintf(parse, "%s", buf);
            break;
		case ')':
            if(start == 1) {
                sprintf(parse, "%s%s", parse, buf);

                bufcmp = &buf[2];

                if(!strncmp(bufcmp, "LPREA", strlen("LPREA"))) {
                    if(pLPREAHASH != NULL ) {
                        memset(&LPREACONF, 0x00, LPREA_CONF_SIZE);
                        LPREA_CONF_GRASP_LEX(parse, strlen(parse), (char *)&LPREACONF);

                        if( dSysTypeInfo == RP_FLAG ) {
                            if( LPREACONF.RpPiFlag == RP_FLAG && (LPREACONF.SysType == TYPE_PDSN || LPREACONF.SysType == TYPE_LNS) ) {
								/* ADD HASH : PDSN INFO */
                                    if((pHASHNODE = hasho_add( pLPREAHASH, (U8 *)&LPREACONF, (U8 *)&LPREACONF)) == NULL)
                                        log_print(LOGN_CRI, LH"LPREA hasho_add NULL", LT);
                            }
                        }
                        else if( dSysTypeInfo == PI_FLAG ) {
                            if( LPREACONF.RpPiFlag == PI_FLAG ) {

                                if( LPREACONF.SysType == TYPE_AAA || LPREACONF.SysType == TYPE_SVC || LPREACONF.SysType == TYPE_PDIF ) {
									/* ADD HASH : AAA & SVC INFO */
                                    if((pHASHNODE = hasho_add( pLPREAHASH, (U8 *)&LPREACONF, (U8 *)&LPREACONF)) == NULL)
                                        log_print(LOGN_CRI, LH"LPREA hasho_add NULL", LT);
                                }
                            }
                        }
                    }
                }
                start = 0;
            } else {
                log_print(LOGN_CRI, LH"FILE Format Error(%s)", LT, buf);
            }
            break;
        default:
            if(start == 1) {
                sprintf(parse, "%s%s", parse, buf);
            }
            break;
        }
    }

    fclose(fp);

	uiCount = hasho_get_occupied_node_count(pLPREAHASH);
}

void Read_SCTPData( stHASHOINFO *pLPREASCTP )
{
    U32         uiCount;
    int         start = 0;
    FILE        *fp;
    char        buf[BUFSIZ];
    char        *bufcmp;
    char        parse[BUFSIZ];
    LPREA_SCTP  LPREASCTP;

    stHASHONODE *pHASHNODE;

    /* SHARED MEMORY INITIALIZE : pstIPPOOLBIT, pLPREAHASH */
	hasho_reset( pLPREASCTP );


    if( (fp = fopen( FILE_FLT_SCTP, "r" )) == NULL ) {
        log_print( LOGN_CRI, LH"OPEN ERROR[%s][%s]", LT, FILE_FLT_SCTP, strerror(errno) );
        exit(0);
    }

    while(fgets(buf, BUFSIZ, fp)){
        switch(buf[0]){
        case '(':
            if(start != 0) {
                log_print(LOGN_CRI, LH"FILE Format Error(%s)", LT, parse);
                parse[0] = 0x00;
            }
            start = 1;
            sprintf(parse, "%s", buf);
            break;
		case ')':
            if(start == 1) {
                sprintf(parse, "%s%s", parse, buf);

                bufcmp = &buf[2];

                if(!strncmp(bufcmp, "LPREA", strlen("LPREA"))) {
                    if(pLPREASCTP != NULL ) {
                        memset(&LPREASCTP, 0x00, LPREA_SCTP_SIZE);
                        LPREA_SCTP_GRASP_LEX(parse, strlen(parse), (char *)&LPREASCTP);

						if((pHASHNODE = hasho_add( pLPREASCTP, (U8 *)&LPREASCTP, (U8 *)&LPREASCTP)) == NULL)
							log_print(LOGN_CRI, LH"LPREA SCTP hasho_add NULL", LT);
                    }
                }
                start = 0;
            } else {
                log_print(LOGN_CRI, LH"FILE Format Error(%s)", LT, buf);
            }
            break;
        default:
            if(start == 1) {
                sprintf(parse, "%s%s", parse, buf);
            }
            break;
        }
    }

    fclose(fp);

    uiCount = hasho_get_occupied_node_count(pLPREASCTP);
}

void Read_SVC_ONOFF()
{
	int     scan_cnt;
	char    buf[1024];
	int	    dSvcCode;
	int 	dOnOffFlag;
	FILE    *fp;
	int 	i=0, ln=0;

	if( (fp = fopen( FILE_FLT_SERVICE, "r" )) == NULL ) {
		log_print( LOGN_CRI, LH"OPEN ERROR[%s][%s]", LT, FILE_FLT_SERVICE, strerror(errno) );
		exit(0);
	}

	for(i=0; i<MAX_SERVICE_CNT; i++) {
		stSVCONOFFINFO[i].dSvcCode = 0;
		stSVCONOFFINFO[i].dOnOffFlag = 0;
	}

	i=0;
	while(fgets(buf, 1024, fp) != NULL ) {
		ln++;
		/*
		 * from Source to Target : sscanf
		 */
		if( buf[0] != '#' ) {
			printf("SYNTAX ERROR FILE:%s, LINE:%d - FIRST CHARACTER IS MUST '#'!\n", FILE_FLT_SERVICE, ln);
			return;
		}
		else if( buf[1] == '#' ) {
			continue;
		}
		else if( buf[1] == 'E' ) {
			/*
			 * EOF
			 */
			break;
		}
		else if( buf[1] == '@' ) {
			scan_cnt = sscanf( &buf[2], "%d %d", &dSvcCode, &dOnOffFlag);
			if(scan_cnt == 2) {
				stSVCONOFFINFO[i].dSvcCode = dSvcCode;
				stSVCONOFFINFO[i].dOnOffFlag = dOnOffFlag;
				log_print( LOGN_CRI, "	SERVICE ON/OFF [%d][%d][%d]", i, stSVCONOFFINFO[i].dSvcCode, stSVCONOFFINFO[i].dOnOffFlag);
				i++;
			}
		}
		else {
			printf("SYNTAX ERROR FILE:%s, LINK:%d\n", FILE_FLT_SERVICE, ln); 
			break;
		}

	} /* while */ 
	fclose(fp);
}
    
void UserControlledSignal(int sign)
{
    JiSTOPFlag = 0;
    FinishFlag = sign;
}

void FinishProgram()
{
    log_print(LOGN_CRI, "PROGRAM IS NORMALLY TERMINATED, Cause = %d", FinishFlag);
    exit(0);
}

void IgnoreSignal(int sign)
{
    if (sign != SIGALRM)
        log_print(LOGN_CRI, "UNWANTED SIGNAL IS RECEIVED, signal = %d", sign);
    signal(sign, IgnoreSignal);
}

void SetUpSignal()
{
    JiSTOPFlag = 1;

    /* WANTED SIGNALS   */
    signal(SIGTERM, UserControlledSignal);
    signal(SIGINT,  UserControlledSignal);
    signal(SIGQUIT, UserControlledSignal);

    /* UNWANTED SIGNALS */
    signal(SIGHUP,  IgnoreSignal);
    signal(SIGALRM, IgnoreSignal);
    signal(SIGPIPE, IgnoreSignal);
    signal(SIGPOLL, IgnoreSignal);
    signal(SIGPROF, IgnoreSignal);
    signal(SIGUSR1, IgnoreSignal);
    signal(SIGUSR2, IgnoreSignal);
    signal(SIGVTALRM, IgnoreSignal);
    signal(SIGCLD, SIG_IGN);

    log_print(LOGN_CRI, "SIGNAL HANDLER WAS INSTALLED");
}

void vIPFRAGTimerReConstruct(stHASHOINFO *pHASH, stTIMERNINFO *pTIMER)
{
	int					i;
	OFFSET				offset;
	stHASHONODE			*p;
	stMEMGNODEHDR		*pMEMGNODEHDR;

	IP_FRAG_COMMON		COMMON;
	IP_FRAG_KEY			*pKey;
	IP_FRAG				*pData;

	U8					szSIP[INET_ADDRSTRLEN];
	U8					szDIP[INET_ADDRSTRLEN];

	/* IPFRAG */
	log_print(LOGN_INFO, "REBUILD IPFRAG TIMER hashcnt=%u", pHASH->uiHashSize);
	for(i = 0; i < pHASH->uiHashSize; i++)
	{
		offset = *(OFFSET *)(((OFFSET *)HASHO_PTR(pHASH, pHASH->offset_psthashnode)) + i);

		while(offset)
		{
			p = (stHASHONODE *)HASHO_PTR(pHASH, offset);

			pKey = (IP_FRAG_KEY *)HASHO_PTR(pHASH, p->offset_Key);
			pData = (IP_FRAG *)HASHO_PTR(pHASH, p->offset_Data);
			pMEMGNODEHDR = (stMEMGNODEHDR *)((U8 *)p - stMEMGNODEHDR_SIZE);

			if((MEMG_ID == pMEMGNODEHDR->uiID) && (MEMG_ALLOCED == pMEMGNODEHDR->ucIsFree))
			{
				log_print(LOGN_INFO, "REBUILD IPFRAG TIMER SIP=%s:%u DIP=%s.%u IDENTIFICATION=%u", 
						util_cvtipaddr(szSIP, pKey->sip), pKey->sip, util_cvtipaddr(szDIP, pKey->dip), pKey->dip, pKey->identification);

				memcpy(&COMMON.IPFRAGKEY, pKey, IP_FRAG_KEY_SIZE);
				pData->timerNID = timerN_add(pTIMER, invoke_del_ipfrag, (U8*)&COMMON, IP_FRAG_COMMON_SIZE, time(NULL) + DEF_IP_FRAG_TIMEOUT);

			}
			offset = p->offset_next;
		}
	}
}
