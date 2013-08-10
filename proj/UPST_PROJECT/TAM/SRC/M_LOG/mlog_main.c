/**		@file	cilog_main.c
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		Copyright (c) 2006~ by Upresto Inc. Korea
 * 		All rights reserved
 *
 * 		 $Id: mlog_main.c,v 1.4 2011/09/06 13:19:39 uamyd Exp $
 *
 * 		@Author		$Author: uamyd $
 * 		@version	$Revision: 1.4 $
 * 		@date		$Date: 2011/09/06 13:19:39 $
 * 		@warning	.
 * 		@ref		http_main.c l4.h http_init.c http_func.c
 * 		@todo		library가 만들어지지 않은 상태, library 생성 이후 함수 대치
 *					기존 데이터 파일이 있는 경우 메모리 재구성(RecordCnt)해서 Append하는 로직
 *					프로세스 기동시 DAT파일만 있는 경우 메모리 재구성해서 FIN 생성 후
 *					특정디렉토리를 움직이게 하는 처리
 *
 * 		@section	Intro(소개)
 * 		- HTTP Transaction을 관리 하는 프로세스
 *
 * 		@section	Requirement
 * 		 @li library 생성 이후 함수 대치
 * 		 @li Service 추가시 Match 함수 UseFlag 변경해야 함.
 *
 **/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "typedef.h"
#include "mems.h"
#include "cifo.h"
#include "gifo.h"
#include "nifo.h"
#include "path.h"
#include "verlib.h"
#include "procid.h"
#include "rppi_def.h"
#include "sshmid.h"
#include "nsocklib.h"
#include "msgdef.h"
#include "loglib.h"
#include "common_stg.h"
#include "mlog_func.h"
#include "mlog_msgq.h"
#include "mlog_init.h"

S32			giFinishSignal;					/**< Finish Signal */
S32			giStopFlag;						/**< main loop Flag 0: Stop, 1: Loop */

stHASHOINFO         *pDEFECTINFO;			/**< Diameter LOG 생성시 threshold를 갖고 오기 위한 hasho struct pointer */
stMEMSINFO   *pMEMSINFO;
stCIFO       *gpCIFO;

S32			dMyQID;
S32			dSIDBQID;

S32			gdSysNo;
char		gsLocCode[MAX_LOCCODE_LEN];

S32 		gdWEBFlag;
char	    vERSION[7] = "R4.0.0";

/** main function.
 *
 *  man Function
 *
 *  @param	argc	:	파라미터 개수
 *  @param	*argv[]	:	파라미터
 *
 *  @return			S32
 *  @see			http_main.c l4.h http_init.c http_func.c
 *
 *  @exception		.
 *  @note			.
 **/
S32 main(S32 argc, S8 *argv[])
{
	S32				dRet, dFileType;
	OFFSET			offset;
	U8				*pNode,	*pNextNode;
	U8				*p, *data;
	S32				type, len, ismalloc, dMax, dCur;
	U8				*pBodyNode;
	time_t			tTime;
//	time_t			tOldTime;
    struct timeval  stNowTime;
	FILEMNG			stFILEMNG;
	pFILEINFO		pstFILEINFO;
	LOG_TCP_SESS	*pLOG_TCP_SESS;
	LOG_HTTP_TRANS	*pLOG_HTTP_TRANS;
	LOG_INET		*pLOG_INET_SESS;
	LOG_ITCP_SESS	*pLOG_ITCP_SESS;
	LOG_IHTTP_TRANS	*pLOG_IHTTP_TRANS;
	LOG_PAGE_TRANS	*pLOG_PAGE_TRANS;
	LOG_RPPI		*pLOG_RPPI;
	LOG_RPPI_ERR	*pLOG_RPPI_ERR;
//	LOG_CALL_TRANS	*pLOG_CALL_TRANS;
	LOG_RTSP_TRANS	stLOG_RTSP_TRANS;
	LOG_VOD_SESS	*pLOG_VOD_SESS;
	LOG_VT_SESS		*pLOG_VT_SESS;
	LOG_IM_SESS		*pLOG_IM_SESS;
	LOG_SIP_TRANS	*pLOG_SIP_TRANS;
	LOG_MSRP_TRANS	*pLOG_MSRP_TRANS;
	LOG_FTP			*pLOG_FTP;
	LOG_SIGNAL		*pLOG_SIGNAL;
#ifdef ENABLE_ANALYZE_DIAMETER
	DB_LOG_DIAMETER stDB_LOG_DIAMETER;
#endif

	log_init(S_SSHM_LOG_LEVEL, getpid(), SEQ_PROC_M_LOG, LOG_PATH"/M_LOG", "M_LOG");

	/* M_LOG 초기화 */
	if((dRet = dInitMLOG(&pMEMSINFO)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInitMLOG dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	pMEMSINFO = nifo_init_zone((unsigned char *)"M_LOG", SEQ_PROC_M_LOG, FILE_NIFO_ZONE);
	if( pMEMSINFO == NULL ){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN nifo_init, NULL",  __FILE__, __FUNCTION__, __LINE__);
		exit(0);
	}

	gpCIFO = gifo_init_group(FILE_CIFO_CONF, FILE_GIFO_CONF);
	if( gpCIFO == NULL ){
		log_print(LOGN_CRI, "F=%s:%s.%d: FAILED IN gifo_init_group. cifo=%s, gifo=%s",
				__FILE__, __FUNCTION__, __LINE__, FILE_CIFO_CONF, FILE_GIFO_CONF);
		exit(0);
	}

	if((dRet = dInitFileMng(&stFILEMNG)) < 0) {
		log_print(LOGN_CRI, "[%s][%s.%d] dInitFileMng dRet[%d]", __FILE__, __FUNCTION__, __LINE__, dRet);
		exit(0);
	}

	if( (dRet = set_version(S_SSHM_VERSION, SEQ_PROC_M_LOG, vERSION)) != 0)
		log_print( LOGN_CRI, "set_version error(ret=%d,idx=%d,ver=%s)", dRet,SEQ_PROC_M_LOG,vERSION);

	log_print(LOGN_CRI, "START M_LOG, VERSION=%s", vERSION);

	/* MAIN LOOP */
	dMax = 0; dCur = 0;
	while(giStopFlag)
	{
		time(&tTime);

		tTime = (tTime / 60) * 60;

#if 0
		if(tTime != tOldTime) {
			dCur = mems_alloced_cnt(pMEMSINFO);
			if(dCur > dMax)
				dMax = dCur;
			log_print(LOGN_CRI, "NODEMAX=%d CUR=%d MAX=%d | CREATE=%llu DEL=%llu",
			pMEMSINFO->uiMemNodeTotCnt, dCur, dMax, pMEMSINFO->createCnt, pMEMSINFO->delCnt);
			tOldTime = tTime;
		}
#endif
			
		tTime = (tTime / MAKEFILE_TIMEOUT) * MAKEFILE_TIMEOUT;
		if(tTime != stFILEMNG.tTime) {
			if((dRet = dMakeFileMng(tTime, &stFILEMNG)) < 0) {
				log_print(LOGN_CRI, "Make File Mng Function Error=[%d]", dRet);
			}
		}

		if((offset = nifo_msg_read(pMEMSINFO, dMyQID, NULL)) > 0) {

			/* DB LOG 전송을 목적으로 하는 NODE (삭제 하지 않고 전송하기 위함 )*/
			pNode = nifo_ptr(pMEMSINFO, offset);
			pNextNode = pNode;

			pLOG_HTTP_TRANS = NULL;
			pBodyNode = NULL;

			do {
				p = pNextNode;

				while(p != NULL) {

					if((dRet = nifo_read_tlv_cont(pMEMSINFO, pNextNode, &type, &len, &data, &ismalloc, &p)) < 0)
						break;

					time(&tTime);
					tTime = (tTime / MAKEFILE_TIMEOUT) * MAKEFILE_TIMEOUT;
					if(tTime != stFILEMNG.tTime) {
						if((dRet = dMakeFileMng(tTime, &stFILEMNG)) < 0) {
							log_print(LOGN_CRI, "Make File Mng Function Error=[%d]", dRet);
						}
					}

					log_print(LOGN_INFO, "TYPE=%d:%s LEN=%d ISMAL=%s", 
						type, PRINT_DEF_NUM_table_log(type), len, (ismalloc == DEF_READ_MALLOC) ? "MALL" : "ORI");

					switch(type)
					{
					case LOG_INET_DEF_NUM:
						pLOG_INET_SESS = (LOG_INET *) data;
						dFileType = FTYPE_NET;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) {
							log_print(LOGN_CRI, "LOG_INET_SESS Not Support type = [%d]", type);
							break;
						}
					
						//LOG_ITCP_SESS_CILOG(pstFILEINFO->pDataFile, pLOG_ITCP_SESS);
						/* jhbaek */
						gettimeofday(&stNowTime, NULL);
						pLOG_INET_SESS->uiOpEndTime = stNowTime.tv_sec;
						pLOG_INET_SESS->uiOpEndMTime = stNowTime.tv_usec;
						CILOG_LOG_INET(pstFILEINFO->pDataFile, pLOG_INET_SESS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;

					case LOG_ITCP_SESS_DEF_NUM:
						pLOG_ITCP_SESS = (LOG_ITCP_SESS *) data;
						pLOG_ITCP_SESS->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = FTYPE_ITC;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) {
							log_print(LOGN_CRI, "LOG_ITCP_SESSS Not Support Platform = [%ld]", pLOG_ITCP_SESS->usPlatformType);
							break;
						}
					
						log_print(LOGN_INFO, "LOG_ITCP_SESS Platform:%ld - %s"
								,pLOG_ITCP_SESS->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_ITCP_SESS->usPlatformType)
								);
						//LOG_ITCP_SESS_CILOG(pstFILEINFO->pDataFile, pLOG_ITCP_SESS);
						/* jhbaek */
						gettimeofday(&stNowTime, NULL);
						pLOG_ITCP_SESS->uiOpEndTime = stNowTime.tv_sec;
						pLOG_ITCP_SESS->uiOpEndMTime = stNowTime.tv_usec;
						CILOG_LOG_ITCP_SESS(pstFILEINFO->pDataFile, pLOG_ITCP_SESS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;

					case LOG_IHTTP_TRANS_DEF_NUM:
						pLOG_IHTTP_TRANS = (LOG_IHTTP_TRANS *) data;
						pLOG_IHTTP_TRANS->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = FTYPE_IHT;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) {
							log_print(LOGN_CRI, "LOG_IHTTP_TRNAS Not Support Platform = [%ld]", pLOG_IHTTP_TRANS->usPlatformType);
							break;
						}
					
						log_print(LOGN_INFO, "LOG_IHTTP_TRANS Platform:%ld - %s"
								,pLOG_IHTTP_TRANS->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_IHTTP_TRANS->usPlatformType)
								);
						//LOG_IHTTP_TRANS_CILOG(pstFILEINFO->pDataFile, pLOG_IHTTP_TRANS);
						/* jhbaek */
						gettimeofday(&stNowTime, NULL);
						pLOG_IHTTP_TRANS->uiOpEndTime = stNowTime.tv_sec;
						pLOG_IHTTP_TRANS->uiOpEndMTime = stNowTime.tv_usec;
						CILOG_LOG_IHTTP_TRANS(pstFILEINFO->pDataFile, pLOG_IHTTP_TRANS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);

						break;
					case LOG_HTTP_TRANS_DEF_NUM:

						pLOG_HTTP_TRANS = (LOG_HTTP_TRANS *) data;
						pLOG_HTTP_TRANS->ucSubSysNo = stFILEMNG.usSystemID;
						switch(pLOG_HTTP_TRANS->ucMethod)
						{
						case METHOD_GET :
						case METHOD_POST :
						case METHOD_HEAD :
						case METHOD_PUT :
						case METHOD_DELETE :
						case METHOD_TRACE :
						case METHOD_CONNECT :
							dFileType = (isRoam(pLOG_HTTP_TRANS->ucBranchID)) ? FTYPE_MHTTP : FTYPE_HTTP;
							log_print(LOGN_INFO, "LOG_HTTP_TRANS Platform:%ld - %s" ,pLOG_HTTP_TRANS->usPlatformType, 
									PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_HTTP_TRANS->usPlatformType));
							dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
							if(dRet != 0) {
								log_print(LOGN_CRI, "MAPPING LOG_HTTP_TRANS Not Support Platform = [%ld]", 
									pLOG_HTTP_TRANS->usPlatformType);
							} else {
		                        /* jhbaek */
								gettimeofday(&stNowTime, NULL);
        		                pLOG_HTTP_TRANS->uiOpEndTime = stNowTime.tv_sec;
                		        pLOG_HTTP_TRANS->uiOpEndMTime =  stNowTime.tv_usec;
								CILOG_LOG_HTTP_TRANS(pstFILEINFO->pDataFile, pLOG_HTTP_TRANS);
								pstFILEINFO->uiRecordCnt++;
								if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
									fflush(pstFILEINFO->pDataFile);
							}
							break;
						case METHOD_DESCRIBE :
						case METHOD_SETUP :
						case METHOD_PLAY :
						case METHOD_PAUSE :
						case METHOD_OPTIONS :
						case METHOD_ANNOUNCE :
						case METHOD_GET_PARAMETER :
						case METHOD_RECORD :
						case METHOD_REDIRECT :
						case METHOD_SET_PARAMETER :
						case METHOD_TEARDOWN :
							dFileType = (isRoam(pLOG_HTTP_TRANS->ucBranchID)) ? FTYPE_MRTSP : FTYPE_RTSP;
							log_print(LOGN_INFO, "LOG_RTSP_TRANS Platform:%ld - %s" ,pLOG_HTTP_TRANS->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_HTTP_TRANS->usPlatformType));
							dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
							if(dRet != 0) {
								log_print(LOGN_CRI, "MAPPING LOG_HTTP_TRANS Not Support Platform = [%ld]", 
									pLOG_HTTP_TRANS->usPlatformType);
							} else { 
								/* RTSP TRANS All Field must be seted by LOG_RTSP_TRANS_CILOG Function, So don't need memset */
								memset(&stLOG_RTSP_TRANS, 0x00, LOG_RTSP_TRANS_SIZE);
                                /* jhbaek */
								gettimeofday(&stNowTime, NULL);
                                pLOG_HTTP_TRANS->uiOpEndTime = stNowTime.tv_sec;
                                pLOG_HTTP_TRANS->uiOpEndMTime = stNowTime.tv_usec;

								dConvertHTTPtoRTSP(pLOG_HTTP_TRANS, &stLOG_RTSP_TRANS);
								//LOG_RTSP_TRANS_CILOG(pstFILEINFO->pDataFile, &stLOG_RTSP_TRANS);
								CILOG_LOG_RTSP_TRANS(pstFILEINFO->pDataFile, &stLOG_RTSP_TRANS);
								pstFILEINFO->uiRecordCnt++;
								if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
									fflush(pstFILEINFO->pDataFile);
							}
							break;
						case METHOD_RESULT :
						default :
							log_print(LOGN_CRI, "LOG_HTTP_TRANS Not Support MethodType = [%ld]", pLOG_HTTP_TRANS->ucMethod);
							break;
						}
						break;
					case LOG_PAGE_TRANS_DEF_NUM:
						pLOG_PAGE_TRANS = (LOG_PAGE_TRANS *) data;
						pLOG_PAGE_TRANS->SubSysNo = stFILEMNG.usSystemID;

						dFileType = (isRoam(pLOG_PAGE_TRANS->ucBranchID)) ? FTYPE_MPAGE : FTYPE_PAGE;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) {
							log_print(LOGN_CRI, "LOG_PAGE_TRANS Not Support Platform = [%ld]", pLOG_PAGE_TRANS->LastPlatformType);
							break;
						}
					
						log_print(LOGN_INFO, "LOG_PAGE_TRANS Platform:%ld - %s"
								,pLOG_PAGE_TRANS->LastPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_PAGE_TRANS->LastPlatformType)
								);
						//LOG_PAGE_TRANS_CILOG(pstFILEINFO->pDataFile, pLOG_PAGE_TRANS);
                        /* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_PAGE_TRANS->OpEndTime = stNowTime.tv_sec;
                        pLOG_PAGE_TRANS->OpEndMTime = stNowTime.tv_usec;
						CILOG_LOG_PAGE_TRANS(pstFILEINFO->pDataFile, pLOG_PAGE_TRANS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;
					case LOG_TCP_SESS_DEF_NUM:
						pLOG_TCP_SESS = (LOG_TCP_SESS *) data;
						pLOG_TCP_SESS->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = (isRoam(pLOG_TCP_SESS->ucBranchID)) ? FTYPE_MTCP : FTYPE_TCP;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) {
							log_print(LOGN_CRI, "LOG_TCP_SESSS Not Support Platform = [%ld]", pLOG_TCP_SESS->usPlatformType);
							break;
						}
					
						log_print(LOGN_INFO, "LOG_TCP_SESS Platform:%ld - %s"
								,pLOG_TCP_SESS->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_TCP_SESS->usPlatformType)
								);
						//LOG_TCP_SESS_CILOG(pstFILEINFO->pDataFile, pLOG_TCP_SESS);
		                /* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_TCP_SESS->uiOpEndTime = stNowTime.tv_sec;
                        pLOG_TCP_SESS->uiOpEndMTime = stNowTime.tv_usec;
						CILOG_LOG_TCP_SESS(pstFILEINFO->pDataFile, pLOG_TCP_SESS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;
					case LOG_RPPI_DEF_NUM:
						pLOG_RPPI = (LOG_RPPI *) data;
//						pLOG_RPPI->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = (isRoam(pLOG_RPPI->ucBranchID)) ? FTYPE_MRPPI : FTYPE_RPPI;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
//							log_print(LOGN_CRI, "LOG_RPPI Not Support Platform = [%ld]", pLOG_RPPI->usPlatformType);
							log_print(LOGN_CRI, "LOG_RPPI Not Support");
							break;  
						}       
/*
						log_print(LOGN_INFO, "LOG_RPPI Platform:%ld - %s"
								,pLOG_RPPI->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_RPPI->usPlatformType)
								);      
*/
						log_print(LOGN_INFO, "LOG_RPPI");
                        /* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_RPPI->uiOpEndTime = stNowTime.tv_sec;
                        pLOG_RPPI->uiOpEndMTime = stNowTime.tv_usec;						
						CILOG_LOG_RPPI(pstFILEINFO->pDataFile, pLOG_RPPI);

						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
#ifdef ENABLE_WEB_LOG 
						/* RPPI 로그중 SvcType이 15100 번인 로그는 외부인터넷 로그로 저장한다. 
						 * BY YOON 20110207
						 */
						if( gdWEBFlag == DEF_WEBLOG_ON && 
							(pLOG_RPPI->uiFirstSvcL4Type == L4_PHONE_ETC || pLOG_RPPI->uiLastSvcL4Type == L4_PHONE_ETC)) {
							dFileType = FTYPE_WEB;
							dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
							if(dRet != 0) { 
								log_print(LOGN_CRI, "LOG_WEB Not Support");
								break;  
							}
	                        /* jhbaek */
							gettimeofday(&stNowTime, NULL);
    	                    pLOG_RPPI->uiOpEndTime = stNowTime.tv_sec;
        	                pLOG_RPPI->uiOpEndMTime = stNowTime.tv_usec;        
							CILOG_LOG_WEB_P1(pstFILEINFO->pDataFile, pLOG_RPPI);
						}
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
#endif
						break;  
					case LOG_RPPI_ERR_DEF_NUM:
						pLOG_RPPI_ERR = (LOG_RPPI_ERR *) data;
						dFileType = (isRoam(pLOG_RPPI_ERR->ucBranchID)) ? FTYPE_MERPPI : FTYPE_ERPPI;
//						pLOG_RPPI_ERR->ucSubSysNo = stFILEMNG.usSystemID;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
//							log_print(LOGN_CRI, "LOG_RPPI_ERR Not Support Platform = [%ld]", pLOG_RPPI_ERR->usPlatformType);
							log_print(LOGN_CRI, "LOG_RPPI_ERR Not Support");
							break;  
						}       
        
/*
						log_print(LOGN_INFO, "LOG_RPPI_ERR Platform:%ld - %s"
								,pLOG_RPPI_ERR->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_RPPI_ERR->usPlatformType)
								);      
*/
						log_print(LOGN_INFO, "LOG_RPPI_ERR");
						/* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_RPPI_ERR->uiOpEndTime = stNowTime.tv_sec;
                        pLOG_RPPI_ERR->uiOpEndMTime = stNowTime.tv_usec; 
						CILOG_LOG_RPPI_ERR(pstFILEINFO->pDataFile, pLOG_RPPI_ERR);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;  
					case LOG_VT_SESS_DEF_NUM:
						pLOG_VT_SESS = (LOG_VT_SESS *) data;
						pLOG_VT_SESS->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = (isRoam(pLOG_VT_SESS->ucBranchID)) ? FTYPE_MVT : FTYPE_VT;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
							log_print(LOGN_CRI, "LOG_VT_SESS Not Support Platform = [%ld]", pLOG_VT_SESS->usPlatformType);
							break;  
						}       
        
						log_print(LOGN_INFO, "LOG_VT_SESS Platform:%ld - %s"
								,pLOG_VT_SESS->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_VT_SESS->usPlatformType)
								);      
                        /* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_VT_SESS->OpEndTime = stNowTime.tv_sec;
                        pLOG_VT_SESS->OpEndMTime = stNowTime.tv_usec;
						CILOG_LOG_VT_SESS(pstFILEINFO->pDataFile, pLOG_VT_SESS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;  
					case LOG_IM_SESS_DEF_NUM:
						pLOG_IM_SESS = (LOG_IM_SESS *) data;
						pLOG_IM_SESS->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = (isRoam(pLOG_IM_SESS->ucBranchID)) ? FTYPE_MIM : FTYPE_IM;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
							log_print(LOGN_CRI, "LOG_IM Not Support Platform = [%ld]", pLOG_IM_SESS->usPlatformType);
							break;  
						}       
        
						log_print(LOGN_INFO, "LOG_IM Platform:%ld - %s"
								,pLOG_IM_SESS->usPlatformType ,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_IM_SESS->usPlatformType));      
                        /* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_IM_SESS->OpEndTime = stNowTime.tv_sec;
                        pLOG_IM_SESS->OpEndMTime = stNowTime.tv_usec;
						CILOG_LOG_IM_SESS(pstFILEINFO->pDataFile, pLOG_IM_SESS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;
					case LOG_VOD_SESS_DEF_NUM:
						pLOG_VOD_SESS = (LOG_VOD_SESS *) data;
						pLOG_VOD_SESS->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = (isRoam(pLOG_VOD_SESS->ucBranchID)) ? FTYPE_MVOD : FTYPE_VOD;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
							log_print(LOGN_CRI, "LOG_VOD_TRANS Not Support Platform = [%ld]", pLOG_VOD_SESS->usPlatformType);
							break;  
						}       
        
						log_print(LOGN_INFO, "LOG_VOD_SESS Platform:%ld - %s"
								,pLOG_VOD_SESS->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_VOD_SESS->usPlatformType)
								);
                        /* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_VOD_SESS->uiOpEndTime = stNowTime.tv_sec;
                        pLOG_VOD_SESS->uiOpEndMTime = stNowTime.tv_usec;      
						CILOG_LOG_VOD_SESS(pstFILEINFO->pDataFile, pLOG_VOD_SESS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;  
					case LOG_SIP_TRANS_DEF_NUM:
						pLOG_SIP_TRANS = (LOG_SIP_TRANS *) data;
						pLOG_SIP_TRANS->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = (isRoam(pLOG_SIP_TRANS->ucBranchID)) ? FTYPE_MSIP : FTYPE_SIP;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
							log_print(LOGN_CRI, "LOG_SIP_TRANS Not Support Platform = [%ld]", pLOG_SIP_TRANS->usPlatformType);
							break;  
						}       
        
						log_print(LOGN_INFO, "LOG_SIP_TRANS Platform:%ld - %s"
								,pLOG_SIP_TRANS->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_SIP_TRANS->usPlatformType)
								);
	                    /* jhbaek */
						gettimeofday(&stNowTime, NULL);
	                    pLOG_SIP_TRANS->OpEndTime = stNowTime.tv_sec;
                        pLOG_SIP_TRANS->OpEndMTime = stNowTime.tv_usec;      
						CILOG_LOG_SIP_TRANS(pstFILEINFO->pDataFile, pLOG_SIP_TRANS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;  
					case LOG_MSRP_TRANS_DEF_NUM:
						pLOG_MSRP_TRANS = (LOG_MSRP_TRANS *) data;
						pLOG_MSRP_TRANS->ucSubSysNo = stFILEMNG.usSystemID;
						dFileType = (isRoam(pLOG_MSRP_TRANS->ucBranchID)) ? FTYPE_MMSRP : FTYPE_MSRP;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
							log_print(LOGN_CRI, "LOG_MSRP_TRANS Not Support Platform = [%ld]", pLOG_MSRP_TRANS->usPlatformType);
							break;  
						}       
        
						log_print(LOGN_INFO, "LOG_MSRP_TRANS Platform:%ld - %s"
								,pLOG_MSRP_TRANS->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_MSRP_TRANS->usPlatformType)
								);
                        /* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_MSRP_TRANS->OpEndTime = stNowTime.tv_sec;
                        pLOG_MSRP_TRANS->OpEndMTime = stNowTime.tv_usec;      
						CILOG_LOG_MSRP_TRANS(pstFILEINFO->pDataFile, pLOG_MSRP_TRANS);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;  
					case LOG_FTP_DEF_NUM:
						pLOG_FTP = (LOG_FTP *) data;
//						pLOG_FTP->ucSubSysNo = stFILEMNG.usSystemID;
						if(isRoam(pLOG_FTP->ucBranchID)) {
							log_print(LOGN_CRI, "LOG_FTP Not Support BRANCHID = [%d]", pLOG_FTP->ucBranchID);
							break;  
						}
						dFileType = FTYPE_FTP;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
							log_print(LOGN_CRI, "LOG_FTP Not Support Platform = [%ld]", pLOG_FTP->usPlatformType);
							break;  
						}       
        
						log_print(LOGN_INFO, "LOG_FTP Platform:%ld - %s"
								,pLOG_FTP->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_FTP->usPlatformType)
								);
                        /* jhbaek */
						gettimeofday(&stNowTime, NULL);
                        pLOG_FTP->OpEndTime = stNowTime.tv_sec;
                        pLOG_FTP->OpEndMTime = stNowTime.tv_usec;      
						CILOG_LOG_FTP(pstFILEINFO->pDataFile, pLOG_FTP);
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;  
					case LOG_SIGNAL_DEF_NUM:
						pLOG_SIGNAL = (LOG_SIGNAL *) data;
						dFileType = (isRoam(pLOG_SIGNAL->ucBranchID)) ? FTYPE_MSIG : FTYPE_DIA;
						dRet = dMapFileInfo(dFileType, &pstFILEINFO, &stFILEMNG);
						if(dRet != 0) { 
//							log_print(LOGN_CRI, "LOG_SIGNAL Not Support Platform = [%ld]", pLOG_SIGNAL->usPlatformType);
							log_print(LOGN_CRI, "LOG_SIGNAL Not Support");
							break;  
						}       
        
/*
						log_print(LOGN_INFO, "LOG_SIGNAL Platform:%ld - %s"
								,pLOG_SIGNAL->usPlatformType
								,PRINT_TAG_DEF_ALL_PLATFORMTYPE(pLOG_SIGNAL->usPlatformType)
								);      
*/
#ifdef ENABLE_ANALYZE_DIAMETER
						if( dFileType != FTYPE_MSIG ){
							if( pLOG_SIGNAL->uiProtoType != DIAMETER_PROTO ){
								log_print(LOGN_INFO,"PROTOTYPE(%d) IS NOT DIAMETER_PROTO=%d",pLOG_SIGNAL->uiProtoType, DIAMETER_PROTO);
								break;
							}
							log_print(LOGN_INFO, "LOG_SIGNAL::DB_LOG_DIAMETER");
							memset(&stDB_LOG_DIAMETER, 0x00, DB_LOG_DIAMETER_SIZE);
							
							dConvertSIGNALtoDIAMETER(pLOG_SIGNAL, &stDB_LOG_DIAMETER);
							CILOG_DB_LOG_DIAMETER(pstFILEINFO->pDataFile, &stDB_LOG_DIAMETER);
						}else{
							log_print(LOGN_INFO, "LOG_SIGNAL::ROAMING");
							CILOG_LOG_SIGNAL(pstFILEINFO->pDataFile, pLOG_SIGNAL);
						}
#else
						log_print(LOGN_INFO, "LOG_SIGNAL::ROAMING");
						CILOG_LOG_SIGNAL(pstFILEINFO->pDataFile, pLOG_SIGNAL);
#endif
						pstFILEINFO->uiRecordCnt++;
						if(pstFILEINFO->uiRecordCnt % FILEFLUSH_CNT == 0)
							fflush(pstFILEINFO->pDataFile);
						break;  
					case LOG_CALL_TRANS_DEF_NUM:
					case TCP_INFO_DEF_NUM:
					case HTTP_REQ_HDR_NUM:
					case HTTP_REQ_BODY_NUM:
					case HTTP_RES_HDR_NUM:
					case HTTP_RES_BODY_NUM:
					default:
						log_print(LOGN_CRI, ">>>>>>> Not Support LOG Type=[%d]", type);
						break;
					}

					if(ismalloc == DEF_READ_MALLOC){ free(data); }
				}
				
				pNextNode = (U8 *)nifo_entry(nifo_ptr(pMEMSINFO, ((NIFO *)pNextNode)->nont.offset_next), NIFO, nont);

			} while(pNode != pNextNode);

			nifo_node_delete(pMEMSINFO, pNode);

		} else {
			usleep(0);
		}

	}

	FinishProgram(&stFILEMNG);

	return 0;
}

int dInitFileMng(pFILEMNG pstFILEMNG)
{
	int			i, j, dRet, dUseFlag, dFlag = 0;
	U8			szSeq[MAX_SEQ_SIZE];
	U8			*sp, *p;

	pFILEINFO	pstFILEINFO;
#ifdef ENABLE_ANALYZE_DIAMETER
	U8			szFileType[MAX_FILETYPE_CNT][MAX_FNAME_SIZE] = {
						"TCP", "HTP", "PAG", "RPP", "RTS", "VOD", "VTS", "IMS", "SIP", "MSR", "ERP", "FTP",
						"TCP", "HTP", "PAG", "RPP", "RTS", "VOD", "VTS", "IMS", "SIP", "MSR", "ERP", "SIG", "DIA", "WEB",
						"NET", "ITC", "IHT" };
#else
	U8			szFileType[MAX_FILETYPE_CNT][MAX_FNAME_SIZE] = {
						"TCP", "HTP", "PAG", "RPP", "RTS", "VOD", "VTS", "IMS", "SIP", "MSR", "ERP", "FTP",
						"TCP", "HTP", "PAG", "RPP", "RTS", "VOD", "VTS", "IMS", "SIP", "MSR", "ERP", "SIG", "WEB" };
#endif /* ENABLE_ANALYZE_DIAMETER */

	/* Just Onece */
	memset(pstFILEMNG, 0x00, MAX_FILEMNG_SIZE);

	/* changed by uamyd.20100916 */
	if( (dRet = dGetSYSCFG()) < 0 ){
		log_print(LOGN_CRI,"dGetSYSCFG()=%d", dRet);
		return -1;
	} 
	pstFILEMNG->usSystemID = gdSysNo;

	for(i = 0; i < MAX_FILETYPE_CNT; i++)
	{
		switch(i)
		{
		case FTYPE_TCP: case FTYPE_HTTP: case FTYPE_PAGE: case FTYPE_RPPI: case FTYPE_RTSP:  case FTYPE_VOD:
		case FTYPE_VT:  case FTYPE_IM:   case FTYPE_SIP:  case FTYPE_MSRP: case FTYPE_ERPPI: case FTYPE_FTP: 
		case FTYPE_NET: case FTYPE_ITC:  case FTYPE_IHT:
#ifdef ENABLE_ANALYZE_DIAMETER
		case FTYPE_DIA:
#endif
			dUseFlag = 1;
			dFlag = 0;
			break;
		case FTYPE_WEB:
			dUseFlag = gdWEBFlag;		/* dgWEBFlag is ON or OFF */
			dFlag = 0;
			break;
		case FTYPE_MTCP: case FTYPE_MHTTP: case FTYPE_MPAGE: case FTYPE_MRPPI: case FTYPE_MRTSP:  case FTYPE_MVOD:
		case FTYPE_MVT:  case FTYPE_MIM:   case FTYPE_MSIP:  case FTYPE_MMSRP: case FTYPE_MERPPI: case FTYPE_MSIG:
			dUseFlag = 1;
			dFlag = 1;
			break;
		default:
			dUseFlag = 0;
			dFlag = 0;
			break;
		}

		pstFILEINFO = &pstFILEMNG->pstFILEINFO[i];
		pstFILEINFO->uiUseFlag = dUseFlag;
		memcpy(pstFILEINFO->szFileType, szFileType[i], MAX_FTYPE_LEN);
		pstFILEINFO->szFileType[MAX_FTYPE_LEN] = 0x00;

/* changed by uamyd 2010.09.16, added SystemName & SystemNo, 일단 SystemNo 만 추가. */
		sprintf(pstFILEINFO->szDataFile, "%s%s_F%.*s_ID0000_TYYYYMMDDhhmmss.DAT", 
			CILOG_PATH, (dFlag == 1) ? "DRMS" : gsLocCode, 
			MAX_FTYPE_LEN, pstFILEINFO->szFileType);
		sprintf(pstFILEINFO->szFinFile, "%s%s_F%.*s_ID0000_TYYYYMMDDhhmmss.FIN", 
			CILOG_PATH, (dFlag == 1) ? "DRMS" : gsLocCode, 
			MAX_FTYPE_LEN, pstFILEINFO->szFileType);

		log_print(LOGN_WARN, "INIT FILEINFO[%d] : USE=[%d] SEQ=%d Rec=%d Type=[%s]",
			i, pstFILEINFO->uiUseFlag, pstFILEMNG->usSeq, pstFILEINFO->uiRecordCnt, 
			pstFILEINFO->szFileType);
		log_print(LOGN_WARN, "INIT DATAFILE=[%s] FINFILE=[%s]",
			pstFILEINFO->szDataFile, pstFILEINFO->szFinFile);
	}

	time(&pstFILEMNG->tTime);
	pstFILEMNG->tTime = (pstFILEMNG->tTime / MAKEFILE_TIMEOUT) * MAKEFILE_TIMEOUT;
	localtime_r(&pstFILEMNG->tTime, &pstFILEMNG->stTime);
	sprintf(pstFILEMNG->szDateTime, "%04d%02d%02d%02d%02d%02d", 
		pstFILEMNG->stTime.tm_year + 1900, pstFILEMNG->stTime.tm_mon + 1, pstFILEMNG->stTime.tm_mday,
	    pstFILEMNG->stTime.tm_hour, pstFILEMNG->stTime.tm_min, pstFILEMNG->stTime.tm_sec);
	pstFILEMNG->usSeq++;
	sprintf(szSeq, "%04d", pstFILEMNG->usSeq);

	for(i = 0; i < MAX_FILETYPE_CNT; i++)
	{
		pstFILEINFO = &pstFILEMNG->pstFILEINFO[i];
		if(pstFILEINFO->uiUseFlag == 1) {

			if(pstFILEINFO->pDataFile != NULL) {
				log_print(LOGN_WARN, "Data File is NOT NULL Check Flow in Process First Start File = [%s]",
					pstFILEINFO->szDataFile);
			}
		}

		pstFILEINFO->uiRecordCnt = 0;

		/* 
		   TAM-DB 연동 SAMPLE
		   70DQM01_FHTP_ID0002_T20090216223500.DAT
		   70DQM02_FSIP_ID0003_T20090216123500.DAT
		   DRMS_FSIP_ID0001_T20090216123500.DAT
		   DRMS_FHTP_ID0002_T20090216123500.DAT

	    */

		for( j=0; j<2; j++ ){
			if( j == 0 )
				sp = pstFILEINFO->szDataFile+CILOG_PATH_SIZE; 
			else
				sp = pstFILEINFO->szFinFile+CILOG_PATH_SIZE;

			/* set seq */
			p = strstr(sp, "_ID");
			/* p is Not NULL */
			if( p == NULL ){
				log_print(LOGN_CRI,"%s:%d]There is NOT ID indicator : '_ID'", __FUNCTION__,__LINE__);
				return -2;
			}
			memcpy( p+3, szSeq, MAX_SEQ_LEN );

			/* set time */
			p = strstr(sp, "_T");
			if( p == NULL ){
				log_print(LOGN_CRI,"%s:%d]There is NOT FileType indicator : '_T'", __FUNCTION__,__LINE__);
				return -3;
			}
			memcpy( p+2, pstFILEMNG->szDateTime, MAX_DATETIME_LEN );
		}

#if 0
		memcpy(pstFILEINFO->szDataFile+POS_SEQ, szSeq, MAX_SEQ_LEN); 
		memcpy(pstFILEINFO->szDataFile+POS_TIME, pstFILEMNG->szDateTime, MAX_DATETIME_LEN);
		memcpy(pstFILEINFO->szFinFile+POS_SEQ, szSeq, MAX_SEQ_LEN); 
		memcpy(pstFILEINFO->szFinFile+POS_TIME, pstFILEMNG->szDateTime, MAX_DATETIME_LEN);
#endif

		if(pstFILEINFO->uiUseFlag == 1) {
			/* Append Mode Coding later */
			umask(000);
			pstFILEINFO->pDataFile = fopen(pstFILEINFO->szDataFile, "w");
			if(pstFILEINFO->pDataFile == NULL) {
				log_print(LOGN_CRI, "Make Data File Open Error FileName=[%s] err=%d[%s]", 
					pstFILEINFO->szDataFile, errno, strerror(errno));
			} else {
				log_print(LOGN_DEBUG, "Make Data File Rec=[%d] DataFile=[%p][%s][%s]", 
					pstFILEINFO->uiRecordCnt, pstFILEINFO->pDataFile, 
					pstFILEINFO->szDataFile, pstFILEINFO->szFinFile);
			}
		}
		log_print(LOGN_INFO,"%d][DATAFILE=%s",i, pstFILEINFO->szDataFile);
		log_print(LOGN_INFO,"%d][FINFILE =%s",i, pstFILEINFO->szFinFile);
	} /* End of for J */

	return 0;
}

int dMakeFileMng(time_t tSetTime, pFILEMNG pstFILEMNG)
{
	int			i, j, dRet, dLen, dDay;
	U8			szSeq[MAX_SEQ_SIZE];
	U8			szFileName[MAX_FNAME_SIZE];
	U8			*sp, *p;

	FILE		*pFinFile;
	pFILEINFO	pstFILEINFO;

	int 		dSNLOG_PATH_SIZE = SNLOG_PATH_SIZE;

	pstFILEMNG->tTime = tSetTime;
	dDay = pstFILEMNG->stTime.tm_mday;
	localtime_r(&pstFILEMNG->tTime, &pstFILEMNG->stTime);
	if(dDay != pstFILEMNG->stTime.tm_mday)
		pstFILEMNG->usSeq = 0;
	sprintf(pstFILEMNG->szDateTime, "%04d%02d%02d%02d%02d%02d", 
		pstFILEMNG->stTime.tm_year + 1900, pstFILEMNG->stTime.tm_mon + 1, pstFILEMNG->stTime.tm_mday,
	    pstFILEMNG->stTime.tm_hour, pstFILEMNG->stTime.tm_min, pstFILEMNG->stTime.tm_sec);
	pstFILEMNG->usSeq++;
	sprintf(szSeq, "%04d", pstFILEMNG->usSeq);
#if 0
	memcpy(szFileName, SNLOG_PATH, SNLOG_PATH_SIZE);
	szFileName[SNLOG_PATH_SIZE] = 0x00;
#endif
	for(i = 0; i < MAX_FILETYPE_CNT; i++)
	{
		if(i==FTYPE_WEB) {
			memcpy(szFileName, WEBLOG_PATH, WEBLOG_PATH_SIZE);
			szFileName[WEBLOG_PATH_SIZE] = 0x00;
			dSNLOG_PATH_SIZE = WEBLOG_PATH_SIZE;
		} else {
			memcpy(szFileName, SNLOG_PATH, SNLOG_PATH_SIZE);
			szFileName[SNLOG_PATH_SIZE] = 0x00;
			dSNLOG_PATH_SIZE = SNLOG_PATH_SIZE;
		}	
		pstFILEINFO = &pstFILEMNG->pstFILEINFO[i];
		if(pstFILEINFO->uiUseFlag == 1) {

			if(pstFILEINFO->pDataFile != NULL)
				fclose(pstFILEINFO->pDataFile);
			else {
				log_print(LOGN_WARN, "Data File is NULL Check Flow except Process First Start File = [%s]",
					pstFILEINFO->szDataFile);
			}

			/* File Close && Move Send Dir */
//			if(pstFILEINFO->uiRecordCnt != 0) {
				umask(000);
				pFinFile = fopen(pstFILEINFO->szFinFile,"w");
				if(pFinFile == NULL) {
					log_print(LOGN_CRI, "Make Fin File Open Error FileName=[%s] err=%d[%s]", 
						pstFILEINFO->szFinFile, errno, strerror(errno));
				} else {
					fprintf(pFinFile, "F\t%.*s\t%d\n", 
						FIN_FILENAME_SIZE, pstFILEINFO->szDataFile+CILOG_PATH_SIZE, pstFILEINFO->uiRecordCnt);
					fclose(pFinFile);

					log_print(LOGN_DEBUG, "Make Fin Rec=[%d] Data=[%s] File=[%s]", 
						pstFILEINFO->uiRecordCnt, pstFILEINFO->szDataFile, pstFILEINFO->szFinFile);

					/* Move Directory CILOG -> SILOG */
					dLen = strlen(pstFILEINFO->szDataFile);
					memcpy(szFileName + dSNLOG_PATH_SIZE, pstFILEINFO->szDataFile + CILOG_PATH_SIZE,   
							dLen - CILOG_PATH_SIZE);
					szFileName[dSNLOG_PATH_SIZE+dLen-CILOG_PATH_SIZE] = 0x00;
					dRet = rename(pstFILEINFO->szDataFile, szFileName);
					if(dRet < 0) {
						log_print(LOGN_CRI, "DATA FILE MOVE Error[%d][%s] from [%s] to [%s]", 
							errno, strerror(errno), pstFILEINFO->szDataFile, szFileName);
					} else {
						memcpy(szFileName + dSNLOG_PATH_SIZE, pstFILEINFO->szDataFile + CILOG_PATH_SIZE,   
							dLen - CILOG_PATH_SIZE - 3);
						dLen = dSNLOG_PATH_SIZE + dLen - CILOG_PATH_SIZE - 3;
						memcpy(szFileName + dLen, "FIN", 3);
						szFileName[dLen+3] = 0x00;
						dRet = rename(pstFILEINFO->szFinFile, szFileName);
						if(dRet < 0) {
							log_print(LOGN_CRI, "FIN FILE MOVE Error[%d][%s] from [%s] to [%s]", 
								errno, strerror(errno), pstFILEINFO->szDataFile, szFileName);
						}
						/* 외부인터넷 로그는 TAM_DB와 연동하지 않는다. 
						 * BY YOON 20110207 
						 */
						if(i!=FTYPE_WEB) {
							dRet = dSendFileName(pstFILEINFO);
							if(dRet < 0) {
								log_print(LOGN_CRI, "SEND FILENAME dRet=%d NAME=%s", dRet, pstFILEINFO->szDataFile);
							}
						} 
					}
				}
//			} else {
//				log_print(LOGN_CRI, "File Record Count = %d So do not make Fin File = [%s]",
//					pstFILEINFO->uiRecordCnt, pstFILEINFO->szFinFile);
//				dRet = unlink(pstFILEINFO->szDataFile);
//				if(dRet < 0) {
//					log_print(LOGN_CRI, "REC=%d But Data File Remove Err[%d][%s] F=[%s]",
//						pstFILEINFO->uiRecordCnt, errno, strerror(errno), pstFILEINFO->szDataFile);
//				}
//			}
		}

		pstFILEINFO->uiRecordCnt = 0;

		for( j=0; j<2; j++ ){
			if( j == 0 )
				sp = pstFILEINFO->szDataFile+CILOG_PATH_SIZE;
			else
				sp = pstFILEINFO->szFinFile+CILOG_PATH_SIZE;

			p = strstr( sp, "_ID" );
			/* p is Not NULL */
			if( p == NULL ){
				log_print(LOGN_CRI,"%s:%d]There is NOT ID indicator : '_ID'", __FUNCTION__,__LINE__);
				return -1;
			}
			memcpy( p+3, szSeq, MAX_SEQ_LEN );

			p = strstr( sp, "_T" );
			/* p is Not NULL */
			if( p == NULL ){
				log_print(LOGN_CRI,"%s:%d]There is NOT ID indicator : '_T'", __FUNCTION__,__LINE__);
				return -2;
			}
			memcpy( p+2, pstFILEMNG->szDateTime, MAX_DATETIME_LEN );

		}
#if 0
		memcpy(pstFILEINFO->szDataFile+POS_SEQ, szSeq, MAX_SEQ_LEN); 
		memcpy(pstFILEINFO->szDataFile+POS_TIME, pstFILEMNG->szDateTime, MAX_DATETIME_LEN);
		memcpy(pstFILEINFO->szFinFile+POS_SEQ, szSeq, MAX_SEQ_LEN); 
		memcpy(pstFILEINFO->szFinFile+POS_TIME, pstFILEMNG->szDateTime, MAX_DATETIME_LEN);
#endif

		if(pstFILEINFO->uiUseFlag == 1) {
			/* Append Mode Coding later */
			umask(000);
			pstFILEINFO->pDataFile = fopen(pstFILEINFO->szDataFile, "w");
			if(pstFILEINFO->pDataFile == NULL) {
				log_print(LOGN_CRI, "Make Data File Open Error FileName=[%s] err=%d[%s]", 
					pstFILEINFO->szDataFile, errno, strerror(errno));
			} else {
				log_print(LOGN_DEBUG, "Make Data File Rec=[%d] DataFile=[%p][%s]", 
					pstFILEINFO->uiRecordCnt, pstFILEINFO->pDataFile, pstFILEINFO->szDataFile);
			}
		}
	} /* End of for J */

	return 0;
}

int dCloseFileMng(pFILEMNG pstFILEMNG)
{
	int			i, dRet, dLen;
	U8			szFileName[MAX_FNAME_SIZE];

	FILE		*pFinFile;
	pFILEINFO	pstFILEINFO;

	log_print(LOGN_CRI, "############>>>> FILEMNG CLOSE FILE SEQ[%d] LASTDATE[%s] >>>>##############", 
		pstFILEMNG->usSeq, pstFILEMNG->szDateTime);
	memcpy(szFileName, SNLOG_PATH, SNLOG_PATH_SIZE);
	szFileName[SNLOG_PATH_SIZE] = 0x00;
	for(i = 0; i < MAX_FILETYPE_CNT; i++)
	{
		pstFILEINFO = &pstFILEMNG->pstFILEINFO[i];
		if(pstFILEINFO->uiUseFlag == 1) {

			if(pstFILEINFO->pDataFile != NULL)
				fclose(pstFILEINFO->pDataFile);
			else {
				log_print(LOGN_WARN, "Data File is NULL Check Flow except Process First Start File = [%s]",
					pstFILEINFO->szDataFile);
			}

			/* File Close && Move Send Dir */
			if(pstFILEINFO->uiRecordCnt != 0) {
				umask(000);
				pFinFile = fopen(pstFILEINFO->szFinFile,"w");
				if(pFinFile == NULL) {
					log_print(LOGN_CRI, "Make Fin File Open Error FileName=[%s] err=%d[%s]", 
						pstFILEINFO->szFinFile, errno, strerror(errno));
				} else {
					fprintf(pFinFile, "F\t%.*s\t%d\n", 
						FIN_FILENAME_SIZE, pstFILEINFO->szDataFile+CILOG_PATH_SIZE, pstFILEINFO->uiRecordCnt);
					fclose(pFinFile);

					log_print(LOGN_DEBUG, "Make Fin Rec=[%d] Data=[%s] File=[%s]", 
						pstFILEINFO->uiRecordCnt, pstFILEINFO->szDataFile, pstFILEINFO->szFinFile);

					/* Move Directory CILOG -> SILOG */
					dLen = strlen(pstFILEINFO->szDataFile);
					memcpy(szFileName + SNLOG_PATH_SIZE, pstFILEINFO->szDataFile + CILOG_PATH_SIZE,   
						dLen - CILOG_PATH_SIZE);
					szFileName[SNLOG_PATH_SIZE+dLen-CILOG_PATH_SIZE] = 0x00;
					dRet = rename(pstFILEINFO->szDataFile, szFileName);
					if(dRet < 0) {
						log_print(LOGN_CRI, "DATA FILE MOVE Error[%d][%s] from [%s] to [%s]", 
							errno, strerror(errno), pstFILEINFO->szDataFile, szFileName);
					} else {
						memcpy(szFileName + SNLOG_PATH_SIZE, pstFILEINFO->szDataFile + CILOG_PATH_SIZE,   
							dLen - CILOG_PATH_SIZE - 3);
						dLen = SNLOG_PATH_SIZE + dLen - CILOG_PATH_SIZE - 3;
						memcpy(szFileName + dLen, "FIN", 3);
						szFileName[dLen+3] = 0x00;
						dRet = rename(pstFILEINFO->szFinFile, szFileName);
						if(dRet < 0) {
							log_print(LOGN_CRI, "FIN FILE MOVE Error[%d][%s] from [%s] to [%s]", 
								errno, strerror(errno), pstFILEINFO->szDataFile, szFileName);
						}

						dRet = dSendFileName(pstFILEINFO);
						if(dRet < 0) {
							log_print(LOGN_CRI, "SEND FILENAME dRet=%d NAME=%s", dRet, pstFILEINFO->szDataFile);
						}
					}
				}
			} else {
				log_print(LOGN_CRI, "File Record Count = %d So do not make Fin File = [%s]",
					pstFILEINFO->uiRecordCnt, pstFILEINFO->szFinFile);
				dRet = unlink(pstFILEINFO->szDataFile);
				if(dRet < 0) {
					log_print(LOGN_CRI, "REC=%d But Data File Remove Err[%d][%s] F=[%s]",
						pstFILEINFO->uiRecordCnt, errno, strerror(errno), pstFILEINFO->szDataFile);
				}
			}
		}
		pstFILEINFO->uiRecordCnt = 0;
	} /* End of for J */

	return 0;
}

int dMapFileInfo(U32 dFileType, pFILEINFO *ppstFILEINFO, pFILEMNG pstFILEMNG)
{
	pFILEINFO	pstFILEINFO;

	pstFILEINFO = &pstFILEMNG->pstFILEINFO[dFileType];

	if(pstFILEINFO->uiUseFlag == 0 || pstFILEINFO->pDataFile == NULL) {

		pstFILEINFO->uiUseFlag = 1;
		umask(000);
		pstFILEINFO->pDataFile = fopen(pstFILEINFO->szDataFile, "w");
		if(pstFILEINFO->pDataFile == NULL) {
			log_print(LOGN_CRI, "Make Data File Open Error FileName=[%s] err=%u[%s]", 
			pstFILEINFO->szDataFile, errno, strerror(errno));
			return -1;
		} else {
			log_print(LOGN_CRI, "Make Data File Rec=[%u] DataFile=[%s]", 
				pstFILEINFO->uiRecordCnt, pstFILEINFO->szDataFile);
		}
	}

	*ppstFILEINFO = pstFILEINFO;

	return 0;
}


//struct dirent
//{
//	long d_ino;                 /* 아이노드 수 */
//	off_t d_off;                /* dirent 의 오프셋 */
//	unsigned short d_reclen;    /* d_name 의 길이 */
//	char d_name [NAME_MAX+1];   /* 파일 이름(널로 종료) */
//}

//SKAS9_FUSRCAL_ID0001_T20061101021000.DAT
int dProRemainDataFile()
{
	DIR				*pDIR;
	FILE			*pFILE;
	struct dirent	*pEntry;
	S32				dRecCnt, dRet;
	U16				usDataFileLen, usFinFileLen;
	U8				szDataFile[MAX_FNAME_SIZE];
	U8				szFinFile[MAX_FNAME_SIZE];
	U8				szData[1024*20];

	pDIR = opendir(CILOG_PATH);
	if(pDIR == NULL) {
		log_print(LOGN_CRI, "Dir Open Error DIR=[%s] err=%d[%s]", 
			CILOG_PATH, errno, strerror(errno));
		return -1;
	}

	memcpy(szDataFile, CILOG_PATH, CILOG_PATH_SIZE);
	szDataFile[CILOG_PATH_SIZE] = 0x00;
	while((pEntry = readdir(pDIR)) != NULL)
	{
		usDataFileLen = strlen(pEntry->d_name);
		if(usDataFileLen != REAL_FILE_LEN) {
			log_print(LOGN_WARN, "DIR FILENAME[%s] Discard beacause of len[%d] != Def [%d]",
				pEntry->d_name, usDataFileLen, REAL_FILE_LEN);
			continue;
		}

		memcpy(szDataFile + CILOG_PATH_SIZE, pEntry->d_name, usDataFileLen);	
		usDataFileLen += CILOG_PATH_SIZE;
		szDataFile[usDataFileLen] = 0x00;
		log_print(LOGN_INFO, "DIR FILENAME[%s]", szDataFile);

		/* Check Data File Format */
		if(memcmp(szData + usDataFileLen - 4, ".DAT", 4) != 0)
			continue;

		pFILE = fopen(szDataFile,"r");
		if(pFILE == NULL) {
			log_print(LOGN_CRI, "Data File Open Error DataFile=[%s] err=%d[%s]", 
			szDataFile, errno, strerror(errno));
		}

		dRecCnt = 0;
		while(fgets(szData, 1024*20, pFILE) != NULL)
			dRecCnt++;

		fclose(pFILE);

		if(dRecCnt == 0) {
			log_print(LOGN_CRI, "File Record Count = %d So do not make Fin File = [%s]",
				dRecCnt, szDataFile);
			dRet = unlink(szDataFile);
			if(dRet < 0) {
				log_print(LOGN_CRI, "REC=%d But Data File Remove Err[%d][%s] F=[%s]",
					dRecCnt, errno, strerror(errno), szDataFile);
			}
			continue;
		}
		
		/* FinFile is used for RenameFile of Data File */
		memcpy(szFinFile, SNLOG_PATH, SNLOG_PATH_SIZE);
		usFinFileLen = SNLOG_PATH_SIZE;
		memcpy(szFinFile + usFinFileLen, szDataFile + CILOG_PATH_SIZE, usDataFileLen - CILOG_PATH_SIZE);
		usFinFileLen += usDataFileLen - CILOG_PATH_SIZE;
		szFinFile[usFinFileLen] = 0x00;


		/* Move Directory CILOG -> SILOG */
		dRet = rename(szDataFile, szFinFile);
		if(dRet < 0) {
			log_print(LOGN_CRI, "DATA FILE MOVE Error[%d][%s] from [%s] to [%s]", 
			errno, strerror(errno), szDataFile, szFinFile);
		} else {
			;
		}

		memcpy(szFinFile + usFinFileLen - 3, "FIN", 3);
		umask(000);
		pFILE = fopen(szFinFile,"w");
		if(pFILE == NULL) {
			log_print(LOGN_CRI, "for Remain Data, Make Fin File Open Error FinFile=[%s] err=%d[%s]", 
				szFinFile, errno, strerror(errno));
			continue;
		} 

		fprintf(pFILE, "F\t%.*s\t%d\n", 
			FIN_FILENAME_SIZE, szDataFile + CILOG_PATH_SIZE, dRecCnt);
		fclose(pFILE);

		log_print(LOGN_CRI, "Remain Make Fin Rec=[%d] Data=[%s] File=[%s]", 
			dRecCnt, szDataFile, szFinFile);
	}

	closedir(pDIR);

	return 0;
}

int dSendFileName(FILEINFO *pFILEINFO)
{
	S32			dRet, len;
	st_SI_DB	*pSIDB;
	U8			*sp, *p;

	st_MsgQ		stMsgQ;
	st_MsgQSub	*pMsgQSub = (st_MsgQSub *)&stMsgQ.llMType;

	pMsgQSub->usType = DEF_SVC;
	pMsgQSub->usSvcID = SID_LOG;
	pSIDB = (st_SI_DB *)stMsgQ.szBody;
	stMsgQ.usBodyLen = sizeof(st_SI_DB);

	/* setting value */
	sp = pFILEINFO->szDataFile + CILOG_PATH_SIZE;
	len = strlen(pFILEINFO->szDataFile) - CILOG_PATH_SIZE;

	p = strstr(sp,"_T");
	if( p == NULL ){ log_print(LOGN_CRI,"%s:%d] Time Field is NOT EXIST : %s",__FUNCTION__, __LINE__, pFILEINFO->szDataFile); return -2; }
	memcpy(pSIDB->date, p+2, SIDB_DATE_LEN);
//	memcpy(pSIDB->date, &pFILEINFO->szDataFile[POS_TIME], SIDB_DATE_LEN);
	pSIDB->date[SIDB_DATE_LEN] = 0x00;


	memcpy(pSIDB->filename, &pFILEINFO->szDataFile[CILOG_PATH_SIZE], len);
	pSIDB->filename[len] = 0x00;
	memcpy(pSIDB->name, pFILEINFO->szDataFile, RNES_PKT_SIZE-1);
	pSIDB->name[RNES_PKT_SIZE-1] = 0x00;
	memcpy(pSIDB->name, SNLOG_PATH, SNLOG_PATH_SIZE);

	log_print(LOGN_DEBUG, "SEND NAME=%s FILENAME=%s DATE=%s LEN=%d", pSIDB->name, pSIDB->filename, pSIDB->date, len);

	if((dRet = dSendMsg(SEQ_PROC_SI_DB, &stMsgQ)) < 0) {
		log_print(LOGN_CRI, "F=%s:%s.%d dSendMsg dRet=%d NAME=%.*s",
				__FILE__, __FUNCTION__, __LINE__, dRet, MAX_FNAME_SIZE, pFILEINFO->szDataFile);
		return -1;
	}

	return 0;
}

int isRoam(int flag)
{
	if(flag >= ROAM_NASIP_BASE)	return 1;
	else						return 0;
}

/*
 *  $Log: mlog_main.c,v $
 *  Revision 1.4  2011/09/06 13:19:39  uamyd
 *  modified log version, MOND -> M_LOG
 *
 *  Revision 1.3  2011/09/05 05:32:59  hhbaek
 *  *** empty log message ***
 *
 *  Revision 1.2  2011/08/31 13:38:35  dcham
 *  *** empty log message ***
 *
 *  Revision 1.1  2011/08/31 13:13:41  dcham
 *  M_LOG added
 *
 *  Revision 1.21  2011/06/30 08:41:49  innaei
 *  *** empty log message ***
 *
 *  Revision 1.20  2011/05/01 14:35:03  jhbaek
 *  *** empty log message ***
 *
 *  Revision 1.19  2011/05/01 10:23:33  jhbaek
 *  *** empty log message ***
 *
 *  Revision 1.18  2011/04/19 12:37:20  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.17  2011/02/15 02:54:55  jsyoon
 *  *** empty log message ***
 *
 *  Revision 1.16  2011/01/11 04:09:17  uamyd
 *  modified
 *
 *  Revision 1.7  2010/11/19 07:48:30  uamyd
 *  주석 제거
 *
 *  Revision 1.6  2010/11/14 10:22:44  jwkim96
 *  STP 작업 내용 반영.
 *
 *  Revision 1.5  2010/09/29 06:50:05  uamyd
 *  added lastFailReason to LOG_DIAMETER
 *
 *  Revision 1.4  2010/09/29 00:53:55  uamyd
 *  added convertLogSignalToLogDiameter
 *
 *  Revision 1.3  2010/09/27 12:17:12  dqms
 *  TAM_DB protocol value set method changed
 *
 *  Revision 1.2  2010/09/17 08:45:03  dqms
 *  added code to apply with file-readed-system no
 *
 *  Revision 1.1.1.1  2010/08/23 01:13:11  uamyd
 *  DQMS With TOTMON, 2nd-import
 *
 *  Revision 1.15  2010/03/11 07:07:21  dqms
 *  A_ROAM LOG_FTP 전송하지 않도록 수정
 *
 *  Revision 1.14  2010/03/04 11:28:43  dark264sh
 *  M_LOG switch break 버그 수정
 *
 *  Revision 1.13  2010/03/03 08:41:47  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.12  2010/02/25 13:01:26  dark264sh
 *  M_LOG ROAM 처리
 *
 *  Revision 1.11  2009/08/03 09:02:55  dark264sh
 *  M_LOG LOG_FTP 처리 버그 수정 (MSRP에 저장하는 버그)
 *
 *  Revision 1.10  2009/07/12 13:09:16  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.9  2009/07/02 08:27:24  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.8  2009/06/27 12:54:23  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.7  2009/06/27 11:44:36  dark264sh
 *  M_LOG, M_SVCMON O_SVCMON 버전 처리
 *
 *  Revision 1.6  2009/06/25 12:24:43  dark264sh
 *  M_LOG LOG_RPPI_ERR 처리
 *
 *  Revision 1.5  2009/06/20 13:22:33  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.4  2009/06/19 11:39:46  dark264sh
 *  *** empty log message ***
 *
 *  Revision 1.3  2009/06/13 11:19:31  dark264sh
 *  M_LOG LOG_VT_SESS 처리
 *
 *  Revision 1.2  2009/06/12 01:12:49  dark264sh
 *  M_LOG file name 전송 처리
 *
 *  Revision 1.1  2009/06/10 23:52:59  dark264sh
 *  *** empty log message ***
 *
 */



