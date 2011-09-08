#include "srchmsg.h"

st_MethodType_t st_MethodType[METHOD_NUM] = {
    {METHOD_OPTIONS, "OPTION"},
    {METHOD_GET, "GET"},
    {METHOD_HEAD, "HEAD"},
    {METHOD_POST, "POST"},
    {METHOD_PUT, "PUT"},
    {METHOD_DELETE, "DELETE"},
    {METHOD_TRACE, "TRACE"},
    {METHOD_CONNECT, "CONNECT"},
    {METHOD_RESUME, "RESUME"}
};

char PARSE_START_STR[2] = "[";
char PARSE_END_STR[2] = "]";
char TIME_PARSE_STR[2] = ",";
char STATUS_PARSE_STR[2] = ":";
char PACKET_PARSE_STR[2] = ":";
char IP_PARSE_STR[2] = ":";

/*
Method         = "OPTIONS"          ; Section 9.2
                | "GET"             ; Section 9.3
                | "HEAD"            ; Section 9.4
                | "POST"            ; Section 9.5
                | "PUT"             ; Section 9.6
                | "DELETE"          ; Section 9.7
                | "TRACE"           ; Section 9.8
                | extension-method 
                extension-method = token
*/

e_ReturnCode_t
DataParsing(char *sGetData, 
            char *sLog, 
            unsigned int *iDataLen,
            unsigned int *iOffset,
            unsigned int iEnd)
{
    e_ReturnCode_t e_Ret = WAP1_SUCC;
    unsigned int i = 0;
    unsigned int iDataCnt = 0;

    for(i=0; i<iEnd; i++)
    {
        if( sLog[i] == PARSE_START_STR[0] )
        {
            break;
        }
        else
        {
            continue;
        }
    }
    
    for(i = i+1 ; i<iEnd; i++)
    {
        if( sLog[i] == PARSE_END_STR[0] )
        {
            break;
        }
        sGetData[iDataCnt] = sLog[i];
        iDataCnt++;
    }
        
    *iDataLen = iDataCnt;
    *iOffset = i;
    sGetData[i] = '\0';

    return e_Ret;
}
   
e_ReturnCode_t    
MethodParsing(int *iMethodType, unsigned char *sMethodLog)
{
    e_ReturnCode_t  e_Ret = WAP1_SUCC;
    unsigned char   tmpStr[16];
    unsigned int i = 0;
    
    memset(tmpStr, 0x00, 16);
    for(i = 0; i<METHOD_NUM ; i++)
    {
        if(!strcmp(sMethodLog, st_MethodType[i].sMethodStr))
        {
            break;
        }
    }
    PASS_EXCEPTION(i == METHOD_NUM, PARSE_ERR);

    *iMethodType = st_MethodType[i].iCode;
    
    PASS_CATCH(PARSE_ERR)
    e_Ret = WAP1_FAIL;
    PASS_CATCH_END;

    return e_Ret;
}



e_ReturnCode_t    
StatusParsing
(
 int            *iResultCode, 
 unsigned char  *sStatus, 
 unsigned char  *sStatusLog
)
{
    e_ReturnCode_t  e_Ret = WAP1_SUCC;
    unsigned char   tmpStr[16];
    unsigned int i = 0;
    
    memset(tmpStr, 0x00, 16);
    for(i = 0; i<strlen(sStatusLog) ; i++)
    {
        if( sStatusLog[i] == STATUS_PARSE_STR[0] )
        {
            break;
        }
        tmpStr[i] = sStatusLog[i];
    }

    tmpStr[i] = '\0';
    
    *iResultCode = atoi(tmpStr);
    PASS_EXCEPTION(*iResultCode == 0, PARSE_ERR);

    sStatusLog += (i+strlen(STATUS_PARSE_STR));
    PASS_EXCEPTION(strlen(sStatusLog) == 0, PARSE_ERR);
    
    snprintf(sStatus, MAX_LOG_STATUS_LEN, "%s", sStatusLog);
    
    PASS_CATCH(PARSE_ERR)
    e_Ret = WAP1_FAIL;
    PASS_CATCH_END;

    return e_Ret;
}

e_ReturnCode_t
ContentLenParsing
(pst_LogParse_t pstLogParse, 
 unsigned char *sPacketLog)
{
    e_ReturnCode_t  e_Ret = WAP1_SUCC;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned char   tmpStr[16];
    unsigned char   tmpPacket[4][16];

    memset(tmpPacket, 0x00, sizeof(tmpPacket));

    for(i = 0; i<4 ; i++)
    {
        memset(tmpStr, 0x00, 16);
        PASS_EXCEPTION(strlen(sPacketLog) == 0, PARSE_ERR);
        for(j = 0; j<strlen(sPacketLog) ; j++)
        {
            if( sPacketLog[j] == PACKET_PARSE_STR[0] )
            {
                break;
            }
            tmpStr[j] = sPacketLog[j];
        }
        
        /* add offset */
        sPacketLog = sPacketLog + j + strlen(TIME_PARSE_STR);
        
        tmpStr[j] = '\0';
        PASS_EXCEPTION(strlen(tmpStr) == 0, PARSE_ERR);
        snprintf(tmpPacket[i], 16, "%s", tmpStr);
    }


    /* Assign Value */
    pstLogParse->iWapReqSize = atoi(tmpPacket[0]);
    pstLogParse->iWapResSize = atoi(tmpPacket[2]);
    pstLogParse->iContentsLen = atoi(tmpPacket[3]);
    

    PASS_CATCH(PARSE_ERR)
    e_Ret = WAP1_FAIL;
    PASS_CATCH_END;

    return e_Ret;


}

e_ReturnCode_t
TimeParsing(pst_TimeVal_t   pst_TimeVal,
            unsigned char   *sTimeLog)
{
    e_ReturnCode_t  e_Ret = WAP1_SUCC;
    unsigned int    i = 0;
    unsigned int    j = 0;
    unsigned char   tmpStr[16];
    
    memset(tmpStr, 0x00, 16);
    
    /* Get Year */
    memcpy(tmpStr, sTimeLog, 4);
    tmpStr[4] = '\0';
    pst_TimeVal->tm_FirstTime.tm_year = atoi(tmpStr) - 1900;
    
    /* Get Month */
    memset(tmpStr, 0x00, 16);
    sTimeLog = sTimeLog + 4;
    memcpy(tmpStr, sTimeLog, 2);
    tmpStr[2] = '\0';
    pst_TimeVal->tm_FirstTime.tm_mon = atoi(tmpStr) - 1;

    /* Get Day */
    memset(tmpStr, 0x00, 16);
    sTimeLog = sTimeLog + 2;
    memcpy(tmpStr, sTimeLog, 2);
    tmpStr[2] = '\0';
    pst_TimeVal->tm_FirstTime.tm_mday = atoi(tmpStr);

    /* Get Hour */
    memset(tmpStr, 0x00, 16);
    sTimeLog = sTimeLog + 3;
    memcpy(tmpStr, sTimeLog, 2);
    tmpStr[2] = '\0';
    pst_TimeVal->tm_FirstTime.tm_hour = atoi(tmpStr);
    
    /* Get Minute */
    memset(tmpStr, 0x00, 16);
    sTimeLog = sTimeLog + 3;
    memcpy(tmpStr, sTimeLog, 2);
    tmpStr[2] = '\0';
    pst_TimeVal->tm_FirstTime.tm_min = atoi(tmpStr);
    
    /* Get Second */
    memset(tmpStr, 0x00, 16);
    sTimeLog = sTimeLog + 3;
    memcpy(tmpStr, sTimeLog, 2);
    tmpStr[2] = '\0';
    pst_TimeVal->tm_FirstTime.tm_sec = atoi(tmpStr);
  
    /* Get Sub Time */
    sTimeLog = sTimeLog + 3;
    for(i = 0; i<4 ; i++)
    {
        memset(tmpStr, 0x00, 16);
        PASS_EXCEPTION(strlen(sTimeLog) == 0, PARSE_ERR);
        for(j = 0; j<strlen(sTimeLog) ; j++)
        {
            if( sTimeLog[j] == TIME_PARSE_STR[0] )
            {
                break;
            }
            tmpStr[j] = sTimeLog[j];
        }
        /* add offset */
        sTimeLog = sTimeLog + j + strlen(TIME_PARSE_STR);
        
        tmpStr[j] = '\0';
        PASS_EXCEPTION(strlen(tmpStr) == 0, PARSE_ERR);
        pst_TimeVal->transTime[i] = atoi(tmpStr);
    }
    
    PASS_EXCEPTION(strlen(tmpStr) == 0, PARSE_ERR);

    PASS_CATCH(PARSE_ERR)
    e_Ret = WAP1_FAIL;
    PASS_CATCH_END;

    return e_Ret;
}
	

e_ReturnCode_t
IpParsing(int          *iSrcIp,
		  int		   *dSrcPort, 
		  char		   *sGetData)
{
    e_ReturnCode_t  e_Ret = WAP1_SUCC;
    unsigned int i = 0;
    unsigned char   tmpStr[16];
	
    for(i = 0; i<strlen(sGetData); i++)
    {
        PASS_EXCEPTION( i > 15, WRONG_SIZE);
        
        if( sGetData[i] == IP_PARSE_STR[0] )
        {
            break;
        }
		tmpStr[i] = sGetData[i];
    }
	tmpStr[i] = '\0';
	
    /* Get IP */
#ifdef _DEBUG
	dAppLog(LOG_INFO,"Parse IP [%s]", tmpStr);
#endif
    *iSrcIp = inet_addr(tmpStr);
    PASS_EXCEPTION(*iSrcIp == -1, WRONG_VAL);

	/* Get Port */
    sGetData += (i+strlen(IP_PARSE_STR));
	
	*dSrcPort = atoi(sGetData);
    PASS_EXCEPTION(*dSrcPort == 0, WRONG_VAL);


    PASS_CATCH(WRONG_SIZE)
    e_Ret = WAP1_FAIL;
    
    PASS_CATCH(WRONG_VAL)
    e_Ret = WAP1_FAIL;
    
    PASS_CATCH_END;

    return e_Ret;
}

e_ReturnCode_t
ParsingDbData
(unsigned char       *sLog,
 st_LogParse_t       *pstLogParse
)
{
    char    sGetData[MAX_ONEDATA_LEN];
    char    *p_Tmp = NULL;
    e_ReturnCode_t  e_Ret = WAP1_SUCC;
    unsigned int    iDataLen = 0;
    unsigned int    iOffset = 0;
    unsigned int    iCharCount = 0;
    st_TimeVal_t    st_TimeVal;
    time_t          t_TmpTime = 0;
    
    
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    p_Tmp = sLog; 
    
    /* Get MIN */
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data MIN [%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
    snprintf(pstLogParse->szMin, MAX_LOG_MIN_LEN, "%s", sGetData);


    /* Get MIP & MPort */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data MIP[%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
	/* Parsing IP & Port */
	e_Ret = IpParsing(&pstLogParse->uiSrcIp,
        &pstLogParse->uiSrcPort, sGetData);
    PASS_EXCEPTION(e_Ret != WAP1_SUCC, PARSE_DATA_ERR);
    
    
    /* Get Sub Number */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data Sub No [%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
    snprintf(pstLogParse->szSubNo, MAX_LOG_SUBNO_LEN, "%s", sGetData);


    /* Get Time */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data Time [%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
    
    /* Parse Time Value */
    e_Ret = TimeParsing(&st_TimeVal, sGetData);
    PASS_EXCEPTION(e_Ret != WAP1_SUCC, PARSE_DATA_ERR);
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data First Time [%d]\n", 
                (int)pthread_self(), (int)mktime(&(st_TimeVal.tm_FirstTime)) );
    fprintf(stdout, "%d, Another Time [%d][%d][%d][%d]\n", 
                (int)pthread_self(),
                st_TimeVal.transTime[0],
                st_TimeVal.transTime[1],st_TimeVal.transTime[2],
                st_TimeVal.transTime[3]);
    fflush(stdout);
#endif
    t_TmpTime = mktime(&(st_TimeVal.tm_FirstTime));
    //pst_UDR->tReqTime = t_TmpTime + (3600 * 9);
    pstLogParse->tReqTime = t_TmpTime;

    /* Calculate Response Time */
    pstLogParse->tResTime = pstLogParse->tReqTime + 
            (st_TimeVal.transTime[1]) / 1000;
#ifdef _DEBUG
    fprintf(stdout, "%d, RequestTime [%d], ResponseTime[%d]\n", 
                (int)pthread_self(),
                (int)pst_UDR->tReqTime, (int)pst_UDR->tResTime);
#endif    
    
    

    /* Get Packet */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data Packet [%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
    
    /* Parse Packet Data */
    e_Ret = ContentLenParsing(pstLogParse, sGetData);
    PASS_EXCEPTION(e_Ret != WAP1_SUCC, PARSE_DATA_ERR);
   
   /* Get Status */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data Status [%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
    
    /* Parse Status */
    e_Ret = StatusParsing(&pstLogParse->uiResultCode, pstLogParse->szStatus, sGetData);
    PASS_EXCEPTION(e_Ret != WAP1_SUCC, PARSE_DATA_ERR);
    
    /*
    if( !strcasecmp(sStatus, "complete") )
    {
        pst_UDR->dTranComplete = 1;
    }
    else
    {
        pst_UDR->dTranComplete = 0;
    }
    */
   
   
   /* Get URL */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data URL [%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
    snprintf(pstLogParse->szURL, MAX_LOG_URL_LEN, "%s", sGetData);


   /* Get Method */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data Method [%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
    snprintf(pstLogParse->szMethodType, MAX_LOG_METHOD_LEN, "%s", sGetData);

    /*
    e_Ret = MethodParsing(&pst_UDR->dMethodType, sGetData);
    PASS_EXCEPTION(e_Ret != WAP1_SUCC, PARSE_DATA_ERR);
    */

   /* Get Retry */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
#ifdef _DEBUG
    fprintf(stdout, "%d, Parsing Data Retry [%s]\n", (int)pthread_self(),sGetData);
    fflush(stdout);
#endif
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
   
   /* Get UserAgent */
    iCharCount = iCharCount + iOffset;
    p_Tmp = sLog + iCharCount;
    iDataLen = 0;
    iOffset = 0;
    memset(sGetData, 0x00, MAX_ONEDATA_LEN);
    DataParsing(sGetData, p_Tmp, &iDataLen, &iOffset, strlen(p_Tmp));
    PASS_EXCEPTION(sGetData[0] == '\0', PARSE_DATA_ERR);
    snprintf(pstLogParse->szUserAgent, MAX_LOG_USERAGENT_LEN, "%s", sGetData);
    
    PASS_CATCH(PARSE_DATA_ERR)
    fprintf(stdout, "Data Parsing Error\n");
    e_Ret = WAP1_FAIL;

    PASS_CATCH_END;

    return e_Ret;
}

