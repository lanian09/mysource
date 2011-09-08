/**********************************************************************
*
* Copyright (c) 2005 Endace Technology Ltd, Hamilton, New Zealand.
* All rights reserved.
*
* This source code is proprietary to Endace Technology Limited and no part
* of it may be redistributed, published or disclosed except as outlined in
* the written contract supplied with this product.
*
**********************************************************************/

#ifndef D37T_MSG_H
#define D37T_MSG_H "d37t_msg.h"
#include "dag_platform.h"

#include "dag_platform.h"
#include "dagapi.h"

/* Device info per dag */

/* Communication modes */
#define DAG_COMM_MODE_DRB	0	/* read and write via the DRB */
#define DAG_COMM_MODE_SOCKET	1	/* read and write via dagconsole socket */

/* DAG COMM parameters */
#define DAG_COMM_SOCK_TIMEOUT	0x0001	/* socket read timeout parameter.  Millisecond value */
#define DAG_COMM_SOCK_TIMEOUT_DEFAULT	2000	/* 2 second defaule */

typedef struct {
    int dagfd;
    volatile char *iom;
    volatile uint32_t *pci_in;	/* DRB input */
    volatile uint32_t *pci_out;	/* DRB output */
    uint32_t pci_out_shadow;	/* Local copy of last sent data - saves PCI bus read access */
    uint8_t write_count;	/* current byte write count */
    int sock;			/* Socket for dagconsole communication mode */
    struct sockaddr_un sock_addr;

    /* tunable library parameters */
    uint32_t mode;		/* Messaging mode */
    uint32_t sock_timeout;	/* socket receive timeout in milliseconds */
} t_d37t_comm;

#define D37T_MSG_PAYLOAD_LENGTH	2048	/* Maximum payload length */

typedef struct {
    uint32_t message_id;
    uint32_t length;
} t_d37t_msg_header;

typedef struct {
    t_d37t_msg_header header;
    uint8_t payload[D37T_MSG_PAYLOAD_LENGTH];
} t_d37t_msg;


/*
 * Message addressing classes
 * Allows dispatch of messages to handlers.
 */
#define D37T_CLASS_CONTROL	0x10000000
#define D37T_CLASS_I2C		0x10010000
#define D37T_CLASS_IMA      0x10020000
#define D37T_CLASS_MASK		0xFFFF0000

/*
 * I2C Messages
 */
#define D37T_MSG_READ_SOFTWARE_ID		(D37T_CLASS_I2C | 0x0001)
#define D37T_MSG_READ_SOFTWARE_ID_CMPLT		(D37T_CLASS_I2C | 0x0002)
#define D37T_MSG_WRITE_SOFTWARE_ID		(D37T_CLASS_I2C | 0x0003)
#define D37T_MSG_WRITE_SOFTWARE_ID_CMPLT	(D37T_CLASS_I2C | 0x0004)
#define D37T_MSG_READ_TEMPERATURE		(D37T_CLASS_I2C | 0x0005)
#define D37T_MSG_READ_TEMPERATURE_CMPLT		(D37T_CLASS_I2C | 0x0006)

#define D37T_MSG_READ_SOFTWARE_ID_SIZE		sizeof(t_d37t_msg_read_software_id)
#define D37T_MSG_READ_SOFTWARE_ID_CMPLT_SIZE	sizeof(t_d37t_msg_read_software_id_cmplt)
#define D37T_MSG_WRITE_SOFTWARE_ID_SIZE		sizeof(t_d37t_msg_write_software_id)
#define D37T_MSG_WRITE_SOFTWARE_ID_CMPLT_SIZE	sizeof(t_d37t_msg_write_software_id_cmplt)
#define D37T_MSG_READ_TEMPERATURE_SIZE		sizeof(t_d37t_msg_read_temperature)
#define D37T_MSG_READ_TEMPERATURE_CMPLT_SIZE	sizeof(t_d37t_msg_read_temperature_cmplt)

#define D37T_MSG_ERROR				(D37T_CLASS_CONTROL | 0x0001)
#define D37T_MSG_ERROR_SIZE			sizeof(t_d37t_msg_error);

#define D37T_MSG_SET_DEBUG			(D37T_CLASS_CONTROL | 0x0003)
#define D37T_MSG_SET_DEBUG_CMPLT		(D37T_CLASS_CONTROL | 0x0004)
#define D37T_MSG_SET_DEBUG_SIZE			sizeof(t_d37t_msg_set_debug)
#define D37T_MSG_SET_DEBUG_CMPLT_SIZE		sizeof(t_d37t_msg_set_debug_cmplt)


/* IMA Messages */
#define D37T_MSG_SET_CONFIG_MODE                (D37T_CLASS_IMA | 0x0001)
#define D37T_MSG_SET_CONFIG_MODE_CMPLT          (D37T_CLASS_IMA | 0x0002)
#define D37T_MSG_ADD_IMA_GROUP                  (D37T_CLASS_IMA | 0x0003)
#define D37T_MSG_ADD_IMA_GROUP_CMPLT            (D37T_CLASS_IMA | 0x0004)
#define D37T_MSG_REMOVE_IMA_GROUP               (D37T_CLASS_IMA | 0x0005)
#define D37T_MSG_REMOVE_IMA_GROUP_CMPLT         (D37T_CLASS_IMA | 0x0006)
#define D37T_MSG_IMA_GROUP_IFCS                 (D37T_CLASS_IMA | 0x0007)
#define D37T_MSG_IMA_GROUP_IFCS_CMPLT           (D37T_CLASS_IMA | 0x0008)
#define D37T_MSG_IFC_STATUS                     (D37T_CLASS_IMA | 0x0009)
#define D37T_MSG_IFC_STATUS_CMPLT               (D37T_CLASS_IMA | 0x000a)
#define D37T_MSG_GROUP_STATUS                   (D37T_CLASS_IMA | 0x000b)
#define D37T_MSG_GROUP_STATUS_CMPLT             (D37T_CLASS_IMA | 0x000c)
#define D37T_MSG_AVAIL_GROUPS                   (D37T_CLASS_IMA | 0x000d)
#define D37T_MSG_AVAIL_GROUPS_CMPLT             (D37T_CLASS_IMA | 0x000e)
#define D37T_MSG_GROUP_SETTINGS                 (D37T_CLASS_IMA | 0x000f)
#define D37T_MSG_GROUP_SETTINGS_CMPLT           (D37T_CLASS_IMA | 0x0010)
#define D37T_MSG_STATS_CELLS                    (D37T_CLASS_IMA | 0x0011)
#define D37T_MSG_STATS_CELLS_CMPLT              (D37T_CLASS_IMA | 0x0012)
#define D37T_MSG_CONF_STATUS                    (D37T_CLASS_IMA | 0x0013)
#define D37T_MSG_CONF_STATUS_CMPLT              (D37T_CLASS_IMA | 0x0014)

#define D37T_MSG_SET_CONFIG_MODE_SIZE           sizeof(t_d37t_msg_set_config_mode)
#define D37T_MSG_SET_CONFIG_MODE_CMPLT_SIZE     sizeof(t_d37t_msg_set_config_mode_cmplt)
#define D37T_MSG_ADD_IMA_GROUP_SIZE             sizeof(t_d37t_msg_add_ima_group) 
#define D37T_MSG_ADD_IMA_GROUP_CMPLT_SIZE       sizeof(t_d37t_msg_add_ima_group_cmplt)
#define D37T_MSG_REMOVE_IMA_GROUP_SIZE          sizeof(t_d37t_msg_remove_ima_group) 
#define D37T_MSG_REMOVE_IMA_GROUP_CMPLT_SIZE    sizeof(t_d37t_msg_remove_ima_group_cmplt) 
#define D37T_MSG_IMA_GROUP_IFCS_SIZE            sizeof(t_d37t_msg_ima_group_ifcs)
#define D37T_MSG_IMA_GROUP_IFCS_CMPLT_SIZE      sizeof(t_d37t_msg_ima_group_ifcs_cmplt)
#define D37T_MSG_IFC_STATUS_SIZE                sizeof(t_d37t_msg_ifc_status)
#define D37T_MSG_IFC_STATUS_CMPLT_SIZE          sizeof(t_d37t_msg_ifc_status_cmplt)
#define D37T_MSG_GROUP_STATUS_SIZE              sizeof(t_d37t_msg_group_status)
#define D37T_MSG_GROUP_STATUS_CMPLT_SIZE        sizeof(t_d37t_msg_group_status_cmplt)
#define D37T_MSG_AVAIL_GROUPS_SIZE              sizeof(t_d37t_msg_avail_groups)
#define D37T_MSG_AVAIL_GROUPS_CMPLT_SIZE        sizeof(t_d37t_msg_avail_groups_cmplt)
#define D37T_MSG_GROUP_SETTINGS_SIZE            sizeof(t_d37t_msg_group_settings)
#define D37T_MSG_GROUP_SETTINGS_CMPLT_SIZE      sizeof(t_d37t_msg_group_settings_cmplt)
#define D37T_MSG_STATS_CELLS_SIZE               sizeof(t_d37t_msg_stats_cells)     
#define D37T_MSG_STATS_CELLS_CMPLT_SIZE         sizeof(t_d37t_msg_stats_cells_cmplt)     
#define D37T_MSG_CONF_STATUS_SIZE               sizeof(t_d37t_msg_conf_status)
#define D37T_MSG_CONF_STATUS_CMPLT_SIZE         sizeof(t_d37t_msg_conf_status_cmplt)


typedef struct {
    uint32_t error_code;
    uint32_t data[8];		/* error-specific diagnostic codes */
} t_d37t_msg_error;

typedef struct {
    uint32_t debug;
} t_d37t_msg_set_debug;

typedef struct {
    uint32_t result;
} t_d37t_msg_set_debug_cmplt;

/* Return results */
#define D37T_ERROR_NONE			0	/* Success */
#define D37T_ERROR_UNKNOWN_MSG		0	/* Message ID is not recognized */

typedef struct {
    uint32_t length;		/* Number of bytes to read */
} t_d37t_msg_read_software_id;

typedef struct {
    uint32_t result;		/* Result of the operation */
    uint32_t length;		/* length of software ID (bytes) */
    uint8_t software_id[0];	/* Variable length software ID */
} t_d37t_msg_read_software_id_cmplt;

typedef struct {
    uint32_t length;		/* Number of bytes in the software ID */
    uint32_t key;		/* Security key to enable write access */
    uint8_t software_id[0];	/* Variable length software ID */
} t_d37t_msg_write_software_id;

typedef struct {
    uint32_t result;		/* Result of the write */
} t_d37t_msg_write_software_id_cmplt;

typedef struct {
    uint32_t sensor_id;		/* Which sensor to read */
} t_d37t_msg_read_temperature;

typedef struct {
    uint32_t result;		/* Result of the operation */
    int32_t temperature;	/* The temperature read */
} t_d37t_msg_read_temperature_cmplt;

typedef struct
{
    uint32_t    wMode;		    
} t_d37t_msg_set_config_mode;

typedef struct
{
    uint32_t    wReturn;
} t_d37t_msg_set_config_mode_cmplt;

typedef struct 
{
	uint32_t 	wGroupId;	
	uint32_t 	wIfcMask;	
} t_d37t_msg_add_ima_group;

typedef struct
{
    uint32_t    wHandle;
} t_d37t_msg_add_ima_group_cmplt;

typedef struct
{
    uint32_t	wHandle;
} t_d37t_msg_remove_ima_group;

typedef struct
{
    uint32_t	wReturn;
} t_d37t_msg_remove_ima_group_cmplt;

typedef struct
{
	uint32_t 	wHandle;	
} t_d37t_msg_ima_group_ifcs;

typedef struct
{
	uint32_t 	wGroupId;	
	uint32_t	wIfcMask;	
} t_d37t_msg_ima_group_ifcs_cmplt;						

typedef struct
{
	uint32_t 	wIfc;		
} t_d37t_msg_ifc_status;					

typedef struct
{
	uint32_t 	wReturn;	
} t_d37t_msg_ifc_status_cmplt;						

typedef struct
{
	uint32_t 	wHandle;	
} t_d37t_msg_group_status;					

typedef struct
{
	uint32_t 	wReturn;	
} t_d37t_msg_group_status_cmplt;

typedef struct
{
    uint32_t wDummy;
    /* there is nothing here since we just want to return the available groups
       and don't need any more info. will this work?
    */
} t_d37t_msg_avail_groups;

typedef struct
{
    uint32_t wNumGroups;
    uint32_t awHandle[MAX_INTERFACES];
} t_d37t_msg_avail_groups_cmplt;

typedef struct
{
    uint32_t wHandle;
} t_d37t_msg_group_settings;

typedef struct
{
    uint32_t wNumIfcs;
    uint32_t wFrameLen;
} t_d37t_msg_group_settings_cmplt;

typedef struct 
{
    uint32_t wCellCmd;  /* tells whether the option is a group or ifc */
    uint32_t wOption;   /* either a group handle or an ifc num */
} t_d37t_msg_stats_cells;

typedef struct
{
    uint32_t wCellCount;
} t_d37t_msg_stats_cells_cmplt;

typedef struct
{
    uint32_t wDummy;    /* there is nothing to send in this message */
} t_d37t_msg_conf_status;

typedef struct
{
    uint32_t wReturn;   /* status of the configuration of IMA groups */
} t_d37t_msg_conf_status_cmplt;

/*
 * Low level communication interface
 */
#define D37T_COMM_ENABLED	0x80000000	/* Flags that bi-direction comm is on */
#define D37T_COMM_CLEAR_TO_SEND	0x40000000
#define D37T_COMM_DATA_VALID	0x20000000
#define D37T_COMM_DATA_READ	0x10000000
#define D37T_COMM_DATA_MASK	0x000000FF

void d37t_comm_set_verbosity(int verbosity);
int d37t_start_xscale(t_d37t_comm *dagc);

/* Initialize communicatins in dagconsole socket mode */
int d37t_comm_init(int dagfd, t_d37t_comm *dagc, int start_xscale);

/* Same as above, but use direct register mode instead of dagconsole socket mode */
int d37t_comm_init_drb(int dagfd, t_d37t_comm *dagc, int start_xscale);

int d37t_read_message(t_d37t_comm *dagc, t_d37t_msg *msgp);
int d37t_send_message(t_d37t_comm *dagc, t_d37t_msg *msgp);
int d37t_setparm_uint32(t_d37t_comm *dagc, uint32_t parm_id, uint32_t value);

/**********************************************************************
* FUNCTION:	d37t_set_debug_handler(*dagc, *handler)
* DESCRIPTION:	Set the handler to call for debug string outputs.
*		This handler will be called whenever a debug string
*		is received from the board.
*		Handler prototype: void handler(char *str, uint32_t *len)
*		Called with a pointer to the string, and the ln is the
*		number of bytes in the string including null terminator.
*		Setting the handler to NULL will supress the outputs.
*		The default handler outputs the strings to stdout.
*
*		If the handler value is D37T_DEFAULT_HANDLER then
*		the default handler is enabled with outputs the debug
*		strings to stdout.
*
* INPUTS:	dagc	- 3.7t communication descriptor
*		handler - pointer to the debug handler function to use.
* RETURNS:	none.
**********************************************************************/
void d37t_set_debug_handler(
    t_d37t_comm *dagc,
    void (*handler)(char *debug_str, uint32_t length));

#define D37T_DEFAULT_HANDLER ((void (*)(char *debug_str, uint32_t length))1)

/* Bit-based debug flags */
#define D37T_DBG_SEND		0x00000001
#define D37T_DBG_I2C		0x00000002
#define D37T_DBG_API		0x00000004

#endif /* _D37T_MSG_H_ */
