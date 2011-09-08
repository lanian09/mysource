#ifndef _IHTTP_UTIL_H_
#define _IHTTP_UTIL_H_


/**
 * Define constants
 */
#define DEF_LIMIT_10		(10*1000000)

/**
 * Declare functions
 */
extern void MakeTcpHashKey(TCP_INFO *pTCPINFO, HTTP_TSESS_KEY *pHTTPTSESSKEY);
extern void MakeHttpHashKey(HTTP_TSESS_KEY *pHTTPTSESSKEY, U16 usTransID, HTTP_TRANS_KEY *pHTTPTRANSKEY);
extern S32 dGetData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen);
extern S32 dGetLengthData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen);
extern S32 dGetChunkedData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen);
extern S32 dGetMultiData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen);
extern void InitBuffer(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, S32 isNotREQ);
extern void InitCount(HTTP_TSESS *pHTTPTSESS);
extern void UpCount(LOG_IHTTP_TRANS *pLOGHTTP, HTTP_TSESS *pHTTPTSESS);
extern void UpdateHttpTsess(TCP_INFO *pTCPINFO, HTTP_TSESS *pHTTPTSESS);
extern S32 dCheckReqHeader(HTTP_TSESS *pHTTPTSESS, U8 *pData, U16 usLen);
extern S32 dGetReqHeaderInfo(HTTP_TSESS *pHTTPTSESS, U8 *pNode, S32 *pdCLen, U8 *isbuf, U32 *maxLen, S32 *multilen, U8 *multi, S32 *zip);
extern S32 dCheckResHeader(HTTP_TSESS *pHTTPTSESS, U8 *pData, U16 usLen);
extern S32 dGetResHeaderInfo(HTTP_TSESS *pHTTPTSESS, U8 *pNode, S32 *pdCLen, U8 *isbuf, U32 *maxLen, S32 *multilen, U8 *multi, S32 *zip);
extern S32 GetURL(U8 *inUrl, S32 urlLen, S32 urlType, U8 *hostName, S32 hostNameLen, U8 *outUrl, S32 maxLen);
extern S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime);
extern U32 GetGap32Time(U32 endtime, U32 endmtime, U32 starttime, U32 startmtime);
extern U16 GetFailCode(LOG_IHTTP_TRANS *pLOGHTTP, S32 httpTransStatus);
extern U16 GetHtttpTransStatus(st_MSG_INFO *pMSGINFO, S32 isNotREQ);
extern U16 GetLogHttpStatus(st_MSG_INFO *pMSGINFO, S32 isNotREQ);
extern char *PrintRtx(U8 ucRtxType);
extern char *PrintIsHDR(S32 isHDR);
extern char *PrintTSESSStatus(S32 status);
extern char *PrintLenType(S32 lentype);
extern char *PrintBuffering(S32 isbuffer);
extern char *PrintEndStatus(S32 status);
extern S32	dGetProcID(S32 appCode, U32 uiClientIP);
extern S32 dGetCALLProcID(U32 uiClientIP);
//S32 dGetPlatformTypeOld(S32 l4Code);
extern U8 *pAllocDataNode(stMEMSINFO *pMEMSINFO, S32 type);
extern S32 dCheckRtx(U8 *pData, U16 usLen, U8 ucRtx);

#endif	/* _IHTTP_UTIL_H_ */
