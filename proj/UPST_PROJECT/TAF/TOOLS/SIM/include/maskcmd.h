/**********************************************************************
*
* Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
* $Id: maskcmd.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		maskcmd.h
* DESCRIPTION:	Mask Command Module
*
* HISTORY:
*
**********************************************************************/
#ifndef MASK_CMD_H
#define MASK_CMD_H maskcmd.h

#include "cfgwcmd.h"

typedef struct mask_cmd MASK_CMD;
typedef MASK_CMD *PMASK_CMD;

struct mask_cmd
{
    uint32_t    *pwCmd;

    PCFG_WCMD   pParent;
    
    PMASK_CMD   (*New)();
    void        (*Delete)(PMASK_CMD pThis);
    
    void        (*MakeCmd)(PMASK_CMD pThis, uint32_t wIfc, uint32_t wTimeslot, uint32_t wMask);
    void        (*MakeChainCmd)(PMASK_CMD pThis, 
                                uint32_t wMask,
                                bool bChaining, 
                                uint32_t wPrevChanNum);
    uint32_t    (*VerifyCmd)(PMASK_CMD pThis, PCONFIG_MGR pCfgMgr, uint32_t wDirection);
    void        (*WriteCmd)(PMASK_CMD pThis, PCONFIG_MGR pCfgMgr, uint32_t wDirection);
    void        (*SetAddr)(PMASK_CMD pThis, uint32_t wIfcNum, uint32_t wTimeSlot, uint32_t wMsgType);
    void        (*SetChainAddr)(PMASK_CMD pThis, uint32_t wCmdType, uint32_t wPrevChanNum);
    void        (*SetData)(PMASK_CMD pThis, uint32_t wData);
    void        (*SetCmd)(PMASK_CMD pThis, uint32_t wCmd);
    uint32_t    (*GetAddr)(PMASK_CMD pThis);
    uint32_t    (*GetData)(PMASK_CMD pThis);
    uint32_t    (*GetCmd)(PMASK_CMD pThis);
    uint32_t    (*GetWriteCmd)(PMASK_CMD pThis);
};


PMASK_CMD MCNew();

#ifdef _WIN32
#define inline
#endif
static inline void SetMCChainMaskBit(PMASK_CMD pThis)
{
    *pThis->pwCmd |= CFG_WRITE_CMD_DATA_CHAIN_MASK_FLAG;
}

static inline void ClrMCChainMaskBit(PMASK_CMD pThis)
{
    *pThis->pwCmd &= ~CFG_WRITE_CMD_DATA_CHAIN_MASK_FLAG;
}

static inline void SetMCChainAddrBit(PMASK_CMD pThis)
{
    *pThis->pwCmd |= CFG_WRITE_CMD_DATA_CHAIN_ADDR_FLAG;
}

static inline void SetMCWriteEntryBit(PMASK_CMD pThis)
{
    *pThis->pwCmd |= CFG_WRITE_CMD_DATA_WRITE_ENTRY_MASK;
}
#ifdef _WIN32
#undef inline
#endif

#endif /* MASK_CMD_H */
