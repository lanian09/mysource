#ifndef	__nifo_h__
#define	__nifo_h__
/**		file  nifo.h
 *      - hash header file
 *
 *     Copyright (c) 2006~ by Upresto Inc, Korea
 *     All rights reserved
 *
 *		$Id: nifo.h,v 1.3 2011/08/19 06:35:02 dcham Exp $
 * 
 *     @Author      $Author: dcham $
 *     @version     $Revision: 1.3 $
 *     @date        $Date: 2011/08/19 06:35:02 $
 *     @warning     $type???�� ���ǵ� �͵鸸 ��밡��
 *     @ref         nifo.h
 *     @todo        Makefile�� ������
 *
 *     @section     Intro(�Ұ�)
 *      - hash header file
 *
 *     @section     Requirement
 *      @li ��Ģ�� Ʋ�� ���� ã���ּ���.
 *
 **/


/* �ʿ��� header file include */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<netinet/in.h>
#include	<time.h>

#include <commdef.h>
#include <typedef.h>

#include "clisto.h"
#include "mems.h"

#define NIFO_ZONE_0			0
#define NIFO_ZONE_1			1
#define NIFO_ZONE_2			2
#define NIFO_ZONE_3			3
#define NIFO_ZONE_4			4
#define NIFO_ZONE_5			5
#define NIFO_ZONE_6			6
#define NIFO_ZONE_7			7
#define NIFO_ZONE_8			8
#define NIFO_ZONE_9			9

#define MAX_ZONE_CNT		2
#define MAX_ZONE_NUM		(MAX_ZONE_CNT - 1)

#define S_SSHM_NIFO			15000 
#define S_SEMA_NIFO			11000 

/** 
 *  @brief st_NIFO_NODE_INFO : 
 *	Shared Memory, Semaphore, Message Queue, Offset Linked List�� �̿��� Interface
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
	S32				cnt;			/**< TLV ���� */
	OFFSET			lastoffset;		/**< ��� ������ OFFSET */
	OFFSET			maxoffset;		/**< ��� ������ �ִ� OFFSET */
} NIFO;
#define STG_DEF_NIFO		101		/* Hex( 531490 ) */
#define NIFO_SIZE sizeof(NIFO)


/**
 * @brief st_TLV : Node�� �⺻ ���� ����
 *
 *
 * @see l4.h
 *
 * @note	��� ����ü �տ� �ٴ´�. 
 *
 */
typedef struct _st_TLV {
	U32				type;				/**< ����ü ��ȣ */
	U32				len;				/**< Value's Length */
} TLV;
#define STG_DEF_TLV		102		/* Hex( 531490 ) */
#define TLV_SIZE sizeof(TLV)


/** 
 *  @brief st_MsgQ : 
 *	Message Queue ���� header
 *
 *	@see nifo.h 
 *
 *  @note 		interface 
 */
typedef struct _st_MsgQ_NIFO {
	OFFSET		mtype;		/**< st_MsgQSub�� cast�ؼ� ��� */
	S32			msgqid;		/**< message queue id */
	U16			len;		/**< data size */
	U16			procid;		/**< process id */
} st_MsgQ_NIFO;
#define STG_DEF_st_MsgQ_NIFO		103		/* Hex( 531490 ) */
#define st_MsgQ_NIFO_SIZE sizeof(st_MsgQ_NIFO)


#define DEF_MSGQ_SVC		1000
#define DEF_MSGQ_SYS		5000

/** 
 *  @brief st_MsgQSub : 
 *	Message Queue mtype ��
 *
 *	@see nifo.h 
 *
 *  @note 		interface 
 */
typedef struct _st_MsgQSub_NIFO {
/**< <TAG_DEFINE:DEF_MSGQ_TYPE>
*	 DEF_MSGQ_SVC		1000
*	 DEF_MSGQ_SYS		5000
*/
	U16 	type;		/**< �޽��� Type (SVC, SYS ����) */	
	U8			svcid;								/**< SVC ���� */
	U8			msgid;								/**< MSG ���� */
} st_MsgQSub_NIFO;
#define STG_DEF_st_MsgQSub_NIFO		104		/* Hex( 531490 ) */
#define st_MsgQSub_NIFO_SIZE sizeof(st_MsgQSub_NIFO)


#define DEF_MEMSET_OFF		0
#define DEF_MEMSET_ON		1
#define DEF_READ_ON			1
#define DEF_READ_OFF		2
#define DEF_READ_EMPTY		0
#define DEF_READ_MALLOC		1
#define DEF_READ_ORIGIN		2
/** 
 *  @brief REAL_VAL : 
 *	nifo_msgq_read���� ���� struct ������ ���� ����ü
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
	S8 	init;		/**< ���� ��������� �Ǵ��ϴ� Flag */
/**< <TAG_DEFINE:DEF_READ_VAL_MEM>
*	 DEF_READ_EMPTY		0
*	 DEF_READ_MALLOC		1
*	 DEF_READ_ORIGIN		2
*/
	S8 	memtype;	/**< ���ο� �޸𸮸� �Ҵ��ϴ��� ǥ�� */
	S16			len;									/**< pVal�� ������ */
	U8			*pVal;									/**< ����ü �� ����Ʈ */
} READ_VAL;
#define STG_DEF_READ_VAL		105		/* Hex( 531490 ) */
#define READ_VAL_SIZE sizeof(READ_VAL)


#define MAX_READVAL_CNT		100
#define DEF_MSG_ALL			MAX_READVAL_CNT
/** 
 *  @brief READ_VAL_LIST : 
 *	nifo_msgq_read���� ���� struct ������ ���� ����ü
 *
 *	@see nifo.h 
 *
 *  @note 		interface 
 */
typedef struct _st_Read_Val_List {
	READ_VAL	READVAL[MAX_READVAL_CNT];	
} READ_VAL_LIST;
#define STG_DEF_READ_VAL_LIST		106		/* Hex( 531490 ) */
#define READ_VAL_LIST_SIZE sizeof(READ_VAL_LIST)

#if defined(COMMERCIALLOG)
#define			DEF_HEADROOM_SIZE			1024
#define			DEF_MEMNODEBODY_SIZE		(4 * 1024)
#define			DEF_MEMNODETOT_CNT			(200 * 10000)

#define			DEF_TAM_HEADROOM_SIZE		1024
#define			DEF_TAM_MEMNODEBODY_SIZE	(8 * 1024)
#define			DEF_TAM_MEMNODETOT_CNT		(100 * 10000)

#define			DEF_CAPD_HEADROOM_SIZE		1024
#define			DEF_CAPD_MEMNODEBODY_SIZE	(8 * 1024)
#define			DEF_CAPD_MEMNODETOT_CNT		(100 * 10000)
#endif

#if !defined(COMMERCIALLOG)
#define			DEF_HEADROOM_SIZE		1024
#define			DEF_MEMNODEBODY_SIZE		(4 * 1024)
#define 		DEF_MEMNODETOT_CNT			(4 * 10000)

#define			DEF_TAM_HEADROOM_SIZE		1024
#define			DEF_TAM_MEMNODEBODY_SIZE	(8 * 1024)
#define 		DEF_TAM_MEMNODETOT_CNT		(100 * 10000)

#define			DEF_CAPD_HEADROOM_SIZE		1024
#define			DEF_CAPD_MEMNODEBODY_SIZE	(4 * 1024)
#define 		DEF_CAPD_MEMNODETOT_CNT		(8 * 10000)
#endif

/**
    config file read define (nifo, cifo, gifo)
	*/

#ifndef __CONF_NCG_FO_H__
#define __CONF_NCG_FO_H_

//#define START_PATH          		"/home/swpark/DIFO/"
//#define START_PATH          		"/DIFO/"
//#define DATA_PATH                   START_PATH"DATA/"
//#define DEF_CIFO_CONF_FILE          DATA_PATH"cifo.conf"
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

#define	INVALID_ID          0xFFFFFFFF
#endif

/**
 *  nifo project : External Functions.
 */
#define nifo_node_head_init(infoptr, ptr)				CINIT_LIST_HEAD(infoptr, ptr)
/**
 *	@note	nifo_node_for_each_start, nifo_node_for_each_end ���� ����� clist_head�� 
 *			����Ʈ�� �ѱ�µ� ���Ŀ��� node�� ����Ʈ�� �ѱ�� ������� ���� ��� 
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
extern stMEMSINFO *nifo_init(st_MEMSCONF *pMEMSCONF, U8 *pDbgStr, S32 processID);
extern stMEMSINFO *nifo_init_zone(U8 *pDbgStr, S32 processID, S8 *confFile);
extern S32 nifo_conf_check(st_MEMSCONF *pMEMSCONF);
extern U8 *nifo_node_alloc(stMEMSINFO *pstMEMSINFO);
extern void nifo_node_link_cont_prev(stMEMSINFO *pstMEMSINFO, U8 *pHead, U8 *pNew);
extern void nifo_node_link_cont_next(stMEMSINFO *pstMEMSINFO, U8 *pHead, U8 *pNew);
extern void nifo_node_link_nont_prev(stMEMSINFO *pstMEMSINFO, U8 *pHead, U8 *pNew);
extern void nifo_node_link_nont_next(stMEMSINFO *pstMEMSINFO, U8 *pHead, U8 *pNew);
extern U8 *nifo_tlv_alloc(stMEMSINFO *pMEMSINFO, U8 *pNode, U32 type, U32 len, S32 memsetFlag);
extern S32 nifo_msg_write(stMEMSINFO *pstMEMSINFO, U32 uiMsgqID, U8 *pNode);
extern OFFSET nifo_msg_read(stMEMSINFO *pstMEMSINFO, U32 uiMsgqID, READ_VAL_LIST *pREADVALLIST);
extern U8 *nifo_get_value(stMEMSINFO *pMEMSINFO, U32 type, OFFSET offset);
extern U8 *nifo_get_tlv(stMEMSINFO *pMEMSINFO, U32 type, OFFSET offset);
extern S32 nifo_get_point_cont(stMEMSINFO *pstMEMSINFO, READ_VAL_LIST *pREADVALLIST, OFFSET offset);
extern S32 nifo_get_point_all(stMEMSINFO *pstMEMSINFO, READ_VAL_LIST *pREADVALLIST, OFFSET offset);
extern S32 nifo_get_tlv_all(stMEMSINFO *pstMEMSINFO, OFFSET offset, S32 (*exec_func)(U32 type, U32 len, U8 *data, S32 memflag, void *out), void *out);
extern S32 nifo_read_tlv_cont(stMEMSINFO *pMEMSINFO, U8 *pHEAD, U32 *type, U32 *len, U8 **value, S32 *ismalloc, U8 **nexttlv);
extern S32 nifo_copy_tlv_cont(stMEMSINFO *pMEMSINFO, U32 type, U32 len, U8 *value, U8 *node);
extern void nifo_splice_nont(stMEMSINFO *pMEMSINFO, U8 *pLIST, U8 *pHEAD);
extern void nifo_splice_cont(stMEMSINFO *pMEMSINFO, U8 *pLIST, U8 *pHEAD);
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

/* Define.  DEF_NUM(type definition number)
*/
#define		stFlat_st_MsgQ_NIFO_DEF_NUM					103			/* Hex ( 794930 ) */
#define		stFlat_TLV_DEF_NUM							102			/* Hex ( 793e20 ) */
#define		st_MsgQSub_NIFO_DEF_NUM						104			/* Hex ( 795540 ) */
#define		stFlat_st_MsgQSub_NIFO_DEF_NUM				104			/* Hex ( 795550 ) */
#define		stFlat_NIFO_DEF_NUM							101			/* Hex ( 791580 ) */
#define		NIFO_DEF_NUM								101			/* Hex ( 791570 ) */
#define		READ_VAL_DEF_NUM							105			/* Hex ( 7964e0 ) */
#define		stFlat_READ_VAL_LIST_DEF_NUM				106			/* Hex ( 796950 ) */
#define		TLV_DEF_NUM									102			/* Hex ( 793e10 ) */
#define		READ_VAL_LIST_DEF_NUM						106			/* Hex ( 796940 ) */
#define		st_MsgQ_NIFO_DEF_NUM						103			/* Hex ( 794920 ) */
#define		stFlat_READ_VAL_DEF_NUM						105			/* Hex ( 7964f0 ) */




/* Define.  MEMBER_CNT(struct���� member���Ǽ� : flat����)
*/
#define		NIFO_MEMBER_CNT								7
#define		READ_VAL_MEMBER_CNT							4
#define		READ_VAL_LIST_MEMBER_CNT					1
#define		Short_NIFO_MEMBER_CNT						7
#define		Short_READ_VAL_MEMBER_CNT					3
#define		Short_READ_VAL_LIST_MEMBER_CNT				1
#define		Short_TLV_MEMBER_CNT						2
#define		Short_st_MsgQSub_NIFO_MEMBER_CNT			3
#define		Short_st_MsgQ_NIFO_MEMBER_CNT				4
#define		TLV_MEMBER_CNT								2
#define		stFlat_NIFO_MEMBER_CNT						7
#define		stFlat_READ_VAL_MEMBER_CNT					4
#define		stFlat_READ_VAL_LIST_MEMBER_CNT				1
#define		stFlat_Short_NIFO_MEMBER_CNT				7
#define		stFlat_Short_READ_VAL_MEMBER_CNT			3
#define		stFlat_Short_READ_VAL_LIST_MEMBER_CNT		1
#define		stFlat_Short_TLV_MEMBER_CNT					2
#define		stFlat_Short_st_MsgQSub_NIFO_MEMBER_CNT		3
#define		stFlat_Short_st_MsgQ_NIFO_MEMBER_CNT		4
#define		stFlat_TLV_MEMBER_CNT						2
#define		stFlat_st_MsgQSub_NIFO_MEMBER_CNT			3
#define		stFlat_st_MsgQ_NIFO_MEMBER_CNT				4
#define		st_MsgQSub_NIFO_MEMBER_CNT					3
#define		st_MsgQ_NIFO_MEMBER_CNT						4




/* Extern Function Define.
*/
extern void NIFO_CILOG(FILE *fp, NIFO *pthis);
extern void NIFO_Dec(NIFO *pstTo , NIFO *pstFrom);
extern void NIFO_Enc(NIFO *pstTo , NIFO *pstFrom);
extern void NIFO_Prt(S8 *pcPrtPrefixStr, NIFO *pthis);
extern void READ_VAL_CILOG(FILE *fp, READ_VAL *pthis);
extern void READ_VAL_Dec(READ_VAL *pstTo , READ_VAL *pstFrom);
extern void READ_VAL_Enc(READ_VAL *pstTo , READ_VAL *pstFrom);
extern void READ_VAL_LIST_CILOG(FILE *fp, READ_VAL_LIST *pthis);
extern void READ_VAL_LIST_Dec(READ_VAL_LIST *pstTo , READ_VAL_LIST *pstFrom);
extern void READ_VAL_LIST_Enc(READ_VAL_LIST *pstTo , READ_VAL_LIST *pstFrom);
extern void READ_VAL_LIST_Prt(S8 *pcPrtPrefixStr, READ_VAL_LIST *pthis);
extern void READ_VAL_Prt(S8 *pcPrtPrefixStr, READ_VAL *pthis);
extern void TLV_CILOG(FILE *fp, TLV *pthis);
extern void TLV_Dec(TLV *pstTo , TLV *pstFrom);
extern void TLV_Enc(TLV *pstTo , TLV *pstFrom);
extern void TLV_Prt(S8 *pcPrtPrefixStr, TLV *pthis);
extern void st_MsgQSub_NIFO_CILOG(FILE *fp, st_MsgQSub_NIFO *pthis);
extern void st_MsgQSub_NIFO_Dec(st_MsgQSub_NIFO *pstTo , st_MsgQSub_NIFO *pstFrom);
extern void st_MsgQSub_NIFO_Enc(st_MsgQSub_NIFO *pstTo , st_MsgQSub_NIFO *pstFrom);
extern void st_MsgQSub_NIFO_Prt(S8 *pcPrtPrefixStr, st_MsgQSub_NIFO *pthis);
extern void st_MsgQ_NIFO_CILOG(FILE *fp, st_MsgQ_NIFO *pthis);
extern void st_MsgQ_NIFO_Dec(st_MsgQ_NIFO *pstTo , st_MsgQ_NIFO *pstFrom);
extern void st_MsgQ_NIFO_Enc(st_MsgQ_NIFO *pstTo , st_MsgQ_NIFO *pstFrom);
extern void st_MsgQ_NIFO_Prt(S8 *pcPrtPrefixStr, st_MsgQ_NIFO *pthis);

#pragma pack()

/** file : nifo.h
 *     $Log: nifo.h,v $
 *     Revision 1.3  2011/08/19 06:35:02  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/08/18 04:58:28  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/07/26 06:13:04  dhkim
 *     *** empty log message ***
 *
 *     Revision 1.3  2011/03/24 07:25:04  dark264sh
 *     nifo, gifo: check validatoin
 *
 *     Revision 1.2  2011/01/21 01:39:36  swpark
 *     LOG classfy
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.10  2011/01/05 08:57:26  swpark
 *     PATH delete
 *
 *     Revision 1.9  2010/12/29 07:27:58  swpark
 *     confile add
 *
 *     Revision 1.8  2010/12/28 07:24:22  swpark
 *     zone matrix add
 *
 *     Revision 1.7  2010/12/13 05:43:41  swpark
 *     START_PATH add
 *
 *     Revision 1.6  2010/12/10 06:43:51  swpark
 *     START_PATH del
 *
 *     Revision 1.5  2010/12/10 06:31:43  swpark
 *     DATA_PATH update
 *
 *     Revision 1.4  2010/12/10 00:45:23  swpark
 *     INVALID ID -> nifo.h
 *
 *     Revision 1.3  2010/12/09 09:25:19  swpark
 *     config define add
 *
 *     Revision 1.2  2010/12/09 08:32:18  swpark
 *     config read func add
 *
 *     Revision 1.1  2010/12/02 12:50:37  upst_cvs
 *     2010.1202 commit start
 *
 *     Revision 1.9  2010/11/25 08:36:04  upst_cvs
 *     ..
 *
 *     Revision 1.8  2010/11/24 17:37:00  upst_cvs
 *     ..
 *
 *     Revision 1.7  2010/11/23 13:30:30  upst_cvs
 *     size
 *
 *     Revision 1.6  2010/11/23 05:07:40  upst_cvs
 *     1122_ GURO work
 *
 *     Revision 1.5  2010/11/22 04:31:14  upst_cvs
 *     NODE CONT
 *
 *     Revision 1.4  2010/11/01 02:48:13  upst_cvs
 *     ..
 *
 *     Revision 1.3  2010/10/19 02:01:28  upst_cvs
 *     ..
 *
 *     Revision 1.2  2010/10/18 12:14:13  upst_cvs
 *     ..
 *
 *     Revision 1.1.1.1  2010/10/15 06:54:37  upst_cvs
 *     WTAS2
 *
 *     Revision 1.1  2009/06/10 16:45:50  dqms
 *     *** empty log message ***
 *
 *     Revision 1.1.1.1  2009/05/26 02:13:28  dqms
 *     Init TAF_RPPI
 *
 *     Revision 1.2  2008/11/17 09:00:04  dark264sh
 *     64bits �۾�
 *
 *     Revision 1.1.1.1  2008/06/09 08:17:19  jsyoon
 *     WATAS3 PROJECT START
 *
 *     Revision 1.1  2007/08/21 12:22:38  dark264sh
 *     no message
 *
 *     */
#endif	/* __nifo_h__*/
