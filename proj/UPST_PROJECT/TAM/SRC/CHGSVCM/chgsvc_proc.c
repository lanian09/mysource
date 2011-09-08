/*******************************************************************************
               DQMS Project

     Author   :
     Section  :
     SCCS ID  :
     Date     :
     Revision History :

     Description :

     Copyright (c) uPRESTO 2005
*******************************************************************************/
/** A.1* FILE INCLUDE *************************************/
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

/* User Define */
// LIB
#include "loglib.h"

// PROJECT
#include "common_stg.h"

// .
#include "chgsvc_list.h"
#include "chgsvc_proc.h"
#include "chgsvc_mem.h"

/** B.1* DEFINITION OF NEW CONSTANTS **********************/
#define MAX_BUF_READ_SIZE		1024	
#define MAX_STR_IP_SIZE			16
#define DEF_SVCLIST_OUT_PATH	"/TAMAPP/FTP_PUTLOG"


/** C.1* DEFINITION OF NEW TYPES **************************/
/** D.1* DECLARATION OF VARIABLES *************************/

/* Extern */
extern st_TOTLOG_LIST		*g_pstTOTLogList;
extern int					gdTAMID;
extern st_GOOGLEPUSH_MEM	stGooglePushMemList;


extern unsigned long long	gullTotCnt; /* TEST */

/** E.1* DEFINITION OF FUNCTIONS **************************/
int dGetHttpLogDataReadFile(char *filename);
int dGetTcpLogDataReadFile(char *filename);


/** E.2* DEFINITION OF FUNCTIONS **************************/
/*******************************************************************************
 [argument]
 	type: HTTP_LOG, TCP_LOG를 구분하기 위한 타입정보
*******************************************************************************/
int dGetDirecFileList(char *rootpath, time_t curtime)
{
	int				dRet, fileCnt = 0, len;
	DIR				*dirp;
	struct dirent	*direntp;
	struct stat		fstat;
	char			filepath[256], direcpath[128], direcname[32];
	time_t			tTime;
	struct tm 		tm;
	int				HttpFileCnt = 0, TcpFileCnt = 0;

	tTime = curtime - 86400;
	localtime_r(&tTime, &tm);
	sprintf(direcname, "%04d%02d%02d", tm.tm_year + 1900, tm.tm_mon +1 , tm.tm_mday);

	sprintf(direcpath, "%s/%s", rootpath, direcname);

	log_print( LOGN_CRI, "[%s:%s] GET LOG DIRECTORY PATH[%s]", __FILE__, __FUNCTION__, direcpath);

	dRet = stat(direcpath, &fstat);
	if( dRet < 0) {
		log_print(LOGN_CRI, "[%s.%d] [ERROR] stat() Fail. PATH[%s] CAUSE[%s]", __FUNCTION__, __LINE__, direcpath, strerror(errno));
		return -1;
	}

	/* 디렉토리 체크 */
	if( S_ISDIR(fstat.st_mode)) {
		if( (dirp = opendir(direcpath)) == (DIR *)NULL) {
			log_print(LOGN_CRI, "[%s.%d] [ERROR] opendir() Fail. PATH[%s] CAUSE[%s]", __FUNCTION__, __LINE__
						, direcpath, strerror(errno));
			return -2;
		}

		while( (direntp = readdir(dirp)) != NULL)
		{
			if( (strcmp(direntp->d_name, ".") == 0) || (strcmp(direntp->d_name, "..") == 0) )
				continue;

			fileCnt++;
			sprintf(filepath, "%s/%s", direcpath, direntp->d_name);

			dRet = stat(filepath, &fstat);
			if( dRet < 0) {
				log_print(LOGN_CRI, "[%s.%d] [ERROR] stat() Fail. PATH[%s] CAUSE[%s]", __FUNCTION__, __LINE__
							, filepath, strerror(errno));
				continue;
			}

			/* 파일이면 */
			if( S_ISREG(fstat.st_mode)) {
				if(strstr(direntp->d_name, "_FIHT_")) {		/* HTTP LOG */
					/* .DAT 파일 */
					len = strlen(direntp->d_name);	
					if( strncmp(&direntp->d_name[len-3], "DAT", 3) == 0) {
						log_print(LOGN_DEBUG, "[%s.%d] READ [HTTP][%d] FILE[%s]", __FUNCTION__, __LINE__
										 , HttpFileCnt, direntp->d_name);
						dRet = dGetHttpLogDataReadFile(filepath);
						if(dRet < 0) {
							log_print(LOGN_CRI, "[%s.%d] [ERROR] dGetHttpLogDataReadFile() Fail. RET[%d] FILEPATH[%s]"
									, __FUNCTION__, __LINE__, dRet, filepath);
						}

						HttpFileCnt++;
					}
				} else if(strstr(direntp->d_name, "_FITC_")) {	/* TCP LOG */
					/* .DAT 파일 */
					len = strlen(direntp->d_name);	
					if( strncmp(&direntp->d_name[len-3], "DAT", 3) == 0) {
						log_print(LOGN_DEBUG, "[%s.%d] READ [TCP][%d] FILE[%s]", __FUNCTION__, __LINE__, TcpFileCnt, direntp->d_name);
						dRet = dGetTcpLogDataReadFile(filepath);
						if(dRet < 0) {
							log_print(LOGN_CRI, "[%s.%d] [ERROR] dGetTcpLogDataReadFile() Fail. RET[%d] FILEPATH[%s]"
									, __FUNCTION__, __LINE__, dRet, filepath);
						}

						TcpFileCnt++;
					}
				}
			}
		}

		closedir(dirp);
	} else {
		log_print(LOGN_CRI, "[%s.%d] [ERROR] NOT DIRECTORY PATH. [%s]", __FUNCTION__, __LINE__, direcpath);
		return -5;
	}

	if(fileCnt == 0)
		log_print( LOGN_CRI, "[%s:%s][ERROR] NO EXIST FILE. PATH[%s]", __FILE__, __FUNCTION__, direcpath);

	return 1;
}

/*******************************************************************************
  
*******************************************************************************/
int dParseHttpLogData(char *inbuf, char *outip, char *outhostname, long long *outpktcnt)
{
	int			i, len, oldtabpos = -1, tokencnt = 0, tokensize;
	int			upcnt = 0, dncnt = 0;
	char		token[256];

	len = strlen(inbuf);

	for(i = 0; i < len; i++) {

		if(inbuf[i] == '\t') {
			if(oldtabpos + 1 == i) {
				memcpy(token, "NONE", 4);
				token[4] = '\0';
			} else {
				tokensize = i - (oldtabpos+1);

				switch(tokencnt) {
					case 22: /* Hostname */
						memcpy(outhostname, &inbuf[oldtabpos+1], tokensize);
						outhostname[tokensize] = '\0';
						break;
					case 27: /* Server IP */
						memcpy(outip, &inbuf[oldtabpos+1], tokensize);
						outip[tokensize] = '\0';
						break;
					case 57: /* Up Packet Cnt*/
						memcpy(token, &inbuf[oldtabpos+1], tokensize);
						token[tokensize] = '\0';
						upcnt = atoi(token);
						break;
					case 58: /* Down Packet Cnt*/
						memcpy(token, &inbuf[oldtabpos+1], tokensize);
						token[tokensize] = '\0';
						dncnt = atoi(token);
						break;
				}
			}
		
			tokencnt++;
			oldtabpos = i;
		}
	
		if(tokencnt > 58)
			break;
	}

	if(i == len)
		return -1;

	*outpktcnt = upcnt + dncnt;	
	return 1;
}

/*******************************************************************************
  
*******************************************************************************/
int dParseTcpLogData(char *inbuf, char *outip, long long *outpktcnt)
{
	int		i;
	int		len, oldtabpos = -1, tokensize, tokencnt = 0;
	int		upcnt = 0, dncnt = 0;
	char	token[64];
	int		dRet;
	char	strport[32];
	int		port = 0;
	int		d1, d2, d3, d4;


	len = strlen(inbuf);

	for(i = 0; i < len; i++) {

		if(inbuf[i] == '\t') {
			if(oldtabpos + 1 == i) {
				memcpy(token, "NONE", 4);
				token[4] = '\0';
			} else {
				tokensize = i - (oldtabpos+1);

				switch(tokencnt) {
					case 30: /* Server IP */
						memcpy(outip, &inbuf[oldtabpos+1], tokensize);
						outip[tokensize] = '\0';
						break;
					case 32: /* Server Port */
						memcpy(strport, &inbuf[oldtabpos+1], tokensize);
						strport[tokensize] = '\0';
						port = atoi(strport);
#if 0
						if(port == 5228)
							log_print(LOG_INFO, "PORT=%d", port);
#endif
						break;
					case 61: /* Up Packet Cnt */
						memcpy(token, &inbuf[oldtabpos+1], tokensize);
						token[tokensize] = '\0';
						upcnt = atoi(token);
						break;
					case 62: /* Down Packet Cnt*/
						memcpy(token, &inbuf[oldtabpos+1], tokensize);
						token[tokensize] = '\0';
						dncnt = atoi(token);
						break;
				}
			}
		
			tokencnt++;
			oldtabpos = i;
		}

		if(tokencnt > 62)
			break;
	}
	
	if(i == len)
		return -1;

	if(port == 5228) {
		dRet = sscanf(outip, "%d.%d.%d.%d", &d1, &d2, &d3, &d4);
		if(dRet == 4) {
			if(d4 == 188) { /* x.x.x.188:5228 */
				struct in_addr inaddr;

				inet_aton(outip, &inaddr);
				dAddGooglePushHash(ntohl(inaddr.s_addr));
			}
		} else {
			log_print(LOGN_CRI, "[%s.%d] GOOGLE_PUSH. INVALID SERVER IP[%s]", __FUNCTION__, __LINE__, outip);
		}
	}

	*outpktcnt = upcnt + dncnt;
	return 1;
}


/*******************************************************************************
 정책: 
 	지정된 depth보다 도메인네임이 더 클 경우, 앞 단부터 지정된 depth 만큼
	삭제한다.
*******************************************************************************/
int dFilteringHostname(char *hostname)
{
	int	keywordch = 0;
	int	depth = 0, pos = 0;
	int	i, len = 0;
	int	cnt = 0, diff, len2;
	char	temp[128];

	if( (hostname[0] == '\0') || ((len = strlen(hostname)) < 3))
		return -1;

	if( strncmp(hostname, "www", 3) == 0)
		keywordch = 1;

	for(i = 0; i < len; i++) {
		if(hostname[i] == '.') {
			depth++;

			if((keywordch == 1) && (pos == 0))
				pos = i;
		}
	}

	if(depth > MAX_HOSTNAME_DEPTH) {
		diff = depth - MAX_HOSTNAME_DEPTH;
		for(i = 0; i < len; i++) {
			if(hostname[i] == '.') {
				cnt++;

				if(cnt >= diff) {
					len2 = strlen(&hostname[i+1]);
					if(len2 > 128)
						len2 = 128;
					strncpy(temp, &hostname[i+1], len2);
					
					strncpy(hostname, temp, len2);
					hostname[len2] = '\0';
					break;
				}
			}
		}
	} else {
		if(keywordch > 0) {
			len2 = strlen(&hostname[pos+1]);
			if(len2 > 128)
				len2 = 128;
			strncpy(temp, &hostname[pos+1], len2);

			strncpy(hostname, temp, len2);
			hostname[len2] = '\0';
		}
	}

	return 1;
}

/*******************************************************************************
  
*******************************************************************************/
int dGetHttpLogDataReadFile(char *filename)
{
	int				dRet;
	FILE			*fp;
	char			buf[MAX_BUF_READ_SIZE];
	char			tokenIP[MAX_STR_IP_SIZE];
	char			tokenHostname[128];
	UINT			IP;
	long long		pktcnt = 0;
	struct in_addr	inaddr;

	if( (fp = fopen(filename, "r")) == NULL) {
		log_print(LOGN_CRI, "[%s.%d] [ERROR] fopen() Fail. FILENAME[%s] CAUSE[%s]"
					, __FUNCTION__, __LINE__, filename, strerror(errno));
		return -1;
	}

	while( fgets(buf, MAX_BUF_READ_SIZE, fp) != NULL) {
		gullTotCnt++;	/* TEST */

		dRet = dParseHttpLogData(buf, tokenIP, tokenHostname, &pktcnt);
		if(dRet > 0) {
			//log_print(LOGN_DEBUG, "[HTTP] HOSTNAME=%s IP=%s PKTSUM=%lld", tokenHostname, tokenIP, pktcnt);
			inet_aton(tokenIP, &inaddr);
			IP = (UINT)ntohl(inaddr.s_addr);

			/* 0.0.0.0 예외 처리 */
			dRet = inet_aton(tokenHostname, &inaddr);
			if(dRet == 0) {		/* IP 에 대한 무효 처리 */
				/* 대표 호스트네임 추출 */
 				dFilteringHostname(tokenHostname);

				dRet = dSetHttpLogHash(tokenHostname, IP, pktcnt);
				if(dRet < 0) {
					fclose(fp);
					return -2;
				}
			} else {
				//log_print(LOGN_CRI, "[%s.%d] [ERROR] INVALID HOSTNAME[%s]", __FUNCTION__, __LINE__, tokenHostname);
			}
		} else {
			log_print(LOGN_CRI, "[%s.%d] [ERROR] dParseHttpLogData() Fail. RET[%d]", __FUNCTION__, __LINE__, dRet);
		}
	}

	fclose(fp);

	return 1;
}

/*******************************************************************************
  
*******************************************************************************/
int dGetTcpLogDataReadFile(char *filename)
{
	int				dRet;
	FILE			*fp;
	char			buf[MAX_BUF_READ_SIZE];
	char			tokenIP[MAX_STR_IP_SIZE];
	UINT			IP;
	long long		pktcnt = 0;
	struct in_addr	inaddr;

	if( (fp = fopen(filename, "r")) == NULL) {
		log_print( LOGN_CRI, "[%s.%d] [ERROR] fopen() Fail. FILENAME[%s]", __FUNCTION__, __LINE__, filename);
		return -1;
	}

	while( fgets(buf, MAX_BUF_READ_SIZE, fp) != NULL) {
		gullTotCnt++; 	/* TEST */

		dRet = dParseTcpLogData(buf, tokenIP, &pktcnt);
		if(dRet > 0) {
			//log_print(LOGN_DEBUG, "[TCP] IP=%s PKTSUM=%lld", tokenIP, pktcnt);
			inet_aton(tokenIP, &inaddr);
			IP = (UINT)ntohl(inaddr.s_addr);

			dRet = dSetTcpLogHash(IP, pktcnt);
			if(dRet < 0) {
				fclose(fp);
				return -2;
			}
		} else {
			log_print(LOGN_CRI, "[%s.%d] [ERROR] dParseTcpLogData() Fail. RET[%d]", __FUNCTION__, __LINE__, dRet);
		}
	}

	fclose(fp);

	return 1;
}

/*******************************************************************************
  
*******************************************************************************/
int dWriteHttpTcpLogStatisticFile(time_t curtime)
{
	int				dRet;
	int             i, j;
	FILE            *fp;
	st_HTTPLOG_LIST *pstHttpLog;
	st_IPLIST       *pstIPList;
	st_TCPLOG_LIST  *pstTcpLog;
	char			filename[64], filepath[128];
	char			filename2[64], filepath2[128];
	time_t			tTime;
	struct tm		tm;

	/* 이전 파일 삭제 */ 
	tTime = curtime - 86400;
	localtime_r(&tTime, &tm);

	/* */
	sprintf(filename, "HTTPSVC_TAM%d_%04d%02d%02d.DAT", gdTAMID
					, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	sprintf(filepath, "%s/%s", DEF_SVCLIST_OUT_PATH, filename);
	dRet = unlink(filepath);		/* 삭제 */
	if(dRet != 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] unlink Fail. FILE[%s] CAUSE[%s]", __FILE__, __FUNCTION__
						, filepath, strerror(errno));
	} else if(dRet == 0)
		log_print( LOGN_DEBUG, "[%s:%s] FILE[%s] DELETE.", __FILE__, __FUNCTION__, filepath);

	/* */
	sprintf(filename, "HTTPSVC_TAM%d_%04d%02d%02d.FIN", gdTAMID
					, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	sprintf(filepath, "%s/%s", DEF_SVCLIST_OUT_PATH, filename);
	dRet = unlink(filepath);		/* 삭제 */
	if(dRet != 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] unlink Fail. FILE[%s] CAUSE[%s]", __FILE__, __FUNCTION__
						, filepath, strerror(errno));
	} else if(dRet == 0)
		log_print( LOGN_DEBUG, "[%s:%s] FILE[%s] DELETE.", __FILE__, __FUNCTION__, filepath);

	/* */
	sprintf(filename, "TCPSVC_TAM%d_%04d%02d%02d.DAT", gdTAMID
					, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	sprintf(filepath, "%s/%s", DEF_SVCLIST_OUT_PATH, filename);
	dRet = unlink(filepath);		/* 삭제 */
	if(dRet != 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] unlink Fail. FILE[%s] CAUSE[%s]", __FILE__, __FUNCTION__
						, filepath, strerror(errno));
	} else if(dRet == 0)
		log_print( LOGN_DEBUG, "[%s:%s] FILE[%s] DELETE.", __FILE__, __FUNCTION__, filepath);

	/* */
	sprintf(filename, "TCPSVC_TAM%d_%04d%02d%02d.FIN", gdTAMID
					, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	sprintf(filepath, "%s/%s", DEF_SVCLIST_OUT_PATH, filename);
	dRet = unlink(filepath);		/* 삭제 */
	if(dRet != 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] unlink Fail. FILE[%s] CAUSE[%s]", __FILE__, __FUNCTION__
						, filepath, strerror(errno));
	} else if(dRet == 0)
		log_print( LOGN_DEBUG, "[%s:%s] FILE[%s] DELETE.", __FILE__, __FUNCTION__, filepath);

	localtime_r(&curtime, &tm);

	/* 새로운 파일 생성 */
	sprintf(filename, "HTTPSVC_TAM%d_%04d%02d%02d.DAT", gdTAMID
					, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	sprintf(filepath, "%s/%s", DEF_SVCLIST_OUT_PATH, filename);

	if( (fp = fopen(filepath, "w")) == NULL) {
		log_print( LOGN_CRI, "[%s.%d] [ERROR] fopen() Fail. FILE[%s] CAUSE[%s]", __FUNCTION__, __LINE__
				, filepath, strerror(errno));
		return -1;
	}

	for(i = 0; i < g_pstTOTLogList->uiHttpLogCnt; i++) {
		pstHttpLog = &g_pstTOTLogList->stHttpLogList[i];
		pstIPList = &g_pstTOTLogList->stIPList[pstHttpLog->uiArrayIndex];

		for(j = 0; j < pstIPList->uiCnt; j++) {
			fprintf(fp, "%s %u %u\n", pstIPList->szHostname, pstIPList->uiIPList[j], pstIPList->uiIPList[j]);
		}
	}

	fclose(fp);

	sprintf(filename2, "HTTPSVC_TAM%d_%04d%02d%02d.FIN", gdTAMID
					, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	sprintf(filepath2, "%s/%s", DEF_SVCLIST_OUT_PATH, filename2);

	dRet = rename(filepath, filepath2);
	if(dRet != 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] [HTTP] rename Fail. OLD_FILE[%s] NEW_FILE[%s]", __FILE__, __FUNCTION__
						, filename, filename2);
	}

	sprintf(filename, "TCPSVC_TAM%d_%04d%02d%02d.DAT", gdTAMID
					, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	sprintf(filepath, "%s/%s", DEF_SVCLIST_OUT_PATH, filename);

	if( (fp = fopen(filepath, "w")) == NULL) {
		log_print( LOGN_CRI, "[%s.%d] [ERROR] fopen() Fail. FILE[%s] CAUSE[%s]", __FUNCTION__, __LINE__
				, filepath, strerror(errno));
		return -1;
	}

	for(i = 0; i < g_pstTOTLogList->uiTcpLogCnt; i++) {
		pstTcpLog = &g_pstTOTLogList->stTcpLogList[i];

		fprintf(fp, "%u %lld\n", pstTcpLog->uiIP, pstTcpLog->llPktCnt);
	}

	fclose(fp);

	sprintf(filename2, "TCPSVC_TAM%d_%04d%02d%02d.FIN", gdTAMID
					, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);

	sprintf(filepath2, "%s/%s", DEF_SVCLIST_OUT_PATH, filename2);

	dRet = rename(filepath, filepath2);
	if(dRet != 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] [TCP] rename Fail. OLD_FILE[%s] NEW_FILE[%s]", __FILE__, __FUNCTION__
						, filename, filename2);
	}

	return 1;
}             

int dWriteGooglePushList(time_t curtime)
{
	int         dRet, i;
	char        filename[64], filepath[128];
	FILE        *fp;
	time_t      tTime;
	struct tm   tm;

	tTime = curtime - 86400;
	localtime_r(&tTime, &tm);

	sprintf(filename, "GOOGLEPUSH_TAM%d_%04d%02d%02d.DAT", gdTAMID
			, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	sprintf(filepath, "%s/%s", DEF_SVCLIST_OUT_PATH, filename);
	dRet = unlink(filepath);
	if(dRet != 0) {
		log_print( LOGN_CRI, "[%s:%s][ERROR] unlink Fail. FILE[%s] CAUSE[%s]", __FILE__, __FUNCTION__
				, filepath, strerror(errno));
	} else if(dRet == 0)
		log_print( LOGN_DEBUG, "[%s:%s] FILE[%s] DELETE.", __FILE__, __FUNCTION__, filepath);

	localtime_r(&curtime, &tm);

	sprintf(filename, "GOOGLEPUSH_TAM%d_%04d%02d%02d.DAT", gdTAMID
			, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	sprintf(filepath, "%s/%s", DEF_SVCLIST_OUT_PATH, filename);

	if( (fp = fopen(filepath, "w")) == NULL) {
		log_print( LOGN_CRI, "[%s.%d] [ERROR] fopen() Fail. FILE[%s] CAUSE[%s]", __FUNCTION__, __LINE__
				, filepath, strerror(errno));
		return -1;
	}

	for(i = 0; i < stGooglePushMemList.dCnt; i++) {
		fprintf(fp, "%u\n", stGooglePushMemList.IPList[i]);
	}

	fclose(fp);

	return 1;
}




