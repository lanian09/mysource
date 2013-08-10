#ifndef __gifo_h__
#define __gifo_h__
/**		file  gifo.h
 *		- header file
 *
 *		Copyright (c) 2006~ by Upresto Inc, Korea
 *		All rights reserved
 *
 *		$Id: gifo.h,v 1.3 2011/08/19 06:35:02 dcham Exp $
 * 
 *		@Author		$Author: dcham $
 *		@version	$Revision: 1.3 $
 *		@date		$Date: 2011/08/19 06:35:02 $
 *		@warning	$type
 *		@ref		gifo.h
 *		@todo		이제 시작이다. 
 *
 *		@section	Intro
 *		- header file
 *
 *		@section	Requirement
 *		@li 
 *
 **/

#include <typedef.h>

#include "nifo.h"
#include "cifo.h"

/**
 *	@brief  GROUP :
 *	개별 GROUP 관리 구조체 header
 *	
 * 	@see gifo.h
 *
 */
typedef struct __GROUP__ 
{
	U32			uiGrID;
	U32			uiChCnt;
	U32			uiChID[MAX_CHAN_CNT];
} st_GROUP;
#define	st_GROUP_SIZE sizeof(st_GROUP)

/**
 *	@brief  GIFO :
 *	GIFO 최상위 헤더 
 *	
 * 	@see gifo.h
 *
 */
//#define 	MAX_SEQ_PROC_NUM	1000  --> mems.h
#define		MAX_GROUP_CNT		100
//#define		INVALID_ID			0xFFFFFFFF
typedef struct __GIFO__
{
	U32 		uiGrCnt;
	st_GROUP	stGROUP[MAX_GROUP_CNT];		
	U32			uiMatrixChID[MAX_SEQ_PROC_NUM*MAX_SEQ_PROC_NUM];		
	U32			uiMatrixGrID[MAX_SEQ_PROC_NUM];		
} stGIFO;
#define	stGIFO_SIZE sizeof(stGIFO)

/**
 *	difo project : gifo external function
 */

extern stCIFO* 	gifo_init_group(S8 *cifo_confFile, S8 *gifo_confFile);
extern stCIFO* 	gifo_init(stGIFO *pstGIFO, st_CIFOCONF *pstCIFOCONF);
extern int		gifo_write(stMEMSINFO *pstMEMSINFO, stCIFO *pstCIFO, U32 uiWProcID, U32 uiRProcID, OFFSET offset);
extern OFFSET	gifo_read(stMEMSINFO *pstMEMSINFO, stCIFO *pstCIFO, U32 uiRProcID);
extern void		gifo_print(stCIFO *pstCIFO);
extern void		gifo_print_channel(stCIFO *pstCIFO, U32 uiGrID);
extern S32 		gifo_conf_init(stGIFO *pGIFO, st_CIFOCONF *pCIFOCONF, S8 *confFile);
extern S32 		gifo_conf_check(stGIFO *pGIFO, st_CIFOCONF *pCIFOCONF);

extern S32		gifo_set_buffcnt(stCIFO *pstCIFO, U32 uiWProcID, U32 uiRProcID, U32 uiBuffCnt, S32 uiRWFlag);
extern U32		gifo_get_buffcnt(stCIFO *pstCIFO, U32 uiWProcID, U32 uiRProcID, S32 uiRWFlag);

extern U32 		gifo_get_channel(stCIFO *pstCIFO, U32 uiWProcID, U32 uiRProcID);
extern void 	gifo_conf_print(stGIFO *pGIFO);

/** file : gifo.h
 *     $Log: gifo.h,v $
 *     Revision 1.3  2011/08/19 06:35:02  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/08/18 04:58:27  dcham
 *     *** empty log message ***
 *
 *     Revision 1.1  2011/07/26 06:13:03  dhkim
 *     *** empty log message ***
 *
 *     Revision 1.3  2011/03/24 07:24:19  dark264sh
 *     nifo, gifo: check validatoin
 *
 *     Revision 1.2  2011/01/21 01:39:35  swpark
 *     LOG classfy
 *
 *     Revision 1.1.1.1  2011/01/11 01:33:02  jjinri
 *     DIFO
 *
 *     Revision 1.12  2011/01/05 08:57:24  swpark
 *     PATH delete
 *
 *     Revision 1.11  2010/12/29 07:28:32  swpark
 *     confile add
 *
 *     Revision 1.10  2010/12/28 08:39:59  swpark
 *     gifo_get_procid delete
 *
 *     Revision 1.9  2010/12/28 07:24:04  swpark
 *     buffcnt api add
 *
 *     Revision 1.8  2010/12/10 00:45:16  swpark
 *     INVALID ID -> nifo.h
 *
 *     Revision 1.7  2010/12/09 08:41:20  jjinri
 *     gifo_print
 *
 *     Revision 1.6  2010/12/09 07:11:45  jjinri
 *     gifo_conf_init
 *
 *     Revision 1.5  2010/12/09 06:47:09  swpark
 *     config proto type add
 *
 *     Revision 1.4  2010/12/09 06:42:15  jjinri
 *     INVALID_ID
 *
 *     Revision 1.3  2010/12/09 06:30:42  jjinri
 *     INVALID GROUP ID
 *
 *     Revision 1.2  2010/12/06 08:55:48  swpark
 *      -MAX_GROUP_CNT 100- add
 *
 *     Revision 1.1  2010/12/02 12:50:37  upst_cvs
 *     2010.1202 commit start
 *
 *
 *		*/
#endif /* __gifo_h__ */
