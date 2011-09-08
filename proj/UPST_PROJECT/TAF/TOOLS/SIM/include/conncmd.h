/**********************************************************************
*
* Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
* $Id: conncmd.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		conncmd.h
* DESCRIPTION:	Connection Number Command Module
*
* HISTORY:
*
**********************************************************************/
#ifndef CONNCMD_H
#define CONNCMD_H "connnum.h"

#include "cfgwcmd.h"

typedef struct connnum_cmd CONNNUM_CMD;
typedef CONNNUM_CMD *PCONNNUM_CMD;

struct connnum_cmd
{
    uint32_t    *pwCmd;

    PCFG_WCMD   pParent;
    
    PCONNNUM_CMD    (*New)();
    void            (*Delete)(PCONNNUM_CMD pThis);
    
    void            (*MakeCmd)(PCONNNUM_CMD pThis, 
                               uint32_t wIfc,
                               uint32_t wTimeslot, 
                               uint32_t wChannelNum, 
                               bool bAddConn);
    uint32_t        (*VerifyCmd)(PCONNNUM_CMD pThis, PCONFIG_MGR pCfgMgr, uint32_t wDirection);
    void            (*WriteCmd)(PCONNNUM_CMD pThis, PCONFIG_MGR pCfgMgr, uint32_t wDirection);
    void            (*SetAddr)(PCONNNUM_CMD pThis, 
                               uint32_t wIfcNum, 
                               uint32_t wTimeSlot, 
                               uint32_t wMsgType);
    void            (*SetData)(PCONNNUM_CMD pThis, uint32_t wData);
    void            (*SetCmd)(PCONNNUM_CMD pThis, uint32_t wCmd);
    uint32_t        (*GetAddr)(PCONNNUM_CMD pThis);
    uint32_t        (*GetData)(PCONNNUM_CMD pThis);
    uint32_t        (*GetCmd)(PCONNNUM_CMD pThis);
    uint32_t        (*GetWriteCmd)(PCONNNUM_CMD pThis);
    void            (*MakeChainCmd)(PCONNNUM_CMD pThis, 
                                   uint32_t wChanNum, 
                                   uint32_t wPrevChanNum);
    void            (*SetChainAddr)(PCONNNUM_CMD pThis, 
                                    uint32_t wCmdType, 
                                    uint32_t wPrevChanNum);

};


PCONNNUM_CMD CNCNew();

#ifdef _WIN32
#define inline
#endif

static inline void SetCNCWriteEntryAddConnBit(PCONNNUM_CMD pThis)
{
    *pThis->pwCmd |= CFG_WRITE_CMD_DATA_WRITE_ENTRY_ADD_CONN_MASK;
}

static inline void ClrCNCAddConnBit(PCONNNUM_CMD pThis)
{
    *pThis->pwCmd &= ~CFG_WRITE_CMD_DATA_WRITE_ENTRY_ADD_CONN_MASK;
    *pThis->pwCmd |= CFG_WRITE_CMD_DATA_WRITE_ENTRY_MASK;
}

static inline void SetCNCWriteEntryDeleteConnBit(PCONNNUM_CMD pThis)
{
    ClrCNCAddConnBit(pThis);
    *pThis->pwCmd |= CFG_WRITE_CMD_DATA_WRITE_ENTRY_DELETE_CONN_MASK;
}

static inline void SetCNCWriteEntryAddConnRawBit(PCONNNUM_CMD pThis)
{
    /* Only add a raw connect mask if the add connection bit is set */
    if (*pThis->pwCmd & CFG_WRITE_CMD_DATA_ADD_CONN_MASK)
    {
        *pThis->pwCmd |= CFG_WRITE_CMD_DATA_ADD_CONN_RAW_MASK;
    }
    else
    {
        VERBOSE_LEVEL_4("Did not add the raw connection bit because the add connection bit was not set\n");
    }
}

#ifdef _WIN32
#undef inline
#endif

#endif /* CONNCMD_H */
