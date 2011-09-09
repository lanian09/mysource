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

#ifndef _D37T_I2C_H_
#define _D37T_I2C_H_

#include "d37t_msg.h"

/* Maximum size of the 3.7T software ID */
#define D37T_SOFTWARE_ID_SIZE	128

/**********************************************************************
* FUNCTION:	d37t_write_software_id(num_bytes, datap, key) 
* DESCRIPTION:	Store a new software ID.  If the key is incorrect the 
*		board will halt and a restart is required.
* INPUTS:	num_bytes	- number of bytes to read.  1-128.
*		datap		- byte array large enough to hold the ID.
*		key		- Security key to enable write mode.
* OUTPUTS:	none
* RETURNS:	0	- success
*		-1	- invalid num_bytes value
*		-2	- operational failure reading ID
*		-3	- Timeout communicating with board
**********************************************************************/
int d37t_write_software_id(t_d37t_comm *dagc, int32_t num_bytes, uint8_t *datap, uint32_t key);


/**********************************************************************
* FUNCTION:	d37t_read_software_id(num_bytes, datap) 
* DESCRIPTION:	Read the contents of the software ID.
* INPUTS:	num_bytes	- number of bytes to read.  1-128.
*		datap		- byte array large enough to hold the ID.
* OUTPUTS:	*datap		- populated with the Software ID.
* RETURNS:	0	- success
*		-1	- invalid num_bytes value
*		-2	- operational failure reading ID
*		-3	- timeout
**********************************************************************/
int d37t_read_software_id(t_d37t_comm *dagc, int32_t num_bytes, uint8_t *datap);


/**********************************************************************
* FUNCTION:	d37t_read_temperature(sensor_id, int *temperature)
* DESCRIPTION:	Read the temperature from the specified sensor.
* INPUTS:	sensor_id	- ID of the sensor to read.  0 = default.
* OUTPUTS:	*temperature	- The current temperature reading in degrees Celcius.
* RETURNS:	0	- success
		-1	- invalid sensor id
*		-2	- operational failure reading sensor
**********************************************************************/
int d37t_read_temperature(t_d37t_comm *dagc, uint32_t sensor_id, int *temperature);


/**********************************************************************
* FUNCTION:	d37t_set_debug(debug_flags)
* DESCRIPTION:	Enable/disable debugging flags on the board.
* INPUTS:	debug_flags	- 32-bit flag set
* RETURNS:	0	- success
*		-1	- operation failed.
**********************************************************************/
int d37t_set_debug(t_d37t_comm *dagc, uint32_t debug_flags);

#endif /* _D37T_I2C_H_ */
