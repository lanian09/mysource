/**********************************************************************
*
* Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
* $Id: global.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		global.h
* DESCRIPTION:	Globals
*
*
* HISTORY:
*
**********************************************************************/

#ifndef GLOBAL_H
#define GLOBAL_H "global.h"

/* Endace headers. */
#include "dag_platform.h"


#ifndef TRUE
    #define TRUE    1
    #define FALSE   0
#endif

/* Channel types */

#define MAX_INTERFACE               16
#define TIMESLOTS_PER_INTERFACE     32
#define MAX_CONNECTIONS             512

#define MAP_STATUS_RAW_CTRL_REG         0x00
    #define  FRAMER_REVISION_ID_MASK        0x0F
    #define  NUM_OPEN_CONNS_MASK            0x1FF0
    #define  LATCH_CLR_COUNTERS_MASK        0x8000
    #define  RAW_MODE_MASK                  0xFFFF0000

/* configuration command register */
#define CFG_CMD_REG                                 0x04
    #define CFG_CMD_NOP                                 0x00
    #define CFG_CMD_READ_BUF                            0x01
    #define CFG_CMD_WRITE_BUF                           0x02
    #define CFG_CMD_EXEC_BUF                            0x04
    #define CFG_CMD_RESET_CMD_COUNT                     0x10

/* configuration status register */
#define CFG_STATUS_REG                              0x08
    #define CFG_STATUS_CMD_COUNT_MASK                   0x7F
    #define CFG_STATUS_BUF_READY_MASK                   0x100

/* configuration write command/data register */
#define CFG_WRITE_CMD_DATA_REG                              0x0C
#define CFG_WRITE_CMD_DATA_WRITE_ENTRY_MASK                 0x01000000
#define CFG_WRITE_CMD_DATA_READ_ENTRY_MASK                  0x02000000
#define CFG_WRITE_CMD_DATA_WRITE_ENTRY_ADD_CONN_MASK        0x05000000
#define CFG_WRITE_CMD_DATA_ADD_CONN_MASK                    0x04000000
#define CFG_WRITE_CMD_DATA_ADD_CONN_RAW_MASK                0x08000000
#define CFG_WRITE_CMD_DATA_WRITE_ENTRY_DELETE_CONN_MASK     0x09000000
#define CFG_WRITE_CMD_DATA_CHAIN_MASK_FLAG                  0x00100000
#define CFG_WRITE_CMD_DATA_CHAIN_ADDR_FLAG                  0x00000100

/* configuration read command/data register */
#define CFG_READ_CMD_DATA_REG                       0x10
    #define CFG_WRITE_CFG_ADDR_MASK                     0x7FF
    #define CFG_WRITE_CFG_DATA_MASK                     0x1FF000
    
/* configuration table read data register */
#define CFG_TABLE_READ_DATA_REG                     0x14
    #define CFG_TBL_READ_DATA_ADDR_MASK                 0x7FF
    #define CFG_TBL_READ_CFG_DATA_MASK                  0x1FF00
/* status counters */
#define STATUS_COUNTERS_REG                         0x18
    #define STATUS_CNTR_RCV_LOST_RECS_MASK              0xFFFF

/* mapper revision id */
#define CFG_REVISION_ID_REG 0x0
#define CFG_REVISION_ID_MASK 0xFF

#define CFG_CELL_CONTROL_REG                     0x18
    #define CFG_CELL_CTRL_HEC_CORRECTION_MASK        0xFFFF0000
    #define CFG_CELL_CTRL_HEC_CORRECTION_SHIFT       16
    #define CFG_CELL_CTRL_SCRAMBLING_MASK            0x0000FFFF
    #define CFG_CELL_CTRL_SCRAMBLING_SHIFT           0

#define CFG_CONN_MSG        0x0
#define CFG_CONN_PTR_MSG    0x200
#define CFG_MASK_MSG        0x400
#define CFG_CHAIN_MSG       0x600

#define DAG_REG_BASE	0x900
#define DAG_REG_ADDR_START	0xffa5
#define DAG_REG_ADDR_END	0xffff
#define CAPS_BASE	0x98
#define CAPS_MAGIC      0xa6000000
#define CAPS_MAGIC_MASK 0xff000000

#define MAX_CHANNEL 2048

#define HYPER_CHANNEL_DATA_MASK 0xFF
#define RAW_MODE_TS_MASK        0xFFFFFFFF
#define MAX_TIMESLOT_CONF       0x7FFFFFFF

#define DAG_TS_UNUSED        0
#define DAG_TS_USED          1
#define DAG_TS_USED_RAW      2

#define DAG_CMD_BUF_SIZE    80
#define DAG_CMD_BUF_THRESHOLD    (DAG_CMD_BUF_SIZE - 1)

#define DAG_CMD_NOT_VERIFIED     0
#define DAG_CMD_VERIFIED         1
#define DAG_CMD_BUFFER_OVERRUN   2

#define DAG37t_NO_ERR            0
#define DAG37T_BUFFER_OVERRUN    1
#define DAG37T_CMD_NOT_VERIFIED  2

#define DAG37T_DEMAPPER_MODE_UNDEFINED 0
#define DAG37T_DEMAPPER_MODE_HDLC      1
#define DAG37T_DEMAPPER_MODE_ATM       2
#define DAG37T_MAPPER_MODE_HDLC        3
#define DAG37T_MAPPER_MODE_ATM         4

#endif /* GLOBAL_H */


