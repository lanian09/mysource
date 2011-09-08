/**********************************************************************
*
* Copyright (c) 2004-2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
* $Id: dag37t_api.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		dag37t_api.h
* DESCRIPTION:	DAG 3.7T Programming Interface
*
* HISTORY:
*   LDM 10/6/04 - Initial version
**********************************************************************/

#ifndef DAG37T_API_H
#define DAG37T_API_H "dag37t_api.h"

#include <dag_platform.h>

typedef enum
{
    CT_UNDEFINED,   /* undefined channel type */
    CT_CHAN,        /* simple channel type */
    CT_HYPER,       /* hyperchannel type */
    CT_SUB,         /* subchannel type */
    CT_RAW,         /* raw connection type */
    CT_CHAN_RAW,
    CT_HYPER_RAW,
    CT_SUB_RAW
} dag_channel_t;


/* directions */
#define DAG_DIR_UNDEFINED   0
#define DAG_DIR_RECEIVE     1
#define DAG_DIR_TRANSMIT    2

/**********************************************************************
* FUNCTION:	dag_add_channel
* DESCRIPTION:	define and activate a channel. ERF records from a 
*               receive channel will begin to be received in the host's
*               circular packet buffer once the add operation has 
*               completed. Records will continue to be received until 
*               the channel is deleted.
*               
*               adds a new channel configuration to the FPGA.
* INPUTS:	
*   dagfd     - dag card handle received from dag_open
*   direction - receive or transmit
*   type      - channel, hyperchannel, or subchannel
*   line      - an interface (0-15)
*   tsConf    - timeslot configuration
* OUTPUTS:	None
* RETURNS:	channel ID number or -1 if failure
* NOTES:    tsConf is different for different types of channels. here
*           is a description of what it should be for each channel type
*   channel      - timeslot number (0-31 for E1 or 0-23 for T1)
*   subchannel   - upper 16 bits = timeslot number
*                  lowest 8 bits are subchannel mask with 1,2,or 4 contiguous
*                  bits set
*   hyperchannel - bitmap of timeslots to configure within the 
*                  hyperchannel
* see below for examples
**********************************************************************/
int dag_add_channel(int dagfd, 
                uint32_t direction, 
                uint32_t type, 
                uint32_t line, 
                uint32_t tsConf,
                uint32_t fake_transmit);
                      
/**********************************************************************
* FUNCTION:	dag_delete_channel
* DESCRIPTION:	removes channel configuration from FPGA and makes
*               channel ID available for re-use. Also deletes any
*               associated filters.
* INPUTS:
*   dagfd  - dag card handle received from dag_open
*   chanId - channel ID of channel to be deleted
* OUTPUTS:	None
* RETURNS:	0 on success
            -1 on failure 
**********************************************************************/
int dag_delete_channel(int dagfd, uint32_t chanId);

/**********************************************************************
* FUNCTION:	dag_delete_line_channels
* DESCRIPTION:	removes all channel configurations for a line from FPGA
*               and makes channel ID available for re-use. Also deletes
*               any associated filters.
* INPUTS:
*   dagfd  - dag card handle received from dag_open
*   lineId - line ID to be deleted
* OUTPUTS:	None
* RETURNS:	0 on success
            -1 on failure 
**********************************************************************/
int dag_delete_line_channel(int dagfd, uint32_t lineId);

/**********************************************************************
* FUNCTION:	dag_delete_board_channels
* DESCRIPTION:	removes all channel configurations for a board from FPGA
*               and makes channel ID available for re-use. Also deletes
*               any associated filters.
* INPUTS:
*   dagfd - dag card handle received from dag_open
*   line  - number of line to make a raw connection on
* OUTPUTS:	None
* RETURNS:	0 on success
            -1 on failure 
**********************************************************************/
int dag_delete_board_channels(int dagfd);

/**********************************************************************
* FUNCTION:	dag_channel_verbosity
* DESCRIPTION:	Sets the verbosity level of the DAG 3.7T api for debug
* INPUTS:
*   verb  - level to set
* OUTPUTS:	None
* RETURNS:	None 
**********************************************************************/
void dag_channel_verbosity(uint32_t verb);

/**********************************************************************
* FUNCTION:	dag_ifc_hec
* DESCRIPTION:	sets HEC correction for an interface
* INPUTS:
*   ifc - interface number (0-15)
*   opt - FALSE == HEC off
*       - TRUE  == HEC on
* OUTPUTS:	None
* RETURNS:	0 if successful
*          -1 if error
**********************************************************************/
int dag_ifc_hec (int dagfd, uint32_t ifc, uint32_t opt);

/**********************************************************************
* FUNCTION:	dag_ifc_cell_scrambling
* DESCRIPTION:	sets Cell Scrambling for an interface
* INPUTS:
*   ifc - interface number (0-15)
*   opt - FALSE == scrambling off
*       - TRUE  == scrambling on
* OUTPUTS:	None
* RETURNS:	0 if successful
*          -1 if error
**********************************************************************/
int dag_ifc_cell_scrambling (int dagfd, uint32_t ifc, uint32_t opt);

/**********************************************************************
* FUNCTION:	dag_reset_37t
* DESCRIPTION:	resets the library, to be used only when firmware has been updated
* INPUTS:
*   dagfd - dag device descriptor
* OUTPUTS:	None
* RETURNS:	0 if successful
*          -1 if error
**********************************************************************/
int dag_reset_37t (int dagfd);

/***************
* Examples: adding different types of channels
*
* add a simple receive channel, interface 3, timeslot 8
*   rslt = dag_add_channel(dagfd, DAG_DIR_RECEIVE, CT_CHAN, 3, 8);
*
* add a receive subchannel, interface 1, timeslot 7, bits 2-5
*   rslt = dag_add_channel(dagfd, DAG_DIR_RECEIVE, CT_SUB, 1, 0x0007003C);
* the 7 is the timeslot number and the 3C is the subchannel mask
*
* add a receive hyperchannel, interface 14, timeslots 1,2,3,7,8,12,15,27
*   rslt = dag_add_channel(dagfd, DAG_DIR_RECEIVE, CT_HYPER, 14, 0x040048c7);
* 0x040048c7 is the timeslot bitmap. the way that is made is by putting a
* 1 at the bit you want part of the hyperchannel, the first bit is for
* timeslot 1.
*
* add a raw connection on line 2
*   rslt = dag_add_channel(dagfd, DAG_DIR_RECEIVE, CT_RAW, 2, 0);
* this can only be done if there is not a channel 2, and no channels 
* exist on line 2. the tsConf does not matter, so a 0 is written there.
****************/           

#endif /* DAG37T_API_H */
