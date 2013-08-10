/**********************************************************************
*
* Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
* $Id: cfgwcmd.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		cfgwcmd.h
* DESCRIPTION:	Configuration Write Command Module
*
* HISTORY:
*
**********************************************************************/

#ifndef CFGWCMD_H
#define CFGWCMD_H "cfgwcmd.h"

/* Endace headers. */
#include "dag_platform.h"
#include "dag37t_api.h"

/* DAG 3.7T headers. */
#include "configmgr.h"
#include "global.h"



#define CFG_ADDR_SHIFT                              0               
#define CFG_ADDR_MASK                               0x7FF
#define CFG_DATA_SHIFT                              12
#define CFG_DATA_MASK                               0x1FF000
#define CFG_CMD_SHIFT                               24
#define CFG_CMD_MASK                                0x0F000000
/* configuration command */
#define CFG_WRITE_CMD_NOP                           0x00
#define CFG_WRITE_CMD_WRITE_ENTRY                   0x01
#define CFG_WRITE_CMD_READ_ENTRY                    0x02
#define CFG_WRITE_CMD_WRITE_ENTRY_ADD_CONN          0x05
#define CFG_WRITE_CMD_WRITE_ENTRY_DELETE_CONN       0x09
#define CFG_WRITE_CMD_WRITE_ENTRY_ADD_CONN_RAW      0x0d


typedef struct t_channel T_CHANNEL;
typedef T_CHANNEL *PT_CHANNEL;

typedef struct cfg_wcmd CFG_WCMD;
typedef CFG_WCMD *PCFG_WCMD;

struct t_channel {
        uint32_t line;
        uint32_t timeslot;
        dag_channel_t type;  /* channel, hyperchannel, or subchannel */
        uint32_t mask;       /* subchannel bitmask, or hyperchannel timeslot map */
        uint32_t ind;        /* current byte index into hdlc data stream */
        uint16_t state;      /* Channel state; active or out of data */
        uint16_t size;       /* Size of subchannel */
        uint8_t data_offset; /* channel data offset, 0 to 7 bit right shift */
        uint8_t first_bit;   /* offset of first bit in subchannel mask */
        uint8_t cur_offset;  /* current subchannel bit offset in the current hdlc byte */
        uint8_t bUsed;       /* if this channel is being used */
        uint8_t bSet;        /* if channel is written to hardware */
        PT_CHANNEL pNext;    /* next in a subchannel chain */ 
        uint32_t wChanNum;   /* channel number */
        uint32_t wNumInChain;/* place in chain */
        uint32_t wDirection;  /* Receive or transmit channel */
}; 

struct cfg_wcmd
{
    uint32_t    wCmd; /* command to write */
    
    PCFG_WCMD   (*New)();
    void        (*Delete)(PCFG_WCMD pThis);
    
    void        (*WriteCmd)(PCFG_WCMD pThis, PCONFIG_MGR pCfgMgr,
                uint32_t direction);
    uint32_t    (*VerifyCmd)(PCFG_WCMD pThis, PCONFIG_MGR pCfgMgr, uint32_t wDirection);
    void        (*SetAddr)(PCFG_WCMD pThis, 
                           uint32_t wIfcNum, 
                           uint32_t wTimeSlot, 
                           uint32_t wMsgType);
    void        (*SetData)(PCFG_WCMD pThis, uint32_t wData);
    void        (*SetCmd)(PCFG_WCMD pThis, uint32_t wCmd);
    void        (*SetChainAddr)(PCFG_WCMD pThis, 
                                uint32_t wCmdType, 
                                uint32_t wPrevChanNum);
};


PCFG_WCMD CWNew();
bool IsLastTsInHyperChannel(PT_CHANNEL pThis, uint32_t wTs);

/***** Get Accessors *****/

/*************************/
static INLINE uint32_t GetChannelIfcNum(PT_CHANNEL pThis)
{
    return pThis->line;
}

static INLINE uint32_t GetChannelTimeslotNum(PT_CHANNEL pThis)
{
    return pThis->timeslot;
}

static INLINE uint32_t GetCWAddr(PCFG_WCMD pThis)
{
    return (pThis->wCmd & CFG_ADDR_MASK) >> CFG_ADDR_SHIFT;
} 

static INLINE uint32_t GetCWData(PCFG_WCMD pThis)
{
    return (pThis->wCmd & CFG_DATA_MASK) >> CFG_DATA_SHIFT;
} 

static INLINE uint32_t GetCWCmd(PCFG_WCMD pThis)
{
    return (pThis->wCmd & CFG_CMD_MASK) >> CFG_CMD_SHIFT;
} 

static INLINE void SetCWReadEntryBit(PCFG_WCMD pThis)
{
    pThis->wCmd |= CFG_WRITE_CMD_DATA_READ_ENTRY_MASK;
}

static INLINE void SetCWWriteEntryDeleteConnBit(PCFG_WCMD pThis)
{
    pThis->wCmd |= CFG_WRITE_CMD_DATA_WRITE_ENTRY_DELETE_CONN_MASK;
}

static INLINE uint32_t GetChannelNum(PT_CHANNEL pThis)
{
     return pThis->wChanNum;
}

static INLINE uint32_t GetChannelNumInChain(PT_CHANNEL pThis)
{
     return pThis->wNumInChain;
}

static INLINE uint32_t GetChannelBitMask(PT_CHANNEL pThis)
{
    if ((CT_CHAN == pThis->type) || (CT_SUB == pThis->type))
    {
        return pThis->mask;
    }   
    else
    {
        return HYPER_CHANNEL_DATA_MASK;
    }
}

#endif /* CFGWCMD_H */
