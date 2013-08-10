/**********************************************************************
*
* Copyright (c) 2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
* $Id: dag_ima_config_api.h,v 1.1.1.1 2011/08/29 05:56:42 dcham Exp $
*
**********************************************************************/

/**********************************************************************
* FILE:		dag_ima_config_api.h
* DESCRIPTION:	API header for IMA configuration
*
*
* HISTORY: 
*   	14/1/05 - Initial version
*
**********************************************************************/

#ifndef DAG_IMA_CONFIG_API_H
#define DAG_IMA_CONFIG_API_H "dag_ima_config_api.h"

#include "global.h"
#include "d37t_msg.h"
#include "d37t_i2c.h"

typedef struct 
{
	uint32_t group_id;	/* IMA Group ID */
	uint32_t ifc_mask;	/* bit mask of included interfaces */
} dag_ima_group_t;

typedef struct
{
    uint32_t num_ifcs;  /* number of ifcs in IMA group */
    uint32_t frame_len; /* IMA frame length for the group */
} dag_ima_group_settings_t;

typedef struct
{
    uint32_t num_groups;                    /* number of available groups */
    uint32_t group_handles[MAX_INTERFACE];  /* handle for each avail group */
} dag_avail_groups_t;

/**********************************************************************
* FUNCTION:	dag_set_mux
* DESCRIPTION:	setup the MUX
* INPUTS:	
*	dagfd     - dag file descriptor
*	host_mode - where to send data coming from the host
*   line_mode - where to send data coming from the line
*   iop_mode  - where to send data coming from the IOP
* OUTPUTS:	None
* RETURNS:	0 on success, 1 on failure
**********************************************************************/
enum 
{
    DA_DATA_TO_HOST = 0,    /* send data to host */
    DA_DATA_TO_LINE,        /* send data to line */
    DA_DATA_TO_IOP          /* send data to IOP  */
};
uint32_t dag_set_mux(int dagfd, uint32_t host_mode, 
                        uint32_t line_mode, uint32_t iop_mode);

/**********************************************************************
* FUNCTION:	dag_set_ima_mode
* DESCRIPTION:	set the configuration mode of the IMA Multiplexer
* INPUTS:	
*	dagfd - dag file descriptor
*	mode  - mode to set
* OUTPUTS:	None
* RETURNS:	Status
**********************************************************************/
enum
{
    DA_RX_UNKNOWN_MODE = 0,
    DA_RX_ATM_IMA_SEARCH,       /* ATM w/ IMA searching */
    DA_RX_ATM_IMA_MANUAL,       /* ATM w/ IMA manually configured */
    DA_RX_IMA_SEARCH,           /* IMA searching only, no ATM */
    DA_RX_IMA_MANUAL,           /* IMA manually configured only, no ATM */
    
    DA_RX_NUM_MODES             /* this must be last */
};
uint32_t dag_set_ima_mode(t_d37t_comm *dagc, uint32_t mode);

/**********************************************************************
* FUNCTION:	dag_add_ima_group
* DESCRIPTION:	add an IMA group to the Multiplexer
* INPUTS:	
*	dagfd    - dag file descriptor
*	group_id - group ID to capture
*   ifc_mask - bit mask of the interfaces included in the group
* OUTPUTS:	None
* RETURNS:	group handle
*           0xFFFFFFFF if unconfigured
**********************************************************************/
uint32_t dag_add_ima_group(t_d37t_comm *dagc, uint32_t group_id, uint32_t ifc_mask);

/**********************************************************************
* FUNCTION:	dag_remove_ima_group
* DESCRIPTION:	remove an IMA group from the Multiplexer
* INPUTS:	
*	dagfd    - dag file descriptor
*	group_handle - handle of group to remove
* OUTPUTS:	None
* RETURNS:	0 if successful, 1 if unsuccessful
**********************************************************************/
uint32_t dag_remove_ima_group(t_d37t_comm *dagc, uint32_t group_handle);

/**********************************************************************
* FUNCTION:	dag_ima_group_ifcs
* DESCRIPTION:	Find the IMA group ID and all the interfaces part of a group
* INPUTS:	
*	dagfd    - dag file descriptor
*	group_handle - handle of group to remove
* OUTPUTS:	None
* RETURNS:	a structure with the IMA Group ID and a bitmask of the ifcs
**********************************************************************/
dag_ima_group_t dag_ima_group_ifcs(t_d37t_comm *dagc, uint32_t group_handle);

/**********************************************************************
* FUNCTION:	dag_ifc_status
* DESCRIPTION:	get the status of an interface
* INPUTS:	
*	dagfd - dag file descriptor
*	ifc   - interface to get status of
* OUTPUTS:	None
* RETURNS:	DAG_IFC_UNCONFIGURED
*			DAG_IFC_ATM
*			DAG_IFC_IMA_WAITING
*			DAG_IFC_IMA_RUNNING
**********************************************************************/
enum    /* values to be returned via DRB_API  for interface status */
{
    DAG_IFC_UNCONFIGURED = 0,   /* unconfigured interface */
    DAG_IFC_ATM,                /* ATM interface */
    DAG_IFC_IMA_RUNNING,        /* IMA interface */
    DAG_IFC_IMA_WAITING,        /* IMA interface waiting for ICP */
    
    DAG_IFC_NUM_MODES           /* this must be last in the enum */
};
uint32_t dag_ifc_status(t_d37t_comm *dagc, uint32_t ifc);

/**********************************************************************
* FUNCTION:	dag_group_status
* DESCRIPTION:	get the status of a group
* INPUTS:	
*	dagfd        - dag file descriptor
*	group_handle - handle of group to get status of
* OUTPUTS:	None
* RETURNS:	DAG_IMA_GROUP_UNCONFIGURED
*			DAG_IMA_GROUP_INCOMPLETE
*			DAG_IMA_GROUP_RUNNING
**********************************************************************/
enum    /* values to be returned via DRB_API for group status */
{
    DAG_IMA_GROUP_UNCONFIGURED = 0, /* unconfigured group */ 
    DAG_IMA_GROUP_INCOMPLETE,       /* incomplete group */
    DAG_IMA_GROUP_RUNNING,          /* group is running */
    
    DAG_IMA_GROUP_NUM_MODES         /* this must be last in the enum */
};
uint32_t dag_group_status(t_d37t_comm *dagc, uint32_t group_handle);
/**********************************************************************
* FUNCTION:	dag_avail_groups
* DESCRIPTION:	get a list of handles for each available group
* INPUTS:	
*	dagfd - dag file descriptor
* OUTPUTS:	
* RETURNS:	dag_avail_groups_t containing num of avail groups and their handles
* NOTE: if the command failed the num of avail groups is 
*       DAG_AVAIL_GROUPS_CMD_FAILED
**********************************************************************/
#define DAG_NO_AVAIL_GROUP  0xFFFFFFFF
#define DAG_AVAIL_GROUPS_CMD_FAILED 0xFFFFFFFF
dag_avail_groups_t dag_avail_groups(t_d37t_comm *dagc);

/**********************************************************************
* FUNCTION:	dag_group_settings
* DESCRIPTION:	get the settings of a group
* INPUTS:	
*	dagfd        - dag file descriptor
*	group_handle - handle of group to get settings of
* OUTPUTS:	None
* RETURNS:	settings for the group
**********************************************************************/
dag_ima_group_settings_t dag_group_settings(t_d37t_comm *dagc, 
                                            uint32_t group_handle);

/**********************************************************************
* FUNCTION:	dag_ima_stats
* DESCRIPTION:	get stats of how many cells have been received
* INPUTS:	
*	dagfd        - dag file descriptor
*	cell_cmd     - specifies the type of cell to be counted and whether the 
*                  count is for all ifcs, a single ifc, or a group
*   option       - the number of the ifc or handle of the group for which cell
*                  count is desired. if total cells are desired for all ifcs
*                  then this is meaningless and can be left as a 0.
* OUTPUTS:	None
* RETURNS:	cell count desired
**********************************************************************/
enum /* values to be used as cell_cmd to define type of cell count requested */
{
    /* total from all groups/ifcs */
    DAG_IMA_TOTAL_CELLS = 0,
    DAG_IMA_TOTAL_DATA_CELLS,
    DAG_IMA_TOTAL_IDLE_CELLS,
    DAG_IMA_TOTAL_ICP_CELLS,
    
    /* single group count */
    DAG_IMA_GROUP_TOTAL_CELLS,
    DAG_IMA_GROUP_DATA_CELLS,
    DAG_IMA_GROUP_IDLE_CELLS,
    DAG_IMA_GROUP_ICP_CELLS,
    
    /* single ifc count */
    DAG_IMA_IFC_TOTAL_CELLS,
    DAG_IMA_IFC_DATA_CELLS,
    DAG_IMA_IFC_IDLE_CELLS,
    DAG_IMA_IFC_ICP_CELLS,
};
uint32_t dag_ima_stats(t_d37t_comm *dagc, uint32_t cell_cmd, uint32_t option);

/**********************************************************************
* FUNCTION:	dag_ima_conf_status
* DESCRIPTION:	get configuration status of the IMA multiplexer
* INPUTS:	None
* OUTPUTS:	None
* RETURNS:	Configuration status
**********************************************************************/
enum    /* configuration status */
{
    DAG_IMA_CONF_UNKNOWN = 0,   /* unknown, no configured interfaces */
    DAG_IMA_CONF_OK,            /* atleast one group is running */
    DAG_IMA_CONF_PENDING,       /* atleast one group is waiting */
};
uint32_t dag_ima_conf_status(t_d37t_comm *dagc);

#endif
