/*****************************************************************
 * PACKET BUFFERING ฐüทร Header
*****************************************************************/

#ifndef __CAPD_BFR_H__
#define __CAPD_BFR_H__

#ifdef BUFFERING
#define COLLECTION_MIN          				4
#else 
#define COLLECTION_MIN          				0
#endif  /* BUFFERING */
 
#define COLLECTION_MAX          				24
#define COLLECTION_TIME         				1
#define COLLECTION_MULTIPLY     				2
                                                                                                     
extern unsigned int    Send_Node_Cnt;
extern unsigned int    Collection_Cnt;

#endif /* __CAPD_BFR_H__ */

