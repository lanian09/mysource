/*******************************************************************************
                DQMS Project

   Author   : Lee Dong-Hwan
   Section  : CAPD 
   SCCS ID  : @(#)capd_init.c (V1.0)
   Date     : 07/02/09
   Revision History :
        '09.    07. 02. initial

   Description :

   Copyright (c) uPRESTO 2005
*******************************************************************************/

/**A.1*  File Inclusion *******************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <string.h>

/* User Define */
#include <typedef.h>
#include <mems.h>
#include <nifo.h>
#include <loglib.h>
#include <common_stg.h>
//#include <tmf_cap.h>
#include <IP_header.h>
#include <Analyze_Ext_Abs.h>
#include <gifo.h>
#include <capdef.h>
#include "capd_test.h"
#include "capd_func.h"

/**B.1*  Definition of New Constants ******************************************/

/**B.2*  Definition of New Type  **********************************************/
/**C.1*  Declaration of Variables  ********************************************/
UCHAR  			*pstBuffer;
UCHAR 			*pstNode;
UCHAR 			*pstTLVNode;
#ifdef BPS
S64				ll_BPS_ReadBytes;
#endif
extern unsigned int	Collection_Cnt;
extern unsigned int Send_Node_Cnt;
UINT				gdSendTotalCnt = 0;
//UCHAR			*Send_Node_Head = NULL;

extern stMEMSINFO 	*pstMEMSINFO;
extern int		dPREQid;
extern U64		nifo_create;
extern U64		nifo_del;
extern int		g_nProcIndex;
//extern int		dPREACnt;
stCIFO		*pCIFO;
//extern UINT		uiSeqProcKey;
//extern UINT		uiPREAKey[];
//컴파일용 임시 변수
UINT		uiProcName=1;
UINT		uiProcKey=1;

/**D.1*  Definition of Functions  *********************************************/
/*
 *
 */
void test_func2(char *szDirName, int SLEEPCNT)
{
	time_t 		tCurTime, tLastTime;
	int 			nCount = 0, ret = 0;
	int			i, fp_read;
	int			pkt_cnt;
	int			read_size, tot_size;
	char			szFileHeader[PCAP_FILE_HDR_SIZE];
	char			szPktHeader[PCAP_DATA_HDR_SIZE];
	char 		szFileName[128];
	char 		szBuff[2000];
	struct stat 	fs;
	struct dirent	**ppstFileList;
	st_PcapHdr_t	stPktHeader;

	nCount = scandir(szDirName, &ppstFileList, FilterSOAP, alphasort);

	pkt_cnt = 0;
	for( i=0; i <nCount; i++)
	{
		sprintf( szFileName, "%s/%s", szDirName, ppstFileList[i]->d_name );
	  	printf(" FILE: %s\n", szFileName);
	  	fp_read = open( szFileName, O_RDONLY);
	  	fstat( fp_read, &fs );
	  	printf(" FILE SIZE : %ld \n", fs.st_size);
	  	read_size = read(fp_read, szFileHeader, PCAP_FILE_HDR_SIZE );

	  	if( read_size != PCAP_FILE_HDR_SIZE )
	  	{
	  		close(fp_read);
	  		continue;
	  	}

	  	tot_size = PCAP_FILE_HDR_SIZE;
	  	time(&tCurTime);

	  	do
	  	{
	  		read_size = read(fp_read, szPktHeader, PCAP_DATA_HDR_SIZE);

	  		if( read_size != PCAP_DATA_HDR_SIZE )
	  		{
	  			printf(" [ERROR] PCAP HEADER READ\n");
	  			break;
	  		}

	  		tot_size += read_size;

	  		memset(&stPktHeader, 0x00, PCAP_DATA_HDR_SIZE);
	  		memcpy(&stPktHeader, &szPktHeader, PCAP_DATA_HDR_SIZE);

	  		if( stPktHeader.dwLen > 4000 )
	  		{
	  			printf(" [EXCEPT] PACKET SIZE: %d\n", stPktHeader.dwLen);
	  			break;
	  		}

	  		read_size = read(fp_read, szBuff, stPktHeader.dwLen);
	  		if(read_size != stPktHeader.dwLen)
	  			break;

	  		if( read_size > 4000 )
	  			break;

	  		tot_size += read_size;

	  		ret = parse_data(&stPktHeader, szBuff);
	  		if( ret < 0 )
	  		{
	  			printf(" [ERROR] parse_data() Fail. RET[%d]\n", ret);
	  			break;
	  		}
	  		usleep(15);
	  		pkt_cnt++;
			gdSendTotalCnt++;
	  		if( SLEEPCNT != 0 && pkt_cnt % SLEEPCNT  == 0 )
	  		{
	  			//usleep(0);
	  		}
#ifdef BPS
	  		ll_BPS_ReadBytes += tot_size;
#endif
		} while(tot_size < fs.st_size);

		time(&tLastTime);
		if( tLastTime != tCurTime )
		printf(" BPS : %d \n", (int)( ( tot_size * 8 ) / (tLastTime - tCurTime) / 1024 / 1024));

		close( fp_read );
	}

#ifdef DEBUG
	log_print(LOG_DEBUG, "TOTAL PACKET COUNT[%u]", gdSendTotalCnt);
#endif

	/* */
	if((ret = dSend_CAPD_Data(pstMEMSINFO, 0, NULL, tCurTime, DEF_END)) < 0)
		log_print( LOG_CRI, " [ERROR] [END PROGRAM SEND] dSend_CAPD_Data() Fail. RET[%d] CAUSE[%s]", ret, strerror(-ret));

	free(ppstFileList);
	return ;
}

/*
 *
 */
int FilterSOAP(const struct dirent *entry)
{
	int	dLen, dEndLen;

	dLen = strlen(entry->d_name);
	dEndLen = strlen(STR_EXT_CAP);

	if( dLen > dEndLen )
	{
		if( strcmp(&entry->d_name[dLen-dEndLen], STR_EXT_CAP)==0 )
			return 1;
	}

	return 0;
}

/*
 *
 */
int parse_data(st_PcapHdr_t *pstHeader, char *szBuff)
{
	int				dRet;
	Capture_Header_Msg	*pstCAPHead;
	struct timeval		tvTime;

	/* Second Argument : NIFO ZONE NUMBER */
	if( (pstNode = nifo_node_alloc(pstMEMSINFO)) == NULL)
	{
		log_print( LOG_WARN, " [ERROR] nifo_node_alloc() Fail");
		usleep(0);
		return -1;
	}

	pstCAPHead = (Capture_Header_Msg *)nifo_tlv_alloc(pstMEMSINFO, pstNode, CAP_HEADER_NUM, CAP_HDR_SIZE, DEF_MEMSET_OFF);
	if(pstCAPHead == NULL)
	{
		log_print( LOG_CRI, " [ERROR] nifo_tlv_alloc(CAP_HEADER_NUM) Fail");
		nifo_node_delete(pstMEMSINFO, pstNode);
		return -2;
	}

	gettimeofday(&tvTime, NULL);

	pstCAPHead->bRtxType = 0;					/* Interface Index */
#if REALTIME
	pstCAPHead->curtime = tvTime.tv_sec;
	pstCAPHead->ucurtime = tvTime.tv_usec;
#else
	pstCAPHead->curtime = pstHeader->capTime;
	pstCAPHead->ucurtime = pstHeader->capMTime;
#endif
	pstCAPHead->datalen = pstHeader->dwLen;
	//pstCAPHead->usKeyType = 0;
	//pstCAPHead->szIPIMSI[0] = '\0';

	if( (pstBuffer = nifo_tlv_alloc(pstMEMSINFO, pstNode, ETH_DATA_NUM, pstCAPHead->datalen + 1, DEF_MEMSET_OFF)) == NULL )
	{
		log_print( LOG_CRI, " [ERROR] nifo_tlv_alloc(ETH_DATA_NUM) Fail");
		nifo_node_delete(pstMEMSINFO, pstNode);
		return -3;
	}

	/* Copy */
	memcpy(pstBuffer, szBuff, pstCAPHead->datalen);

	if((dRet = dSend_CAPD_Data(pstMEMSINFO, dGetPREAIndex(szBuff), pstNode, pstCAPHead->curtime, DEF_CONTINUE)) < 0)
	{
		log_print( LOG_CRI, " [ERROR] dSend_CAPD_Data() Fail. RET[%d] CAUSE[%s]", dRet, strerror(-dRet));
		nifo_node_delete(pstMEMSINFO, pstNode);
		return -4;
	}
#ifdef DEBUG
	else
	{
		log_print(LOG_DEBUG, " SEND OFFSET[%ld]. SEC[%ld] MICRO_SEC[%ld] DATA_LEN[%d]"
					, nifo_offset(pstMEMSINFO, pstNode), pstCAPHead->curtime, pstCAPHead->ucurtime, pstCAPHead->datalen);
	
	}
#endif

	return 0;
}

/*
 *
 */
int dGetPREAIndex(char *data)
{
	UINT 	srcip, dstip;
	IP_HDR_T	*hdr = (IP_HDR_T *)(data+14);

	srcip = TOUINT(hdr->Source);
	dstip = TOUINT(hdr->Destination);

	//return (srcip+dstip) % dPREACnt;
	return 0; 
}

/*
 *
 */
int dSend_CAPD_Data(stMEMSINFO *pstMEMSINFO, S32 dIdx, U8 *pNode, U32 sec, int type)
{
	int		dRet;
	OFFSET 	offset;

	/*
	 * SIGTERM, SIGUSR1 시그널을 받은 경우에는 
	 * 버퍼링하고 있는 패킷을 모두 전송 한 후에 종료 한다. 
	 */
	if(pNode == NULL)
	{
		Collection_Cnt = 0;
	}
	else
	{
		if(Send_Node_Head)
		{
			if(type == DEF_CONTINUE)
				nifo_node_link_nont_prev(pstMEMSINFO, Send_Node_Head, pNode);
			else if(type == DEF_END)
			{
				Send_Node_Cnt = Collection_Cnt + 1;
			}
		}
		else
		{
			Send_Node_Head = pNode;
		}
		Send_Node_Cnt++;
	}

	if(Send_Node_Cnt > Collection_Cnt)
	{
		offset = nifo_offset(pstMEMSINFO, Send_Node_Head);
#if 0
		if((dRet = gifo_write(pstMEMSINFO, pCIFO, uiSeqProcKey, uiPREAKey[dIdx], offset)) < 0)
		{
			log_print(LOG_CRI, " [ERROR] gifo_write() Fail. RET[%d] CAUSE[%s]", dRet, strerror(-dRet));
				return -1;
		}
#endif
		if((dRet = gifo_write(pstMEMSINFO, pCIFO, uiProcKey, uiProcName, offset)) < 0)
		{
			log_print(LOG_CRI, " [ERROR] gifo_write() Fail. RET[%d] CAUSE[%s]", dRet, strerror(-dRet));
				return -1;
		}
		else
		{
			//log_print(LOG_DEBUG, "### SEND. NODE CNT[%d] OFFSET[%ld]", Send_Node_Cnt, offset);
			Send_Node_Cnt = 0;
			Send_Node_Head = NULL;
		}
	}

	return 0;
}


/* END */

