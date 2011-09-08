#ifndef	__NIFO_H__
#define	__NIFO_H__
/**
 *		- hash header file
 *
 *		Copyright (c) 2006~ by Upresto Inc, Korea
 *		All rights reserved
 *
 *		$Id: nifo.h,v 1.3 2011/04/27 14:30:40 jjinri Exp $
 * 
 *		@Author      $Author: jjinri $
 *		@version     $Revision: 1.3 $
 *		@date        $Date: 2011/04/27 14:30:40 $
 *		@ref         nifo.h
 *		@todo        Makefile을 만들자
 *
 *		@section     Intro(소개)
 *		- hash header file
 *
 *		@section     Requirement
 *		@li 규칙에 틀린 곳을 찾아주세요.
 *
 **/



/* 필요한 header file include */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <time.h>
#include "clisto.h"
#include "mems.h"

/* NIFO STRUCTURE NUMBER */
#define CAP_HEADER_NUM          1
#define ETH_DATA_NUM            2
#define INFO_ANA_NUM            3
#define TCP_DATA_NUM            4
#define INFO_ACC_NUM            5
#define MSGQ_HEADER_NUM         6
#define NOTIFY_SIG_DEF_NUM		7

#define OFFSET_SIZE				sizeof(OFFSET)

#define NIFO_ZONE_0         0
#define NIFO_ZONE_1         1
#define NIFO_ZONE_2         2
#define NIFO_ZONE_3         3
#define NIFO_ZONE_4         4
#define NIFO_ZONE_5         5
#define NIFO_ZONE_6         6
#define NIFO_ZONE_7         7
#define NIFO_ZONE_8         8
#define NIFO_ZONE_9         9

#define MAX_ZONE_CNT        2
#define MAX_ZONE_NUM        (MAX_ZONE_CNT - 1)

/** 
 *  @brief st_NIFO_NODE_INFO : 
 *	Shared Memory, Semaphore, Message Queue, Offset Linked List를 이용한 Interface
 *
 *	@see nifo.h 
 *
 *  @note 		interface 
 */
typedef struct _st_nifo_common {
	clist_head 		nont;
	clist_head 		cont;
	S32 			from;
	S32 			to;
	S32				cnt;			/**< TLV 개수 */
	OFFSET			lastoffset;		/**< 사용 가능한 OFFSET */
	OFFSET			maxoffset;		/**< 사용 가능한 최대 OFFSET */
} NIFO;
#define STG_DEF_NIFO		101		/* Hex( 0x65 ) */
#define NIFO_SIZE sizeof(NIFO)


/**
 * @brief st_TLV : Node의 기본 정보 수록
 *
 *
 * @see l4.h
 *
 * @note	모든 구조체 앞에 붙는다. 
 *
 */
typedef struct _st_TLV {
	U32				type;				/**< 구조체 번호 */
	U32				len;				/**< Value's Length */
} TLV;
#define STG_DEF_TLV		102		/* Hex( 0x66 ) */
#define TLV_SIZE sizeof(TLV)

#if 0
/** 
 *  @brief st_MsgQ : 
 *	Message Queue 전송 header
 *
 *	@see nifo.h 
 *
 *  @note 		interface 
 */
typedef struct _st_MsgQ_nifo {
	OFFSET		mtype;		/**< st_MsgQSub로 cast해서 사용 */
	S32			msgqid;		/**< message queue id */
	U16			len;		/**< data size */
	U16			procid;		/**< process id */
} st_MsgQ_nifo;
#define STG_DEF_st_MsgQ		103		/* Hex( 0x67 ) */
#define st_MsgQ_nifo_SIZE sizeof(st_MsgQ_nifo)


#define DEF_MSGQ_SVC		1000
#define DEF_MSGQ_SYS		5000
/** 
 *  @brief st_MsgQSub : 
 *	Message Queue mtype 값
 *
 *	@see nifo.h 
 *
 *  @note 		interface 
 */
typedef struct _st_MsgQSub_nifo {
/**< <TAG_DEFINE:DEF_MSGQ_TYPE>
*	 DEF_MSGQ_SVC		1000
*	 DEF_MSGQ_SYS		5000
*/
	U16 	type;		/**< 메시지 Type (SVC, SYS 구분) */	
	U8			svcid;								/**< SVC 구분 */
	U8			msgid;								/**< MSG 구분 */
} st_MsgQSub_nifo;
#define STG_DEF_st_MsgQSub		104		/* Hex( 0x68 ) */
#define st_MsgQSub_nifo_SIZE sizeof(st_MsgQSub_nifo)
#endif

/** 
 *  @brief st_MsgQ : 
 *  Message Queue 전송 header
 *
 *  @see nifo.h 
 t_MsgQSub
 *  @note       interface 
 */
typedef struct _st_MsgQ_NIFO {
	OFFSET      mtype;      /**< st_MsgQSub·I castCØ¼­ ≫c¿e */
	S32         msgqid;     /**< message queue id */
	U16         len;        /**< data size */
	U16         procid;     /**< process id */
} st_MsgQ_NIFO;     
#define STG_DEF_st_MsgQ_NIFO        103     /* Hex( 531490 ) */
#define st_MsgQ_NIFO_SIZE sizeof(st_MsgQ_NIFO)
    
    
#define DEF_MSGQ_SVC        1000    
#define DEF_MSGQ_SYS        5000    

/** 
 *  @brief st_MsgQSub : 
 *  Message Queue mtype 값
 *
 *  @see nifo.h 
 *
 *  @note       interface 
 */
typedef struct _st_MsgQSub_NIFO {
	/**< <TAG_DEFINE:DEF_MSGQ_TYPE>
	 *    DEF_MSGQ_SVC       1000
	 *    DEF_MSGQ_SYS       5000
	 */
	U16     type;       /**< ¸Þ½AAo Type (SVC, SYS ±¸ºÐ) */
	U8          svcid;                              /**< SVC ±¸ºÐ */
	U8          msgid;                              /**< MSG ±¸ºÐ */
} st_MsgQSub_NIFO;
#define STG_DEF_st_MsgQSub_NIFO     104     /* Hex( 531490 ) */
#define st_MsgQSub_NIFO_SIZE sizeof(st_MsgQSub_NIFO)

#define IS_NOT_NIFO			1

#define DEF_MEMSET_OFF		0
#define DEF_MEMSET_ON		1
#define DEF_READ_ON			1
#define DEF_READ_OFF		2
#define DEF_READ_EMPTY		0
#define DEF_READ_MALLOC		1
#define DEF_READ_ORIGIN		2
/** 
 *  @brief REAL_VAL : 
 *	nifo_msgq_read에서 읽은 struct 관리를 위한 구조체
 *
 *	@see nifo.h 
 *
 *  @note 		interface 
 */
typedef struct _st_Read_Val {
/**< <TAG_DEFINE:DEF_READ_VAL_INIT>
*	 DEF_READ_ON			1
*	 DEF_READ_OFF		2
*/
	S8 	init;		/**< 관심 대상인지를 판단하는 Flag */
/**< <TAG_DEFINE:DEF_READ_VAL_MEM>
*	 DEF_READ_EMPTY		0
*	 DEF_READ_MALLOC		1
*	 DEF_READ_ORIGIN		2
*/
	S8 	memtype;	/**< 새로운 메모리를 할당하는지 표시 */
	S16			len;									/**< pVal의 사이즈 */
	U8			*pVal;									/**< 구조체 값 포인트 */
} READ_VAL;
#define STG_DEF_READ_VAL		105		/* Hex( 0x69 ) */
#define READ_VAL_SIZE sizeof(READ_VAL)


#define MAX_READVAL_CNT		100
#define DEF_MSG_ALL			MAX_READVAL_CNT
/** 
 *  @brief READ_VAL_LIST : 
 *	nifo_msgq_read에서 읽은 struct 관리를 위한 구조체
 *
 *	@see nifo.h 
 *
 *  @note 		interface 
 */
typedef struct _st_Read_Val_List {
	READ_VAL	READVAL[MAX_READVAL_CNT];	
} READ_VAL_LIST;
#define STG_DEF_READ_VAL_LIST		106		/* Hex( 0x6a ) */
#define READ_VAL_LIST_SIZE sizeof(READ_VAL_LIST)


#if defined(COMMERCIALLOG)
#define			DEF_MNIC_HEADROOM_SIZE		1024
#define			DEF_MNIC_MEMNODEBODY_SIZE	(4 * 1024)
#define 		DEF_MNIC_MEMNODETOT_CNT		(100 * 10000)

#define         DEF_HEADROOM_SIZE           1024
#define         DEF_MEMNODEBODY_SIZE        (7 * 1024)
#define         DEF_MEMNODETOT_CNT          (5 * 10000)
#endif

#if !defined(COMMERCIALLOG)
#define			DEF_MNIC_HEADROOM_SIZE		1024
#define			DEF_MNIC_MEMNODEBODY_SIZE	(4 * 1024)
#define 		DEF_MNIC_MEMNODETOT_CNT		(100 * 10000)

#define         DEF_HEADROOM_SIZE           1024
#define         DEF_MEMNODEBODY_SIZE        (7 * 1024)
#define         DEF_MEMNODETOT_CNT          (5 * 10000)
#endif

/**
      config file read define (nifo, cifo, gifo)
	      */

#ifndef __CONF_DIFO_H__
#define __CONF_DIFO_H_

//#define DEF_CIFO_CONF_FILE         	DATA_PATH"/cifo.conf"
//#define DEF_GIFO_CONF_FILE          DATA_PATH"/gifo.conf"
//#define DEF_NIFO_ZONE_CONF_FILE     DATA_PATH"/nifo_zone.conf"
//#define DEF_CIFO_CONF_FILE         	DATA_PATH"cifo.conf"
//#define DEF_GIFO_CONF_FILE          DATA_PATH"gifo.conf"
//#define DEF_NIFO_ZONE_CONF_FILE     DATA_PATH"nifo_zone.conf"

#define DEF_CHANCONF_MEMBERS 9
#define DEF_ZONECONF_MEMBERS 5

#define DEF_CHECK_FLAG_ZERO  0
#define DEF_CHECK_FLAG_MORE  1
#define DEF_CHECK_FLAG_BOOL  2

#define DEF_ERR_NOTNUM      -2
#define DEF_ERR_LOWZERO     -3
#define DEF_ERR_LOWONE      -4
#define DEF_ERR_NOTBOOL     -5
#define DEF_ERR_NOTFLAG     -6
#define DEF_ERR_EXIST       -7
#define DEF_ERR_FORM        -8
#define DEF_ERR_MEMBERS     -9
#define DEF_ERR_NOT_EXIST   -10
#define DEF_MAX_OVERFLOW    -11
#define DEF_ERR_ORDER       -12

#define DEBUG_CHANNEL_CONF  "CHANNEL CONFIG"
#define DEBUG_CIFO_CONF     "CIFO CONFIG"
#define DEBUG_GROUP_CONF    "GROUP CONFIG"
#define DEBUG_W_MET_CONF    "WRITE METRIX CONFIG"
#define DEBUG_R_MET_CONF    "READ METRIX CONFIG"
#define DEBUG_NIFO_CONF     "NIFO CONFIG"
#define DEBUG_ZONE_CONF     "ZONE CONFIG"

#define INVALID_ID          0xFFFFFFFF
#endif


/**
 *  nifo project : External Functions.
 */
#define nifo_node_head_init(infoptr, ptr)				CINIT_LIST_HEAD(infoptr, ptr)
/**
 *	@note	nifo_node_for_each_start, nifo_node_for_each_end 사용시 현재는 clist_head의 
 *			포인트를 넘기는데 향후에는 node의 포인트를 넘기는 방식으로 변경 요망 
 */
#define nifo_node_for_each_start(infoptr, pos, head)	clist_for_each_start(infoptr, pos, head)
#define nifo_node_for_each_end(infoptr, pos, head)		clist_for_each_end(infoptr, pos, head)
#define nifo_entry(ptr, type, member)					clist_entry(ptr, type, member)
#define nifo_offset(infoptr, ptr)						clisto_offset(infoptr, ptr)
#define nifo_ptr(infoptr, offset)						(U8 *)clisto_ptr(infoptr, offset)
#define nifo_free_len(common)	(((NIFO *)common)->maxoffset - ((NIFO *)common)->lastoffset)

extern int nifo_dump_DebugString(char *debug_str, char *s, int len);
extern OFFSET nifo_get_offset_node(stMEMSINFO *pMEMSINFO, U8 *ptr);
extern S32 nifo_node_check(stMEMSINFO *pMEMSINFO, OFFSET offset);
extern S32 nifo_msgq_init(U32 uiMsgqKey);
//extern stMEMSINFO *nifo_init(U32 uiShmKey, U32 uiSemKey, U8 *pDbgStr, S32 processID, U32 uiHeadRoomSize, U32 uiMemNodeBodySize, U32 uiMemNodeTotCnt);
extern stMEMSINFO *nifo_init(st_MEMSCONF *pMEMSCONF, U8 *pDbgStr, S32 processID);
extern stMEMSINFO *nifo_init_zone(U8 *pDbgStr, S32 processID, S8 *confFile);
extern U8 *nifo_node_alloc(stMEMSINFO *pstMEMSINFO);
//extern U8 *nifo_node_alloc(stMEMSINFO *pstMEMSINFO, U32 uiZoneID);
extern void nifo_node_link_cont_prev(stMEMSINFO *pstMEMSINFO, U8 *pHead, U8 *pNew);
extern void nifo_node_link_cont_next(stMEMSINFO *pstMEMSINFO, U8 *pHead, U8 *pNew);
extern void nifo_node_link_nont_prev(stMEMSINFO *pstMEMSINFO, U8 *pHead, U8 *pNew);
extern void nifo_node_link_nont_next(stMEMSINFO *pstMEMSINFO, U8 *pHead, U8 *pNew);
extern void nifo_splice_nont(stMEMSINFO *pMEMSINFO, U8 *pLIST, U8 *pHEAD);
extern void nifo_splice_cont(stMEMSINFO *pMEMSINFO, U8 *pLIST, U8 *pHEAD);
extern U8 *nifo_tlv_alloc(stMEMSINFO *pMEMSINFO, U8 *pNode, U32 type, U32 len, S32 memsetFlag);
//extern S32 nifo_msg_write(stMEMSINFO *pstMEMSINFO, U32 uiMsgqID, U8 *pNode, st_MsgQ *pstMsgQ);
//extern OFFSET nifo_msg_read(stMEMSINFO *pstMEMSINFO, U32 uiMsgqID, st_MsgQ *pstMsgQ);
extern S32 nifo_msg_write(stMEMSINFO *pstMEMSINFO, U32 uiMsgqID, U8 *pNode);
extern OFFSET nifo_msg_read(stMEMSINFO *pstMEMSINFO, U32 uiMsgqID, READ_VAL_LIST *pREADVALLIST);
extern U8 *nifo_get_value(stMEMSINFO *pMEMSINFO, U32 type, OFFSET offset);
extern U8 *nifo_get_tlv(stMEMSINFO *pMEMSINFO, U32 type, OFFSET offset);
extern S32 nifo_get_point_cont(stMEMSINFO *pstMEMSINFO, READ_VAL_LIST *pREADVALLIST, OFFSET offset);
extern S32 nifo_get_point_all(stMEMSINFO *pstMEMSINFO, READ_VAL_LIST *pREADVALLIST, OFFSET offset);
extern S32 nifo_get_tlv_all(stMEMSINFO *pstMEMSINFO, OFFSET offset, S32 (*exec_func)(U32 type, U32 len, U8 *data, S32 memflag, void *out), void *out);
extern S32 nifo_read_tlv_cont(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 *type, U32 *len, U8 **value, S32 *ismalloc, U8 **nexttlv);
extern S32 nifo_copy_tlv_cont(stMEMSINFO *pMEMSINFO, U32 type, U32 len, U8 *value, U8 *node);
extern void nifo_node_unlink_nont(stMEMSINFO *pstMEMSINFO, U8 *pDel);
extern void nifo_node_unlink_cont(stMEMSINFO *pstMEMSINFO, U8 *pDel);
extern S32 nifo_node_free(stMEMSINFO *pstMEMSINFO, U8 *pFree);
extern U8 *nifo_cont_delete(stMEMSINFO *pMEMSINFO, U8 *pDel);
extern void nifo_node_delete(stMEMSINFO *pMEMSINFO, U8 *pDel);
extern void nifo_print_nont(stMEMSINFO *pstMEMSINFO, U8 *p, void (*print_func)(stMEMSINFO *pmem, U8 *pnode, U8 *str), U8 *PrefixStr);
extern void nifo_print_cont(stMEMSINFO *pstMEMSINFO, U8 *p, void (*print_func)(stMEMSINFO *pmem, U8 *pnode, U8 *str), U8 *PrefixStr);
extern void nifo_print_node(stMEMSINFO *pstMEMSINFO, U8 *p);
extern void nifo_draw_all(S8 *filename, S8 *labelname, stMEMSINFO *pstMEMSINFO);

/* nifo zone config function*/
extern S32 nifo_zone_conf_init(st_MEMSCONF *pMEMSCONF, S8 *confFile);
extern S32 defaultCheck(S8 *s, U32 *uiValue, S32 dFlag);
extern void errorPrint(S32 errn, S8* grp, char *var, U32 line);
extern S8 *trim(S8 *s);
extern void nifo_conf_print(st_MEMSCONF *pMEMSCONF);

/**
 *	$Log: nifo.h,v $
 *	Revision 1.3  2011/04/27 14:30:40  jjinri
 *	*** empty log message ***
 *	
 *	Revision 1.2  2011/04/26 09:08:15  jjinri
 *	nifo_zone
 *	
 *	Revision 1.4  2011/03/28 06:59:01  jjinri
 *	*** empty log message ***
 *	
 *	Revision 1.3  2011/03/28 06:35:01  jjinri
 *	*** empty log message ***
 *	
 *	Revision 1.4  2011/01/21 02:36:56  swpark
 *	LOG classfy
 *	
 *	Revision 1.3  2011/01/20 02:31:16  jjinri
 *	FILTERING 방식 변경. PROTO, S_MNG, RADIUS
 *	
 *	Revision 1.2  2011/01/11 05:45:19  swpark
 *	config init function config file name parameter add
 *	
 *	Revision 1.7  2011/01/06 09:07:10  swpark
 *	config file parameter addINC/cifo.h
 *	
 *	Revision 1.6  2011/01/03 07:20:39  upst_cvs
 *	DATA_PATH
 *	
 *	Revision 1.5  2010/12/29 06:57:43  swpark
 *	zone matrix add
 *	
 *	Revision 1.4  2010/12/23 08:04:19  swpark
 *	*** empty log message ***
 *	
 *	Revision 1.3  2010/12/23 07:59:55  swpark
 *	MEMS_SHM_SIZE: 4GB-1
 *	
 *	Revision 1.2  2010/12/23 01:30:50  swpark
 *	nifo zone add
 *	
 *	Revision 1.1.1.1  2010/11/30 05:22:39  uamyd
 *	64bit WNTAM, WGTAM add starting
 *	
 *	Revision 1.8  2010/06/01 05:22:10  dark264sh
 *	*** empty log message ***
 *	
 *	Revision 1.7  2010/05/22 07:52:05  pkg
 *	*** empty log message ***
 *	
 *	Revision 1.6  2010/05/22 07:03:15  dark264sh
 *	HSORT nifo 관련 define 변경
 *	
 *	Revision 1.5  2010/05/19 12:15:57  pkg
 *	NIFO 작업중
 *	
 *	Revision 1.4  2010/05/19 11:52:54  dark264sh
 *	nifo st_MsgQ 처리 추가
 *	
 *	Revision 1.3  2010/05/16 07:44:49  dark264sh
 *	nifo 최신 버전으로 변경
 *	
 *	Revision 1.2  2009/12/07 06:29:04  dark264sh
 *	nifo msgsnd, msgrcv 64 bits 처리
 *	
 *	Revision 1.1.1.1  2009/11/24 06:44:25  pkg
 *	WNTAS 64bits
 *	
 *	Revision 1.2  2008/09/29 04:50:10  dark264sh
 *	*** empty log message ***
 *	
 *	Revision 1.1  2008/09/29 02:23:15  dark264sh
 *	multi tcp 처리
 *	
 *	Revision 1.2  2008/02/22 08:20:03  pkg
 *	*** empty log message ***
 *	
 *	Revision 1.1  2008/01/11 12:09:01  pkg
 *	import two-step by uamyd
 *	
 **/
#endif
