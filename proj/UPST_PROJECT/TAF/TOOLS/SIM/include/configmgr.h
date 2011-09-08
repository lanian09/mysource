/**********************************************************************
*
* Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
* $Id: configmgr.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		configmgr.h
* DESCRIPTION:	Configuration manager
*
* HISTORY:
*
**********************************************************************/

#ifndef CONFIG_MGR_H
#define CONFIG_MGR_H "configmgr.h"

/* Endace headers. */
#include "dag_platform.h"
#include "daginf.h"
#include "dagreg.h"

/* DAG 3.7T headers. */
#include "moduleio.h"


#define VERBOSE_LEVEL_0   if (CMGetVerbosity() >= 0) printf
#define VERBOSE_LEVEL_1   if (CMGetVerbosity() >= 1) printf
#define VERBOSE_LEVEL_2   if (CMGetVerbosity() >= 2) printf
#define VERBOSE_LEVEL_3   if (CMGetVerbosity() >= 3) printf
#define VERBOSE_LEVEL_4   if (CMGetVerbosity() >= 4) printf
#define VERBOSE_LEVEL_5   if (CMGetVerbosity() >= 5) printf

#ifdef _WIN32
#define DEFAULT_DAG_NAME    "dag0"
#else
#define DEFAULT_DAG_NAME    "/dev/dag0"
#endif

typedef struct config_mgr CONFIG_MGR;
typedef CONFIG_MGR *PCONFIG_MGR;
struct config_mgr
{
    uint32_t iVerbosity;
    char *pcChannelConfigFile;
    char *pcDagName;
    int iDagFd;
    daginf_t *pDagInf;
    uint8_t *pucDagIom;
    dag_reg_t drRegs[DAG_REG_MAX_ENTRIES];
    uint32_t iRegCount;

    /* Demapper */
    PMODULE_IO pmioDemapper;
    uint8_t byDemapperMode;

    /* Mapper */
    PMODULE_IO pmioMapper;
    uint8_t byMapperMode;

    uint32_t bRemoveChannels;
    uint32_t bQualityCheck;

    PCONFIG_MGR(*New) (char *pcDagName,
                       uint32_t iVerb, uint32_t bRemoveChannels, uint32_t bQualityCheck);
    PCONFIG_MGR(*ACNew) (int iDagFd);
    void (*Delete) (PCONFIG_MGR pThis);

    void (*DemapperWrite) (PCONFIG_MGR pThis, unsigned addr, unsigned value);
    unsigned (*DemapperRead) (PCONFIG_MGR pThis, unsigned addr);
    void (*ResetDemapperCmdCount) (PCONFIG_MGR pThis);
    void (*CopyDemapperCmdToBuffer) (PCONFIG_MGR pThis);
    uint32_t(*GetDemapperCmdCount) (PCONFIG_MGR pThis);
    void (*ExecDemapperCfgBuf) (PCONFIG_MGR pThis);
    uint32_t(*ReadDemapperBufEntry) (PCONFIG_MGR pThis);

    uint32_t(*ReadDemapperConfigAddr) (PCONFIG_MGR pThis, uint32_t wAddr);

    /**
    \sa CMGetDemmaperRevisionId
    */
    uint32_t(*GetDemapperRevisionId) (PCONFIG_MGR pThis);

    /**
    \sa CMDagIomMapperWrite
    */
    void (*MapperWrite) (PCONFIG_MGR pThis, unsigned addr, unsigned value);

    /**
    \sa CMDagIomMapperRead
    */
    unsigned (*MapperRead) (PCONFIG_MGR pThis, unsigned addr);

    /**
    \sa CMResetMapperCmdCount
    */
    void (*ResetMapperCmdCount) (PCONFIG_MGR pThis);

    /**
    \sa CMCopyMapperCmdToBuffer
    */
    void (*CopyMapperCmdToBuffer) (PCONFIG_MGR pThis);

    /**
    \sa CMGetMapperCmdCount
    */
    uint32_t(*GetMapperCmdCount) (PCONFIG_MGR pThis);

    /**
    \sa CMExecMapperCfgBuf
    */
    void (*ExecMapperCfgBuf) (PCONFIG_MGR pThis);

    /**
    \sa CMReadMapperBufEntry
    */
    uint32_t(*ReadMapperBufEntry) (PCONFIG_MGR pThis);

    /**
    \sa CMReadMapperConfigAddr
    */
    uint32_t(*ReadMapperConfigAddr) (PCONFIG_MGR pThis, uint32_t wAddr);

    /**
    \sa CMGetMapperRevisionId
    */
    uint32_t(*GetMapperRevisionId) (PCONFIG_MGR pThis);

};

PCONFIG_MGR CMNew(char *pcDagName, uint32_t iVerb, uint32_t bRemoveChannels,
                  uint32_t bQualityCheck);

PCONFIG_MGR ACNew(int iDagFd);
void CMReset(PCONFIG_MGR pCfgMgr, int iDagFd);

uint32_t CMGetVerbosity();
PCONFIG_MGR CMGetCfgMgrPtr();


static INLINE int
GetCMDagFd(PCONFIG_MGR pThis)
{
    return pThis->iDagFd;
}

static INLINE void
CMSetVerbosity(PCONFIG_MGR pThis, uint32_t iVerb)
{
    pThis->iVerbosity = iVerb;
}


static INLINE uint32_t
GetCMQualityCheck(PCONFIG_MGR pThis)
{
    return pThis->bQualityCheck;
}

static INLINE void
SetCMRemoveChannels(PCONFIG_MGR pThis, uint32_t wVal)
{
    pThis->bRemoveChannels = wVal;
}

static INLINE PMODULE_IO
CMGetDemapper(PCONFIG_MGR pThis)
{
    return pThis->pmioDemapper;
}

static INLINE uint8_t
CMGetDemapperMode(PCONFIG_MGR pThis)
{
    return pThis->byDemapperMode;
}

static INLINE PMODULE_IO
CMGetMapper(PCONFIG_MGR pThis)
{
    return pThis->pmioMapper;
}

static INLINE uint8_t
CMGetMapperMode(PCONFIG_MGR pThis)
{
    return pThis->byMapperMode;
}

#endif /* CONFIG_MGR_H */
