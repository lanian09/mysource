#ifndef _HTTP_UTIL_H_
#define _HTTP_UTIL_H_

#define DEF_LIMIT_10		(10*1000000)

extern void MakeTcpHashKey(TCP_INFO *pTCPINFO, HTTP_TSESS_KEY *pHTTPTSESSKEY);
extern void MakeHttpHashKey(HTTP_TSESS_KEY *pHTTPTSESSKEY, U16 usTransID, HTTP_TRANS_KEY *pHTTPTRANSKEY);
extern S32 dGetData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen);
extern S32 dGetLengthData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen);
extern S32 dGetChunkedData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen);
extern S32 dGetMultiData(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, U8 *szInData, U32 uiDataLen, S32 *pdLen);
extern S32 dCheckReqHeader(HTTP_TSESS *pHTTPTSESS, U8 *pData, U16 usLen);
extern S32 dGetReqHeaderInfo(HTTP_TSESS *pHTTPTSESS, U8 *pNode, S32 *pdCLen, U8 *isbuf, U32 *maxLen, S32 *multilen, U8 *multi, S32 *zip);
extern S32 dCheckResHeader(HTTP_TSESS *pHTTPTSESS, U8 *pData, U16 usLen);
extern S32 dGetResHeaderInfo(HTTP_TSESS *pHTTPTSESS, U8 *pNode, S32 *pdCLen, U8 *isbuf, U32 *maxLen, S32 *multilen, U8 *multi, S32 *zip);
extern S32 GetURL(U8 *inUrl, S32 urlLen, S32 urlType, U8 *hostName, S32 hostNameLen, U8 *outUrl, S32 maxLen);
extern void InitBuffer(stMEMSINFO *pMEMSINFO, st_MSG_INFO *pMSGINFO, S32 isNotREQ);
extern void InitCount(HTTP_TSESS *pHTTPTSESS);
extern void UpCount(LOG_HTTP_TRANS *pLOGHTTP, HTTP_TSESS *pHTTPTSESS);
extern void UpdateHttpTsess(TCP_INFO *pTCPINFO, HTTP_TSESS *pHTTPTSESS);
extern U8 *PrintRtx(U8 ucRtxType);
extern U8 *PrintIsHDR(S32 isHDR);
extern U8 *PrintTSESSStatus(S32 status);
extern U8 *PrintLenType(S32 lentype);
extern U8 *PrintBuffering(S32 isbuffer);
extern U8 *PrintEndStatus(S32 status);
extern S64 GetGapTime(STIME endtime, MTIME endmtime, STIME starttime, MTIME startmtime);
extern U16 GetFailCode(LOG_HTTP_TRANS *pLOGHTTP, S32 httpTransStatus);
extern U16 GetHtttpTransStatus(st_MSG_INFO *pMSGINFO, S32 isNotREQ);
extern U16 GetLogHttpStatus(st_MSG_INFO *pMSGINFO, S32 isNotREQ);
extern S32 dGetSeqProcID(S32 appCode, U32 uiClientIP);
//extern S32 dGetPlatformType(S32 l4Code, S32 l7Code);
extern U8 *pAllocDataNode(stMEMSINFO *pMEMSINFO, S32 type);


#endif /* _HTTP_UTIL_H_ */
