/**********************************************************************
*
* Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
* $Id: iomgr.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		iomgr.h
* DESCRIPTION:	I/O Manager
*
* HISTORY:
*
**********************************************************************/

#ifndef IO_MGR_H
#define IO_MGR_H iomgr.h

#include <cfgwcmd.h>
#include <dagapi.h>

typedef struct io_mgr IO_MGR;
typedef IO_MGR *PIO_MGR;

struct io_mgr
{
    T_CHANNEL aChannel[MAX_CHANNEL];
    uint32_t wTotalChannels;
    PCONFIG_MGR pCfgMgr;

    PIO_MGR pIoMgrNext;

    PIO_MGR(*New) ();
    void (*Delete) (PIO_MGR pThis);

    /**
    Make a simple channel.
    \param pThis pointer to this io_mgr.
    \param wChanNum the channel number.
    */
    void (*MakeSimpleChannel) (PIO_MGR pThis, uint32_t wChanNum);

    /**
    Make a subchannel.
    \param pThis pointer to this io_mgr.
    \param wIfcNum the interface number.
    \param wTsNum the timeslot number.
    */
    void (*MakeSubChannels) (PIO_MGR pThis, uint32_t wIfcNum, uint32_t wTsNum, uint32_t wDirection);

    void (*MakeHyperChannel) (PIO_MGR pThis, uint32_t wChanNum);
    void (*AddSubChannelToChain) (PIO_MGR pThis, PT_CHANNEL pChannel, PT_CHANNEL pLastInChain);
    void (*MakeSimpleChannelOnRaw) (PIO_MGR pThis, uint32_t wChanNum);
    void (*MakeSubChannelsOnRaw) (PIO_MGR pThis, uint32_t wChanNum);
    void (*MakeHyperChannelOnRaw) (PIO_MGR pThis, uint32_t wChanNum);
    void (*AddSubChannelToChainOnRaw) (PIO_MGR pThis, PT_CHANNEL pChannel, PT_CHANNEL pLastInChain);
    uint32_t(*IsTsUsed) (PIO_MGR pThis, uint32_t wType, uint32_t wIfc, uint32_t wTimeslotConf, uint32_t wDirection);
    void (*DeleteChannel) (PIO_MGR pThis, PT_CHANNEL pChannel);
    void (*DeleteInterface) (PIO_MGR pThis, uint32_t wIfcIndex);
    void (*DeleteAll) (PIO_MGR pThis);
    PT_CHANNEL(*GetChainHead) (PIO_MGR pThis, uint32_t wIfc, uint32_t wTimeslot, uint32_t wDirection);
    PT_CHANNEL(*GetChainTail) (PIO_MGR pThis, uint32_t wIfc, uint32_t wTimeslot, uint32_t wDirection);
    PT_CHANNEL(*GetChainPrev) (PIO_MGR pThis, PT_CHANNEL pChannel);
    void (*RenumberChain) (PIO_MGR pThis, PT_CHANNEL pChannel);
    void (*DeleteRawLine) (PIO_MGR pThis, PT_CHANNEL pChannel);
    void (*MakeRawLine) (PIO_MGR pThis, uint32_t wIfc);
};

PIO_MGR IMNew(PCONFIG_MGR pCfgMgr);
PIO_MGR IMFindNew(int iDagFd);

#ifndef INLINE
#ifdef _WIN32
#define INLINE __inline
#endif
#endif

static INLINE PT_CHANNEL
GetIMChannelArray(PIO_MGR pThis)
{
    return &pThis->aChannel[0];
}

static INLINE uint32_t
GetIMTotalChannels(PIO_MGR pThis)
{
    return pThis->wTotalChannels;
}

static INLINE void
IncrIMTotalChannels(PIO_MGR pThis)
{
    pThis->wTotalChannels++;
}

static INLINE void
DecrIMTotalChannels(PIO_MGR pThis)
{
    pThis->wTotalChannels--;
}

static INLINE void
SetIMTotalChannels(PIO_MGR pThis, uint32_t wTotalChannels)
{
    pThis->wTotalChannels = wTotalChannels;
}

static INLINE PT_CHANNEL
GetIMChannelPtr(PIO_MGR pThis, uint32_t wChannelNumber)
{
    return &pThis->aChannel[wChannelNumber];
}

static INLINE uint32_t
GetIMUnusedChannelNum(PIO_MGR pThis)
{
    uint32_t i;

    for (i = MAX_INTERFACES; i < MAX_CHANNEL; i++)
    {
        if (pThis->aChannel[i].bUsed == FALSE)
        {
            break;
        }
    }

    return i;
}

static INLINE void
SetIMRawMode(PIO_MGR pThis, uint32_t wIfc, uint32_t wDirection)
{
    uint32_t wTemp;

    if (wDirection == DAG_DIR_RECEIVE)
    {
        if (DAG37T_DEMAPPER_MODE_HDLC == CMGetDemapperMode(pThis->pCfgMgr))
        {
            wTemp = *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                            MAP_STATUS_RAW_CTRL_REG);

            VERBOSE_LEVEL_3("    Read 0x%08x\n", wTemp);
            wTemp |= (0x01 << (wIfc + 16LL));
            VERBOSE_LEVEL_3("    Write 0x%08x\n", wTemp);

            *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                    MAP_STATUS_RAW_CTRL_REG) = wTemp;
        }
        else
        {
            VERBOSE_LEVEL_3("RAW channels not available in current mode\n");
        }
    }
    else if (wDirection == DAG_DIR_TRANSMIT)
    {
        if (DAG37T_MAPPER_MODE_HDLC == CMGetMapperMode(pThis->pCfgMgr))
        {
            wTemp = *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioMapper) +
                                            MAP_STATUS_RAW_CTRL_REG);

            VERBOSE_LEVEL_3("    Read 0x%08x\n", wTemp);
            wTemp |= (0x01 << (wIfc + 16LL));
            VERBOSE_LEVEL_3("    Write 0x%08x\n", wTemp);

            *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioMapper) +
                                    MAP_STATUS_RAW_CTRL_REG) = wTemp;
        }
        else
        {
            VERBOSE_LEVEL_3("RAW channels not available in current mode\n");
        }
    }
    else
    {
        VERBOSE_LEVEL_1("  Invalid direction");
    }
}

static INLINE void
ClrIMRawMode(PIO_MGR pThis, uint32_t wIfc, uint32_t wDirection)
{
    uint32_t wTemp;

    if (wDirection == DAG_DIR_RECEIVE)
    {
        if (DAG37T_DEMAPPER_MODE_HDLC == CMGetDemapperMode(pThis->pCfgMgr))
        {
            wTemp = *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                            MAP_STATUS_RAW_CTRL_REG);

            VERBOSE_LEVEL_3("    Read 0x%08x\n", wTemp);
            wTemp &= ~(0x01 << (wIfc + 16LL));
            VERBOSE_LEVEL_3("    Write 0x%08x\n", wTemp);

            *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                    MAP_STATUS_RAW_CTRL_REG) = wTemp;
        }
        else
        {
            VERBOSE_LEVEL_3("RAW channels not available in current mode\n");
        }
    }
    else if (wDirection == DAG_DIR_TRANSMIT)
    {
        if (DAG37T_MAPPER_MODE_HDLC == CMGetMapperMode(pThis->pCfgMgr))
        {
            wTemp = *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioMapper) +
                                            MAP_STATUS_RAW_CTRL_REG);

            VERBOSE_LEVEL_3("    Read 0x%08x\n", wTemp);
            wTemp &= ~(0x01 << (wIfc + 16LL));
            VERBOSE_LEVEL_3("    Write 0x%08x\n", wTemp);

            *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioMapper) +
                                    MAP_STATUS_RAW_CTRL_REG) = wTemp;
        }
        else
        {
            VERBOSE_LEVEL_3("RAW channels not available in current mode\n");
        }
    }
    else
    {
        VERBOSE_LEVEL_1("  Invalid direction");
    }
}

static INLINE void
IMEnableHecCorrection(PIO_MGR pThis, uint32_t wIfc)
{
    uint32_t wTemp;

    if (DAG37T_DEMAPPER_MODE_ATM == CMGetDemapperMode(pThis->pCfgMgr))
    {
        wTemp = *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                        CFG_CELL_CONTROL_REG);

        VERBOSE_LEVEL_3("    Read 0x%08x\n", wTemp);
        wTemp |= (0x01 << (wIfc + CFG_CELL_CTRL_HEC_CORRECTION_SHIFT));
        VERBOSE_LEVEL_3("    Write 0x%08x\n", wTemp);

        *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                CFG_CELL_CONTROL_REG) = wTemp;
    }
    else
    {
        VERBOSE_LEVEL_3("HEC correction not available in current mode\n");
    }
}

static INLINE void
IMDisableHecCorrection(PIO_MGR pThis, uint32_t wIfc)
{
    uint32_t wTemp;

    if (DAG37T_DEMAPPER_MODE_ATM == CMGetDemapperMode(pThis->pCfgMgr))
    {
        wTemp = *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                        CFG_CELL_CONTROL_REG);

        VERBOSE_LEVEL_3("    Read 0x%08x\n", wTemp);
        wTemp &= ~(0x01 << (wIfc + CFG_CELL_CTRL_HEC_CORRECTION_SHIFT));
        VERBOSE_LEVEL_3("    Write 0x%08x\n", wTemp);

        *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                CFG_CELL_CONTROL_REG) = wTemp;
    }
    else
    {
        VERBOSE_LEVEL_3("HEC correction not available in current mode\n");
    }
}

static INLINE void
IMEnableCellScrambling(PIO_MGR pThis, uint32_t wIfc)
{
    uint32_t wTemp;

    if (DAG37T_DEMAPPER_MODE_ATM == CMGetDemapperMode(pThis->pCfgMgr))
    {
        wTemp = *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                        CFG_CELL_CONTROL_REG);

        VERBOSE_LEVEL_3("    Read 0x%08x\n", wTemp);
        wTemp |= (0x01 << (wIfc + CFG_CELL_CTRL_SCRAMBLING_SHIFT));
        VERBOSE_LEVEL_3("    Write 0x%08x\n", wTemp);

        *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                CFG_CELL_CONTROL_REG) = wTemp;
    }
    else
    {
        VERBOSE_LEVEL_3("Cell scrambling not available in current mode\n");
    }
}

static INLINE void
IMDisableCellScrambling(PIO_MGR pThis, uint32_t wIfc)
{
    uint32_t wTemp;

    if (DAG37T_DEMAPPER_MODE_ATM == CMGetDemapperMode(pThis->pCfgMgr))
    {
        wTemp = *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                        CFG_CELL_CONTROL_REG);

        VERBOSE_LEVEL_3("    Read 0x%08x\n", wTemp);
        wTemp &= ~(0x01 << (wIfc + CFG_CELL_CTRL_SCRAMBLING_SHIFT));
        VERBOSE_LEVEL_3("    Write 0x%08x\n", wTemp);

        *(volatile unsigned *) (MIOGetLoc(pThis->pCfgMgr->pmioDemapper) +
                                CFG_CELL_CONTROL_REG) = wTemp;
    }
    else
    {
        VERBOSE_LEVEL_3("Cell scrambling not available in current mode\n");
    }
}

#ifdef _WIN32
#undef inline
#endif

#endif /* IO_MGR_H */
