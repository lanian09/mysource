#ifndef __cifo_h__
#define __cifo_h__
/**		file  cifo.h
 *		- header file
 *
 *		Copyright (c) 2006~ by Upresto Inc, Korea
 *		All rights reserved
 *
 *		$Id: cifo.h,v 1.3 2011/08/19 06:36:22 dcham Exp $
 * 
 *		@Author		$Author: dcham $
 *		@version	$Revision: 1.3 $
 *		@date		$Date: 2011/08/19 06:36:22 $
 *		@warning	$type
 *		@ref		cifo.h
 *		@todo		이제 시작이다. 
 *
 *		@section	Intro
 *		- header file
 *
 *		@section	Requirement
 *		@li 
 *
 **/

#include "nifo.h"
#include "mems.h"

#define CIFO_SHM_SIZE		800*1024*1024

#define MAX_CHAN_CNT		1000

#define	S_SSHM_CIFO			16000
#define	S_WSEMA_CIFO		12000
#define	S_RSEMA_CIFO		12002

#define	CIFO_R_FLAG			0
#define	CIFO_W_FLAG			1

enum CHANN_ID 
{
	CIFO_CHAN_0 = 0,
	CIFO_CHAN_1,
	CIFO_CHAN_2,
	CIFO_CHAN_3,
	CIFO_CHAN_4,
	CIFO_CHAN_5,
	CIFO_CHAN_6,
	CIFO_CHAN_7,
	CIFO_CHAN_8,
	CIFO_CHAN_9,
	CIFO_CHAN_10
};

/**
 *	@brief  CHAN :
 *	개별 CHANNEL 관리 구조체 header
 *	
 * 	@see cifo.h
 *
 */
typedef struct __CHANNEL__ 
{
	U32 	uiChID;		// 채널 ID
	U32 	wCellPos; 	// cifo_write에서 NIFO NODE OFFSET 값을 저장할 write cell position 
	U32		rCellPos;	// cifo_read에서 NIFO NODE OFFSET 값을 읽어올 read cell position
	U32		uiCellCnt;	// 한 채널에 할당된 Cell 개수
	U32		uiCellSize; 	// 한 Cell에 할당된 memory 사이즈 
	OFFSET	cellStartOffset;	// 자기 채널에서 Cell이 시작되는 메모리 OFFSET 값을 저장. 
	OFFSET	cellEndOffset;		// 자기 채널에서 Cell이 끝나는 메모리 OFFSET 값을 저장. 
	U32		uiWBuffCnt;	// 버퍼링 관련 변수 : write 시 buffering 할 개수
	U32		uiRBuffCnt;	// 버퍼링 관련 변수 : read 시 buffering 할 개수 
	U32		uiWCnt;		// 버퍼링 관련 변수 : write buffering 에서 현재까지 buffering 한 개수
	U32		uiRCnt;		// 버퍼링 관련 변수 : read	buffering 에서 현재까지 buffering 한 개수 
	OFFSET	wOffset;	// 버퍼링 관련 변수 : write buffering 시 최초 NODE의 OFFSET 임시 저장용.
	OFFSET	rOffset;	// 버퍼링 관련 변수 : read buffering 시 최초 NODE의 OFFSET 임시 저장용. 
	U32		uiWSemFlag;	// write 시 세마포어 사용여부. 
	U32		uiWSemKey;	// write 시 세마포어 key
	U32		iWSemID;		// write 시 세마포어 ID
	U32		uiRSemFlag;	// read 시 세마포어 사용여부. 
	U32		uiRSemKey;	// read 시 세마포어 key
	U32		iRSemID;		// read 시 세마포어 ID
} st_CHAN;
#define	st_CHAN_SIZE sizeof(st_CHAN)

/**
 *	@brief  CIFO :
 *	CIFO 최상위 헤더 
 *	
 * 	@see cifo.h
 *
 */
typedef struct __CIFO__
{
	U32 		uiShmKey;			// DIFO(CIFO + GIFO + CELL) Shared Memory Key
	OFFSET		uiTotMemSize;		// DIFO 전체 메모리 사이즈 
	U32			uiHeadRoomSize;		// GIFO 위치할 공간의 사이즈  또는 다른 용도로 사용하기 위한 공간
	OFFSET		offsetHeadRoom;		// headRoom의 시작 OFFSET
	U32			uiChCnt;			// 사용중인 Channel Count
	st_CHAN 	stCHAN[MAX_CHAN_CNT];	
} stCIFO;
#define	st_CIFO_SIZE sizeof(stCIFO)

/**
 *	@brief  CHAN_CONF :
 *	채널 별 Configuration 헤더 
 *	
 * 	@see cifo.h
 *
 */
typedef struct __CHANNEL_CONFIG__
{
	U32 		uiChID;
	U32			uiCellCnt;
	U32			uiCellSize;
	U32			uiWBuffCnt;
	U32			uiRBuffCnt;
	U32			uiWSemFlag;
	U32			uiWSemKey;
	U32			iWSemID;
	U32			uiRSemFlag;
	U32			uiRSemKey;
	U32			iRSemID;
} st_CHANCONF;
#define	st_CHANCONF_SIZE sizeof(st_CHANCONF)

/**
 *	@brief  CIFO_CONF :
 *	CIFO Configuration 헤더 
 *	
 * 	@see cifo.h
 *
 */
typedef struct __CIFO_CONF__
{
	U32			uiShmKey;
	U32			uiHeadRoomSize;
	U32			uiChCnt;
	st_CHANCONF	stCHANCONF[MAX_CHAN_CNT];
} st_CIFOCONF;
#define	st_CIFOCONF_SIZE sizeof(st_CIFOCONF)

/**
 *	difo project : cifo external function
 */

#define cifo_next_pos(curPos, cellCnt) ((curPos + 1) % cellCnt)
#define cifo_get_cell(p, pos) ((OFFSET *)(p->cellStartOffset + pos * p->uiCellSize))

extern stCIFO* 	cifo_init_channel(S8 *confFile);
extern stCIFO* 	cifo_init(st_CIFOCONF *stCIFOCONF);
extern int		cifo_write(stMEMSINFO *pstMEMSINFO, stCIFO *pstCIFO, U32 uiChID, OFFSET offset);
extern OFFSET	cifo_read(stMEMSINFO *pstMEMSINFO, stCIFO *pstCIFO, U32 uiChID);
extern S32 		cifo_conf_init(st_CIFOCONF* pCIFOCONF, S8 *confFile);
extern U32 		cifo_used_count(stCIFO *pstCIFO, U32 uiChID);
extern U32 		cifo_free_count(stCIFO *pstCIFO, U32 uiChID);
extern U32 		cifo_max_count(stCIFO *pstCIFO, U32 uiChID);
extern void		cifo_print(stCIFO *pstCIFO);
extern void		cifo_print_channel(stCIFO *pstCIFO, U32 uiChID);

extern U32		cifo_get_buffcnt(stCIFO *pstCIFO, U32 uiChID, S32 uiRWFlag);
extern S32		cifo_set_buffcnt(stCIFO *pstCIFO, U32 uiChID, U32 uiBuffCnt, S32 uiRWFlag);
extern void 	cifo_conf_print(st_CIFOCONF *pCIFOCONF);

/** file : cifo.h
 *     $Log: cifo.h,v $
 *     Revision 1.3  2011/08/19 06:36:22  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/08/18 04:58:27  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/07/26 06:13:03  dhkim
 *     *** empty log message ***
 *
 *     Revision 1.2  2011/01/21 01:39:34  swpark
 *     LOG classfy
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.10  2011/01/05 08:57:23  swpark
 *     PATH delete
 *
 *     Revision 1.9  2010/12/29 07:28:30  swpark
 *     confile add
 *
 *     Revision 1.8  2010/12/28 07:23:49  swpark
 *     buffcnt api add
 *
 *     Revision 1.7  2010/12/09 08:57:52  jjinri
 *     CIFO_SHM_SIZE
 *
 *     Revision 1.6  2010/12/09 08:27:19  jjinri
 *     cifo_print
 *
 *     Revision 1.5  2010/12/09 07:42:05  jjinri
 *     CIFO_SHM_SIZE
 *
 *     Revision 1.4  2010/12/09 06:46:53  swpark
 *     config proto type add
 *
 *     Revision 1.3  2010/12/09 05:21:16  dark264sh
 *     *** empty log message ***
 *
 *     Revision 1.2  2010/12/06 06:49:51  jjinri
 *     cellStartPos -> cellStartOffset, rCellPos, wCellPos : OFFSET -> U32
 *
 *     Revision 1.1  2010/12/02 12:50:37  upst_cvs
 *     2010.1202 commit start
 *
 *
 *		*/
#endif /* __cifo_h__ */
